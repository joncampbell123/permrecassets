
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>

#include "lib_procmount.h"
#include "lib_path_rel_label.h"
#include "lib_prluuid.h"

using namespace std;

int main(int argc,char **argv) {
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

    if (prl.fs_label.empty()) {
        fprintf(stderr,"Filesystem must have a label\n");
        return 1;
    }

    return 0;
}

