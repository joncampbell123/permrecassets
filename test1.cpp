
#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

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

class TextSourceBase {
public:
    TextSourceBase() {
    }
    virtual ~TextSourceBase() {
    }
    virtual bool is_open(void) const {
        return false;
    }
    virtual void close(void) {
    }
    virtual bool open(void) {
        return false;
    }
};

class TextSource : public TextSourceBase {
public:
    TextSource() : TextSourceBase() {
    }
    TextSource(const string &_path) : TextSourceBase(), path(_path) {
        open();
    }
    virtual ~TextSource() {
        close();
    }
    virtual bool is_open(void) const {
        return (fd >= 0);
    }
    virtual void close(void) {
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
        }
    }
    virtual bool open(void) {
        if (!is_open()) {
            if (path.empty()) return false;
            fd = ::open(path.c_str(),O_RDONLY|O_BINARY);
            if (fd < 0) return false;
        }

        return true;
    }
public:
    int             fd = -1;
    string          path;
};

class TextSourceStdin : public TextSourceBase {
public:
    TextSourceStdin() : TextSourceBase() {
    }
    virtual ~TextSourceStdin() {
    }
    virtual bool is_open(void) const {
        return true;
    }
    virtual bool open(void) {
        return true;
    }
};

bool process_file(const string &path) {
    if (path == "-"/*stdin*/) {
        TextSourceStdin src;
        if (!src.is_open()) return false;
    }
    else {
        TextSource src(path);
        if (!src.is_open()) return false;
    }

    return true;
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

