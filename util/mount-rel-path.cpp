
#include <assert.h>
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

using namespace std;

struct procmount_entry {
    std::string         device;
    std::string         mountpoint;
    std::string         filesystem;
    std::string         options;
};

struct procmount_list {
    std::vector<procmount_entry>    mounts;

    void clear(void) {
        mounts.clear();
    }
};

bool procmount_list_read(procmount_list &l) {
    static const int field_max = 6;
    char line[1024];
    char *s,*fields[field_max];
    int field;

    l.clear();

    FILE *fp = fopen("/proc/mounts","r");
    if (fp == NULL) return false;
    while (!feof(fp) && !ferror(fp)) {
        if (fgets(line,sizeof(line),fp) == NULL) break;
        s = line;
        field = 0;
        while (*s != 0) {
            if (field < field_max)
                fields[field++] = s;

            while (*s != 0 && *s != ' ') s++;
            if (*s == ' ') *s++ = 0;
        }
        assert(field <= field_max);

        if (field >= 4) {
            l.mounts.resize(l.mounts.size()+(size_t)1);
            assert(l.mounts.size() > 0);

            procmount_entry &ent = l.mounts[l.mounts.size()-(size_t)1];
            ent.device     = fields[0]; // assume != NULL
            ent.mountpoint = fields[1]; // assume != NULL
            ent.filesystem = fields[2]; // assume != NULL
            ent.options    = fields[3]; // assume != NULL
        }
    }
    fclose(fp);

    return true;
}

struct path_rel_label {
    std::string             abs_path;
    std::string             fs_label;
    std::string             mountpoint;
    std::string             relpath;

    void clear(void) {
        abs_path.clear();
        fs_label.clear();
        mountpoint.clear();
        relpath.clear();
    }
};

bool path_to_prl_inner(const procmount_list &pml,path_rel_label &prl,const char *ipath) {
    prl.clear();
    prl.abs_path = ipath;

    size_t ipath_match = 0;
    const size_t ipath_length = strlen(ipath);
    for (size_t mi=0;mi < pml.mounts.size();mi++) {
        const procmount_entry &ent = pml.mounts[mi];

        if (!strncmp(ipath,ent.mountpoint.c_str(),ent.mountpoint.size())) {
            if (ipath_match < ent.mountpoint.size()) {
                ipath_match = ent.mountpoint.size();
                prl.mountpoint = ent.mountpoint;
                const char *s = ipath + ent.mountpoint.size();
                assert(s <= (ipath+ipath_length));
                while (*s == '/') s++;
                prl.relpath = s;
            }
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

    return 0;
}

