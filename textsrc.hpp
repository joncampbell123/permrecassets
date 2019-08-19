
#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <stack>
#include <deque>

class TextSourceBase {
public:
    TextSourceBase();
    virtual ~TextSourceBase();
    virtual bool is_open(void) const;
    virtual void close(void);
    virtual bool open(void);
    virtual int getc_direct(void);
    virtual int getc(void);
    virtual bool eof(void) const;
    virtual bool error(void) const;
protected:
    bool                _eof = false;
    bool                _error = false;
private:
    std::deque<int>     in;
};

class TextSource : public TextSourceBase {
public:
    TextSource();
    TextSource(const std::string &_path);
    virtual ~TextSource();
    virtual bool is_open(void) const;
    virtual void close(void);
    virtual bool open(void);
    virtual int getc_direct(void);
private:
public:
    int             fd = -1;
    std::string     path;
};

class TextSourceStdin : public TextSourceBase {
public:
    TextSourceStdin();
    virtual ~TextSourceStdin();
    virtual bool is_open(void) const;
    virtual bool open(void);
    virtual int getc_direct(void);
};

