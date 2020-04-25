
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

using namespace std;

std::string cpp_realpath(const char *path) {
    char *rp = realpath(path,NULL);
    if (rp == NULL) return std::string();
    std::string r = rp;
    free(rp);
    return r;
}

std::string cpp_realpath(const std::string &path) {
    return cpp_realpath(path.c_str());
}

