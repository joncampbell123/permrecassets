PROGS=mount-is-readonly mount-rel-path

CXXFLAGS=
LDFLAGS=

all: $(PROGS)

install: $(PROGS)
	mkdir -p $(DESTDIR)/usr/bin/
	cp -v mount-rel-path $(DESTDIR)/usr/bin/
	cp -v mount-is-readonly $(DESTDIR)/usr/bin/
	cp -v permasset-identify $(DESTDIR)/usr/bin/
	cp -v permasset-scan $(DESTDIR)/usr/bin/

clean:
	rm -f $(PROGS) *.o *.a

lib_procmount.o: lib_procmount.cpp
	g++ $(CXXFLAGS) -c -o $@ $^
libpermrecassetutil.a: lib_procmount.o
	rm -f $@
	ar r $@ $^

mount-is-readonly: mount-is-readonly.o
	g++ $(LDFLAGS) -o $@ $^
mount-is-readonly.o: mount-is-readonly.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

mount-rel-path: mount-rel-path.o libpermrecassetutil.a
	g++ $(LDFLAGS) -o $@ $^
mount-rel-path.o: mount-rel-path.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

