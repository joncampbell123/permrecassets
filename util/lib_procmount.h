
#ifndef UTIL_LIB_PROCMOUNT_H
#define UTIL_LIB_PROCMOUNT_H

#include <string>
#include <vector>

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

#endif /* UTIL_LIB_PROCMOUNT_H */

