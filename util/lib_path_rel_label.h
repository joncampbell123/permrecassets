
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

bool path_to_prl(const procmount_list &pml,path_rel_label &prl,const char *ipath);
bool path_to_prl(const procmount_list &pml,path_rel_label &prl,const std::string ipath);
 
#endif /* UTIL_LIB_PATH_REL_LABEL_H */

