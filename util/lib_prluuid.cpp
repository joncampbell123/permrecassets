
#include <assert.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>

#include "lib_prluuid.h"

#include <uuid/uuid.h>

using namespace std;

void prluuidgen(prluuid &u) {
    struct timeval tv;
    uint64_t tv64;

    assert(sizeof(u) >= 24);

    gettimeofday(&tv,NULL);
    tv64 = ((uint64_t)tv.tv_sec * (uint64_t)1000) + ((uint64_t)tv.tv_usec / (uint64_t)1000)/*us->ms*/;

    uuid_generate(&u.uuid[0]);
    *((uint64_t*)(&u.uuid[16])) = htobe64(tv64);
}

string prluuid_to_string(const prluuid &u) {
    char tmp[8];
    string s;

    for (size_t i=0;i < 24;i++) {
        sprintf(tmp,"%02x",u.uuid[i]);
        s += tmp;
    }

    return s;
}

std::string prluuid::to_string() const {
    return prluuid_to_string(*this);
}

