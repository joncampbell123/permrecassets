
#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

#include "textsrc.hpp"

#ifndef O_BINARY
#define O_BINARY (0)
#endif

using namespace std;

vector<string>      files;

static void help(void) {
    fprintf(stderr,"TODO\n");
}

static bool parse_argv(int argc,char **argv) {
    char *a;
    int i=1;

    while (i < argc) {
        a = argv[i++];
        if (a[0] == '-' && a[1] != 0) { /* - alone is not a switch */
            do { a++; } while (*a == '-');

            if (!strcmp(a,"h")) {
                help();
                return false;
            }
            else {
                fprintf(stderr,"Unknown %s\n",a);
                return false;
            }
        }
        else {
            files.push_back(a);
        }
    }

    return true;
}

bool process_file(TextSourceBase &ts) {
    int c;

    while ((c=ts.getc_nl()) >= 0)
        printf("%c",c);

    if (ts.error()) {
        fprintf(stderr,"File read error\n");
        return false;
    }
    if (!ts.eof()) {
        fprintf(stderr,"File not eof\n");
        return false;
    }

    return true;
}

bool process_file(const string &path) {
    if (path == "-"/*stdin*/) {
        TextSourceStdin src;
        if (!src.is_open()) return false;
        return process_file(src);
    }
    else {
        TextSource src(path);
        if (!src.is_open()) return false;
        return process_file(src);
    }
}

int main(int argc,char **argv) {
    if (!parse_argv(argc,argv))
        return 1;

    if (files.empty()) {
        fprintf(stderr,"Please specify files to parse\n");
        return 1;
    }

    for (auto &file : files) {
        if (!process_file(file)) {
            fprintf(stderr,"Unable to process file %s\n",file.c_str());
            return 1;
        }
    }

    return 0;
}

