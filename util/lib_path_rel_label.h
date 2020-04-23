
#ifndef UTIL_LIB_PATH_REL_LABEL_H
#define UTIL_LIB_PATH_REL_LABEL_H

#include <string>

struct path_rel_label {
    std::string             device;
    std::string             abs_path;
    std::string             fs_label;
    std::string             mountpoint;
    std::string             relpath;

    void clear(void) {
        device.clear();
        abs_path.clear();
        fs_label.clear();
        mountpoint.clear();
        relpath.clear();
    }
};

#endif /* UTIL_LIB_PATH_REL_LABEL_H */

