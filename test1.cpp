
#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>

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
        if (*a == '-') {
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

int main(int argc,char **argv) {
    if (!parse_argv(argc,argv))
        return 1;

    return 0;
}

