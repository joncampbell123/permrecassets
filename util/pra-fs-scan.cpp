
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>

#include "lib_procmount.h"
#include "lib_path_rel_label.h"

#include <uuid/uuid.h>

using namespace std;

typedef unsigned char prluuid[24]; /* 16-byte UUID + 8-byte timestamp */

void prluuidgen(prluuid &u) {
    struct timeval tv;
    uint64_t tv64;

    assert(sizeof(u) >= 24);

    gettimeofday(&tv,NULL);
    tv64 = ((uint64_t)tv.tv_sec * (uint64_t)1000) + ((uint64_t)tv.tv_usec / (uint64_t)1000)/*us->ms*/;

    uuid_generate(&u[0]);
    *((uint64_t*)(&u[16])) = htobe64(tv64);
}

string prluuid_to_string(const prluuid &u) {
    char tmp[8];
    string s;

    for (size_t i=0;i < 24;i++) {
        sprintf(tmp,"%02x",u[i]);
        s += tmp;
    }

    return s;
}

int main(int argc,char **argv) {
    path_rel_label prl;
    procmount_list pml;
    struct stat st;

    if (argc < 2) {
        fprintf(stderr,"Specify location\n");
        return 1;
    }

    prluuid uuid;
    prluuidgen(uuid);
    fprintf(stderr,"%s\n",prluuid_to_string(uuid).c_str());

    /* the given location must be a directory. not a file. not a symlink. */
    if (lstat(argv[1],&st)) {
        perror("Cannot stat");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        perror("Not a directory");
        return 1;
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

    return 0;
}

