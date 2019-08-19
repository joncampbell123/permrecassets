
#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

#include "textsrc.hpp"

#include "json11.hpp"

#ifndef O_BINARY
#define O_BINARY (0)
#endif

using namespace std;

vector<string>      files;

struct WordToken {
    /* first of map pair is the word */
    unsigned int                count = 0;
};

map<string,WordToken>           words;

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

void refill_deque(std::deque<int> &in,TextSourceBase &ts) {
    /* load buffer. make sure there's plenty of chars in memory to handle.
     * we're looking for words. */
    while (!ts.eof() && !ts.error() && in.size() < 1024)
        in.push_back(ts.getc());
}

bool isalphanumeric(int c) {
    if (c >= '0' && c <= '9')
        return true;
    if (c >= 'a' && c <= 'z')
        return true;
    if (c >= 'A' && c <= 'Z')
        return true;

    return false;
}

bool ishyphen(int c) {
    if (c == '-')
        return true;

    return false;
}

bool iswordspace(int c) {
    if (c == ' ')
        return true;

    return false;
}

bool isnewline(int c) {
    if (c == '\n')
        return true;

    return false;
}

void eat_wordspace(std::deque<int> &in) {
    while (!in.empty() && in[0] == ' ')
        in.pop_front();
}

void get_word(std::string &word,std::deque<int> &in) {
    word.clear();

    while (!in.empty() && isalphanumeric(in[0])) {
        word += (char)in.front();
        in.pop_front();
    }
}

bool process_file(TextSourceBase &ts) {
    std::deque<int> in;

    do {
        refill_deque(in,ts);
        if (in.empty())
            break;

        /* read groups of numbers and letters as a word. */
        if (isalphanumeric(in[0])) {
            /* register individual words, as well as words-hyphenated-like-this */
            std::string group_word,word;

            while (!in.empty() && isalphanumeric(in[0])) {
                refill_deque(in,ts);
                get_word(/*&*/word,in);// will also consume word
                eat_wordspace(in);// eat space
                if (!in.empty() && in[0] == '\n') { // eat newline
                    in.pop_front();
                    eat_wordspace(in);// eat space
                }
                refill_deque(in,ts);
                group_word += word;

                printf("Word: %s\n",word.c_str());

                // if the next is a hyphen/separator followed by space and another word, then eat the hypen and parse the next word
                if (!in.empty() && ishyphen(in[0])) {
                    size_t i=1;
                    while (i < in.size() && (iswordspace(in[i]) || isnewline(in[i]))) i++;
                    if (i < in.size() && isalphanumeric(in[i])) {
                        // found it. add hypen to group word
                        group_word += in[0];
                        // eat the hypen, and space
                        for (size_t c=0;c < i;c++) {
                            assert(!in.empty());
                            in.pop_front();
                        }
                        // loop around, to process it
                    }
                }
                else {
                    break;
                }
            }

            if (word != group_word)
                printf("Group: %s\n",group_word.c_str());
        }
        else {
            /* drop it */
            in.pop_front();
        }
    } while (1);

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

