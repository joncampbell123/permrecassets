
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
    virtual int getc_direct(void) {
        return -1;
    }
    virtual int getc_nl(void) {
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
    virtual int getc(void) {
        if (_unget) {
            _unget = false;
            return _unget_c;
        }

        return getc_direct();
    }
    virtual bool ungetc(int c) {
        if (_unget) return false;
        _unget_c = c;
        _unget = true;
        return true;
    }
    virtual bool eof(void) const {
        return _eof;
    }
    virtual bool error(void) const {
        return _error;
    }
protected:
    bool            _eof = false;
    bool            _error = false;
private:
    bool            _unget = false;
    int             _unget_c = -1;
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
    virtual int getc_direct(void) {
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
private:
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
    virtual int getc_direct(void) {
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
};

