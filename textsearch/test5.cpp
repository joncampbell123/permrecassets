
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

#include <algorithm>

using namespace std;

vector<string>      files;

struct WordToken {
    /* first of map pair is the word */
    unsigned int                count = 0;
};

map<string,WordToken>           words;
signed long long                highest_word_count = 0;

std::string filter_word(const std::string &raw_word) {
    std::string res = raw_word;

    for (auto &c : res) c = tolower(c);

    return res;
}

void count_word(const std::string &raw_word) {
    std::string filtered_word = filter_word(raw_word);

    if (!filtered_word.empty()) {
        words[filtered_word].count++;
        if (highest_word_count < words[filtered_word].count)
            highest_word_count = words[filtered_word].count;
    }
}

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
            std::string group_word,word,tmp;

            while (!in.empty() && isalphanumeric(in[0])) {
                refill_deque(in,ts);
                get_word(/*&*/tmp,in);// will also consume word
                word = tmp;

                // Initialisms like M.D. with no spaces
                while (in.size() >= 2 && in[0] == '.' && isalphanumeric(in[1]) && tmp.size() < 4) {
                    word += (char)in.front();
                    in.pop_front(); // eat the '.'
                    get_word(/*&*/tmp,in);// will also consume word
                    word += tmp;
                }

                eat_wordspace(in);// eat space
                if (!in.empty() && in[0] == '\n') { // eat newline
                    in.pop_front();
                    eat_wordspace(in);// eat space
                }
                refill_deque(in,ts);
                group_word += word;

                count_word(word);

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
                count_word(group_word);
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

    if (highest_word_count == 0)
        highest_word_count = 1;

    vector< pair<double,string> > res;

    for (auto &wordent : words)
        res.push_back( pair<double,string>((double)wordent.second.count / highest_word_count,wordent.first) );

    sort(res.begin(), res.end());
    reverse(res.begin(), res.end());

    json11::Json::array resa;
    for (auto &result : res) {
        json11::Json::object reso = { {"frequency",result.first}, {"count",double(words[result.second].count)}, {"word",result.second} };
        resa.push_back(reso);
    }

    {
        string res;
        json11::Json json = json11::Json::object { {"highest count", double(highest_word_count)}, {"word count", double(words.size())}, {"results", resa} };
        json.dump(res);
        printf("%s\n",res.c_str());
    }

    return 0;
}

