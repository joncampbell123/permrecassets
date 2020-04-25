
#include <sys/types.h>
#include <sys/statvfs.h>
#include <stdio.h>

#include <string>

using namespace std;

bool prl_fs_readonly(const char *path) {
    struct statvfs st;

    if (statvfs(path,&st) != 0)
        return false;

    return (st.f_flag & ST_RDONLY) != 0;
}

bool prl_fs_readonly(const std::string &path) {
    return prl_fs_readonly(path.c_str());
}

int main(int argc,char **argv) {
    if (argc < 2) {
        fprintf(stderr,"Specify location\n");
        return 1;
    }

    return prl_fs_readonly(argv[1]) ? 0 : 1;
}

