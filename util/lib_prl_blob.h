
#ifndef UTIL_LIB_PRL_BLOB_H
#define UTIL_LIB_PRL_BLOB_H

#include <stdint.h>
#include <string>
#include <vector>

// TODO: Just make this code C++11 so we can use "using" instead of this mess
typedef std::vector<uint8_t> prl_blob_base;

class prl_blob : public prl_blob_base {
public:
    prl_blob() : prl_blob_base() { }
    prl_blob(const std::string &x) : prl_blob_base() {
        resize(x.size());
        for (size_t i=0;i < x.size();i++) prl_blob_base::operator[](i) = x[i]; /* Not the NUL at the end, though */
    }
    ~prl_blob() { }
    std::string string_value(void) const {
        return std::string((char*)(&prl_blob_base::operator[](0)),prl_blob_base::size());
    }
};

#endif /*UTIL_LIB_PRL_BLOB_H*/

