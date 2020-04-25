
#include <stdio.h>

#include "lib_fs_isrdonly.h"

int main(int argc,char **argv) {
    if (argc < 2) {
        fprintf(stderr,"Specify location\n");
        return 1;
    }

    return prl_fs_readonly(argv[1]) ? 0 : 1;
}

