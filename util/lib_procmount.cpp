
#include <assert.h>
#include <stdio.h>

#include "lib_procmount.h"

using namespace std;

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

