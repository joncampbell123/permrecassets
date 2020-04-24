
#ifndef UTIL_LIB_PRLUUID_H
#define UTIL_LIB_PRLUUID_H

#include <string>

struct prluuid {
    unsigned char       uuid[24]; /* 16-byte UUID + 8-byte timestamp */
};

void prluuidgen(prluuid &u);
std::string prluuid_to_string(const prluuid &u);

#endif /*UTIL_LIB_PRLUUID_H*/

