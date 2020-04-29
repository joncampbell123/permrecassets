
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "lib_procmount.h"
#include "lib_path_rel_label.h"
#include "lib_prluuid.h"
#include "lib_splitpath.h"
#include "lib_prl_blob.h"
#include "lib_prl_nodes.h"
#include "lib_cpp_realpath.h"
#include "lib_fs_isrdonly.h"

using namespace std;

static void scan_dir(const path_rel_label &prl,const std::string &rpath,const prl_node_entry &parent_node) {
    const std::string fpath = prl.mountpoint + (rpath.empty() ? "" : ("/" + rpath));
    prl_node_entry child_node;
    struct dirent *d;
    struct stat st;
    time_t pt=0,ct;
    DIR *dir;

    std::string ver;
    if ((ver=cpp_realpath(fpath)) != fpath) {
        fprintf(stderr,"Realpath failure for %s (scan_dir) got %s\n",fpath.c_str(),ver.c_str());
        return;
    }

    dir = opendir(fpath.c_str());
    if (dir == NULL) return;

    printf("\x0D" "%s" "\x1B[K",rpath.c_str());
    fflush(stdout);

    /* files first */
    rewinddir(dir);
    while ((d=readdir(dir)) != NULL) {
        if (!strcmp(d->d_name,".") || !strcmp(d->d_name,".."))
            continue;

        const std::string filepath = fpath + "/" + d->d_name;

        if (lstat(filepath.c_str(),&st)) continue;

        ct = time(NULL);
        if (pt != ct) {
            printf("\x0D" "%s/%s" "\x1B[K",rpath.c_str(),d->d_name);
            fflush(stdout);
            pt = ct;
        }

        child_node.inode = st.st_ino;
        child_node.mtime = st.st_mtime;
        child_node.name_charset = "UTF-8";/*I always use UTF-8 for archives and the local fs */
        child_node.assume_not_exist = parent_node.this_node_did_not_exist;
        child_node.parent_node = parent_node.node_id;
        child_node.real_name = std::string(d->d_name);
        child_node.name = d->d_name;

        if (S_ISREG(st.st_mode)) {
            child_node.type = NODE_TYPE_FILE;
            child_node.size = (uint64_t)st.st_size;
        }
        else if (S_ISLNK(st.st_mode)) {
            child_node.type = NODE_TYPE_SYMLINK;
        }
        else {
            continue;
        }

        /* also updates node_id */
        if (!prl_node_db_add_fsentbyname(/*in&out*/child_node)) {
            fprintf(stderr,"Failed to add or update fs node\n");
            break;
        }
    }

    /* then directories */
    rewinddir(dir);
    while ((d=readdir(dir)) != NULL) {
        if (!strcmp(d->d_name,".") || !strcmp(d->d_name,".."))
            continue;

        const std::string filepath = fpath + "/" + d->d_name;

        if (lstat(filepath.c_str(),&st)) continue;

        ct = time(NULL);
        if (pt != ct) {
            printf("\x0D" "%s/%s" "\x1B[K",rpath.c_str(),d->d_name);
            fflush(stdout);
            pt = ct;
        }

        child_node.inode = st.st_ino;
        child_node.mtime = st.st_mtime;
        child_node.name_charset = "UTF-8";/*I always use UTF-8 for archives and the local fs */
        child_node.assume_not_exist = parent_node.this_node_did_not_exist;
        child_node.parent_node = parent_node.node_id;
        child_node.real_name = std::string(d->d_name);
        child_node.name = d->d_name;

        if (S_ISDIR(st.st_mode)) {
            child_node.type = NODE_TYPE_DIRECTORY;
        }
        else {
            continue;
        }

        /* also updates node_id */
        if (!prl_node_db_add_fsentbyname(/*in&out*/child_node)) {
            fprintf(stderr,"Failed to add or update fs node\n");
            break;
        }

        if (S_ISDIR(st.st_mode)) {
            scan_dir(prl,rpath + (rpath.empty() ? "" : "/") + d->d_name,child_node); /* child node becomes parent later */

            printf("\x0D" "%s" "\x1B[K",rpath.c_str());
            fflush(stdout);
        }
    }

    closedir(dir);

    printf("\x0D" "\x1B[K");
    fflush(stdout);
}

int main(int argc,char **argv) {
    prl_node_entry parent_node;
    path_rel_label prl;
    procmount_list pml;
    struct stat st;

    if (argc < 2) {
        fprintf(stderr,"Specify location\n");
        return 1;
    }

    /* the given location must be a directory. not a file. not a symlink. */
    if (lstat(argv[1],&st)) {
        perror("Cannot stat");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr,"Not a directory\n");
        return 1;
    }

    if (!prl_fs_readonly(argv[1])) {
        if (isatty(0)) {
            char resp[512];

            printf("Filesystem should be readonly for safety reasons!\n");
            printf("Type YES and hit ENTER to continue\n");

            resp[0]=0;
            fgets(resp,sizeof(resp),stdin);
            if (!strncasecmp(resp,"yes",3)) {
                /* OK */
            }
            else {
                return 1;
            }
        }
        else {
            fprintf(stderr,"Filesystem must be read only for safety reasons\n");
            return 1;
        }
    }

    procmount_list_read(pml);
    if (!path_to_prl(pml,prl,argv[1])) {
        fprintf(stderr,"Unable to map\n");
        return 1;
    }
    printf("Rel of '%s'\n",argv[1]);
    printf("  abs_path:         '%s'\n",prl.abs_path.c_str());
    printf("  fs_label:         '%s'\n",prl.fs_label.c_str());
    printf("  mountpoint:       '%s'\n",prl.mountpoint.c_str());
    printf("  relpath:          '%s'\n",prl.relpath.c_str());
    printf("  device:           '%s'\n",prl.device.c_str());

    if (prl.fs_label.empty()) {
        fprintf(stderr,"Filesystem must have a label\n");
        return 1;
    }

    if (!prl_node_db_open()) {
        fprintf(stderr,"Unable to open SQLite3 DB. Use pra-fs-scan-db-init.sh\n");
        return 1;
    }

    parent_node.type = NODE_TYPE_ARCHIVE;
    parent_node.name_charset = "UTF-8";/*I always use UTF-8 for archives and the local fs */
    parent_node.parent_node = prl_zero_node;
    parent_node.real_name = prl.fs_label;
    parent_node.name = prl.fs_label;
    parent_node.name_charset = "UTF-8";/*I always use UTF-8 for archives and the local fs */
    if (!prl_node_db_add_fsentbyname(/*&return*/parent_node)) {
        fprintf(stderr,"Failed to add or update archive node\n");
        prl_node_db_close();
        return 1;
    }

    /* relative path */
    {
        string rpath = prl.mountpoint;
        vector<string> path;

        if (cpp_realpath(rpath) != rpath) {
            fprintf(stderr,"Realpath failure for %s (207)\n",rpath.c_str());
            prl_node_db_close();
            return 1;
        }

        prl_path_split(path,prl.relpath);
        for (vector<string>::iterator i=path.begin();i!=path.end();i++) {
            prl_node_entry child_node;

            if (lstat(rpath.c_str(),&st) || !S_ISDIR(st.st_mode)) {
                fprintf(stderr,"rpath dir verification fail\n");
                prl_node_db_close();
                return 1;
            }

            child_node.inode = st.st_ino;
            child_node.mtime = st.st_mtime;
            child_node.name_charset = "UTF-8";/*I always use UTF-8 for archives and the local fs */
            child_node.type = NODE_TYPE_DIRECTORY;/*remember the above code MAKES SURE the path given is a directory*/
            child_node.assume_not_exist = parent_node.this_node_did_not_exist;
            child_node.parent_node = parent_node.node_id;
            child_node.real_name = (*i);
            child_node.name = (*i);

            if (!prl_node_db_add_fsentbyname(/*in&out*/child_node)) {
                fprintf(stderr,"Failed to add or update fs node\n");
                prl_node_db_close();
                return 1;
            }

            rpath += "/";
            rpath += (*i);

            if (cpp_realpath(rpath) != rpath) {
                fprintf(stderr,"Realpath failure for %s (234)\n",rpath.c_str());
                prl_node_db_close();
                return 1;
            }

            /* updates child_node.node_id, which becomes the parent_node going forward */
            parent_node = child_node;
        }
    }

    /* scandir */
    scan_dir(prl,prl.relpath,parent_node);

    prl_node_db_close();
    return 0;
}

