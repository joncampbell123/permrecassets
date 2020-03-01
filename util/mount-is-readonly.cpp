
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include <string>

using namespace std;


int main(int argc,char **argv) {
    struct statvfs st;

    if (argc < 2) {
        fprintf(stderr,"Specify location\n");
        return 1;
    }

    if (statvfs(argv[1],&st) != 0) {
        fprintf(stderr,"Cannot stat\n");
        return 1;
    }

    if (st.f_flag & ST_RDONLY)
        return 0;

    return 1;
}

