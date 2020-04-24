
#include <assert.h>

#include "lib_splitpath.h"

using namespace std;

void prl_path_split(std::vector<std::string> &spath,const std::string &path) {
    spath.clear();

    {
        const char *s;

        s = path.c_str();
        while (*s != 0) {
            const char *f = s;
            while (*s != 0 && *s != '/') s++;
            if (s > f) spath.push_back(std::string(f,(size_t)(s-f)));
            while (*s != 0 && *s == '/') s++;
        }
    }
}

