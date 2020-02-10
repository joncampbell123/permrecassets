
#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#include "textsrc.hpp"

#ifndef O_BINARY
#define O_BINARY (0)
#endif

using namespace std;

TextSourceBase::TextSourceBase() {
}

TextSourceBase::~TextSourceBase() {
}

bool TextSourceBase::is_open(void) const {
    return false;
}

void TextSourceBase::close(void) {
}

bool TextSourceBase::open(void) {
    return false;
}

int TextSourceBase::getc_direct(void) {
    return -1;
}

int TextSourceBase::getc(void) {
    int c;

    while (!eof() && !error() && in.size() < 8) /* 8 chars should be MORE than enough for even weird esoteric newline sequences */
        in.push_back(getc_direct()); /* WARNING: May include -1 at eof which is OK */

    if (in.empty())
        return -1;

    /* match newline sequences */
    /* ASSUME: in.size() > 0 because !in.empty() */
    assert(in.size() > 0);
    if (in[0] == 0x0D || in[0] == 0x0A) { /* newline? */
        if (in.size() >= 3 && in[0] == 0x0D && in[1] == 0x0D && in[2] == 0x0A) {
            /* strange 0x0D 0x0D 0x0A seen in some old internet docs */
            c = '\n';
            for (size_t i=0;i < 3;i++) in.pop_front();
        }
        else if (in.size() >= 2 && in[0] == 0x0D && in[1] == 0x0A) {
            /* 0x0D 0x0A MS-DOS style */
            c = '\n';
            for (size_t i=0;i < 2;i++) in.pop_front();
        }
        else {
            /* 0x0A (Unix) or 0x0D (Mac OS) */
            c = '\n';
            in.pop_front();
        }
    }
    else {
        assert(!in.empty());
        c = in.front();
        in.pop_front();
    }

    return c;
}

bool TextSourceBase::eof(void) const {
    return _eof;
}

bool TextSourceBase::error(void) const {
    return _error;
}

TextSource::TextSource() : TextSourceBase() {
}

TextSource::TextSource(const string &_path) : TextSourceBase(), path(_path) {
    open();
}

TextSource::~TextSource() {
    close();
}

bool TextSource::is_open(void) const {
    return (fd >= 0);
}

void TextSource::close(void) {
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

bool TextSource::open(void) {
    if (!is_open()) {
        if (path.empty()) return false;
        fd = ::open(path.c_str(),O_RDONLY|O_BINARY);
        if (fd < 0) return false;
    }

    return true;
}

int TextSource::getc_direct(void) {
    if (is_open() && !error() && !eof()) {
        char c;
        int rd = read(fd,&c,1);
        if (rd < 0) {
            _error = true;
            return rd;
        }
        else if (rd == 0) {
            _eof = true;
            return -1;
        }
        return (int)((unsigned char)c);
    }

    return -1;
}

TextSourceStdin::TextSourceStdin() : TextSourceBase() {
}

TextSourceStdin::~TextSourceStdin() {
}

bool TextSourceStdin::is_open(void) const {
    return true;
}

bool TextSourceStdin::open(void) {
    return true;
}

int TextSourceStdin::getc_direct(void) {
    if (is_open()) {
        char c;
        int rd = read(0/*stdin*/,&c,1);
        if (rd < 0) {
            _error = true;
            return rd;
        }
        else if (rd == 0) {
            _eof = true;
            return -1;
        }
        return (int)((unsigned char)c);
    }

    return -1;
}

