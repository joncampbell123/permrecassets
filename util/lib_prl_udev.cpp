
#include "lib_prl_udev.h"

using namespace std;

std::string prl_udev_unescape(const std::string &scpp) {
    const char *s = scpp.c_str();
    string d;
    char c;

    do {
        c = *s++;
        if (c == 0) break;

        if (c == '\\') {
            c = *s++;
            if (c == 0) break;

            if (c == 'x') {
                char tmp[3];

                c = *s++;
                if (c == 0) break;
                tmp[0] = c;

                c = *s++;
                if (c == 0) break;
                tmp[1] = c;

                tmp[2] = 0;

                c = (char)strtol(tmp,NULL,0x10);
                if (c == 0) break;
                d += c;
            }

            continue;
        }

        d += c;
    } while (1);

    return d;
}

