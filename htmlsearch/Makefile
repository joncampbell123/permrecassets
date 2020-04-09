PROGS=html-text

CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic `pkg-config libxml-2.0 --cflags`
LDFLAGS=`pkg-config libxml-2.0 --libs`

all: $(PROGS) $(JSON11)

install: $(PROGS)
	mkdir -p $(DESTDIR)/usr/bin/
	cp -v html-text $(DESTDIR)/usr/bin/
	cp -v html-text-dictionary $(DESTDIR)/usr/bin/

clean:
	rm -f $(PROGS) *.o

html-text: html-text.o
	g++ $(LDFLAGS) -o $@ $^
html-text.o: html-text.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

