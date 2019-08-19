
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

int TextSourceBase::getc_nl(void) {
    /* for use in reading without considering newline formatting */
    int c = getc();
    if (c < 0) return -1;

    /* 0x0A             Unix    \n          LF
     * 0x0D 0x0A        DOS     \r\n        CR LF
     * 0x0D             Mac     \r          CR
     *
     * For simplicity we do NOT support oddball constructions like 0x0D 0x0D 0x0A! */
    if (c == 0x0D) {
        c = getc();
        if (c != 0x0A) {
            if (!ungetc(c)) return -1;
        }

        return '\n';
    }
    else if (c == 0x0A) {
        return '\n';
    }

    return c;
}

int TextSourceBase::getc(void) {
    if (!_unget.empty()) {
        const int c = _unget.top();
        _unget.pop();
        return c;
    }

    return getc_direct();
}

bool TextSourceBase::ungetc(int c) {
    _unget.push(c);
    return true;
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

