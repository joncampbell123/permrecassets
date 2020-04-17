
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

int main(int argc,char **argv) {
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

    return 0;
}

