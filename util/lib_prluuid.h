
#ifndef UTIL_LIB_PRLUUID_H
#define UTIL_LIB_PRLUUID_H

#include <string>

struct prluuid {
    unsigned char       uuid[24]; /* 16-byte UUID + 8-byte timestamp */

    std::string         to_string() const;
};

void prluuidgen(prluuid &u);

#endif /*UTIL_LIB_PRLUUID_H*/

