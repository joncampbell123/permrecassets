
#include <assert.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include <string>
#include <vector>

#include "lib_procmount.h"
#include "lib_path_rel_label.h"

using namespace std;

bool path_to_prl_inner(const procmount_list &pml,path_rel_label &prl,const char *ipath) {
    prl.clear();
    prl.abs_path = ipath;

    /* NTS: Match the longest mountpoint, in case of filesystems within filesystems */
    size_t ipath_match = 0;
    const size_t ipath_length = strlen(ipath);
    for (size_t mi=0;mi < pml.mounts.size();mi++) {
        const procmount_entry &ent = pml.mounts[mi];

        /* Centos likes to list the root '/' filesystem twice, one as the device, the other as "rootfs", which is unhelpful */
        if (ent.device == "rootfs")
            continue;

        if (!strncmp(ipath,ent.mountpoint.c_str(),ent.mountpoint.size())) {
            if (ipath_match < ent.mountpoint.size()) {
                ipath_match = ent.mountpoint.size();
                prl.mountpoint = ent.mountpoint;
                const char *s = ipath + ent.mountpoint.size();
                assert(s <= (ipath+ipath_length));
                while (*s == '/') s++;
                prl.relpath = s;
                prl.device = ent.device;
            }
        }
    }

    if (!prl.mountpoint.empty() && !prl.device.empty()) {
        /* use udev /dev/disk filesystem structures to get label, which should work on my custom Linux distro and most common distros */
        const char *basepath = "/dev/disk/by-label";
        struct dirent *d;
        DIR *dir;

        dir = opendir(basepath);
        if (dir != NULL) {
            while ((d=readdir(dir)) != NULL && prl.fs_label.empty()) {
                if (d->d_name[0] == '.') continue;
                const std::string path = std::string(basepath) + "/" + d->d_name;
                char *rpath = realpath(path.c_str(),NULL);
                if (rpath != NULL) {
                    if (prl.device == rpath) {
                        prl.fs_label = d->d_name;
                    }
                    free(rpath);
                }
            }
            closedir(dir);
        }
    }

    return !prl.mountpoint.empty();
}

bool path_to_prl(const procmount_list &pml,path_rel_label &prl,const char *ipath) {
    bool res = false;
    char *ripath = realpath(ipath,NULL);
    if (ripath == NULL) return false;
    if (*ripath == '/') res = path_to_prl_inner(pml,prl,ripath);
    free(ripath);
    return res;
}

bool path_to_prl(const procmount_list &pml,path_rel_label &prl,const std::string ipath) {
    return path_to_prl(pml,prl,ipath.c_str());
}

int main(int argc,char **argv) {
    path_rel_label prl;
    procmount_list pml;
    struct statvfs st;

    if (argc < 2) {
        fprintf(stderr,"Specify location\n");
        return 1;
    }

    procmount_list_read(pml);
    printf("Mounts:\n");
    for (size_t i=0;i < pml.mounts.size();i++) {
        procmount_entry &ent = pml.mounts[i];
        printf("  device='%s' mountpoint='%s' filesystem='%s' options='%s'\n",
            ent.device.c_str(),             ent.mountpoint.c_str(),
            ent.filesystem.c_str(),         ent.options.c_str());
    }

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

    return 0;
}

