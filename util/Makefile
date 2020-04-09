PROGS=mount-is-readonly

CXXFLAGS=
LDFLAGS=

all: $(PROGS)

install: $(PROGS)
	mkdir -p $(DESTDIR)/usr/bin/
	cp -v mount-is-readonly $(DESTDIR)/usr/bin/
	cp -v permasset-identify $(DESTDIR)/usr/bin/

clean:
	rm -f $(PROGS) *.o

mount-is-readonly: mount-is-readonly.o
	g++ $(LDFLAGS) -o $@ $^
mount-is-readonly.o: mount-is-readonly.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

