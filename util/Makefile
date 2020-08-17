PROGS=mount-is-readonly mount-rel-path pra-fs-scan pra-fs-browse

CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic 
LDFLAGS=

all: $(PROGS)

install: $(PROGS)
	mkdir -p $(DESTDIR)/usr/bin/
	cp -v pra-fs-browse $(DESTDIR)/usr/bin/
	cp -v mount-rel-path $(DESTDIR)/usr/bin/
	cp -v mount-is-readonly $(DESTDIR)/usr/bin/
	cp -v permasset-identify $(DESTDIR)/usr/bin/
	cp -v pra-fs-scan-db-init.sh $(DESTDIR)/usr/bin/
	cp -v pra-fs-scan-db-view.sh $(DESTDIR)/usr/bin/
	cp -v pra-fs-search-db-view.sh $(DESTDIR)/usr/bin/
	cp -v pra-fs-scan-db-checkpoint.sh $(DESTDIR)/usr/bin/
	cp -v permasset-scan $(DESTDIR)/usr/bin/
	cp -v pra-fs-scan $(DESTDIR)/usr/bin/

clean:
	rm -f $(PROGS) *.o *.a *.db

lib_prluuid.o: lib_prluuid.cpp
	g++ $(CXXFLAGS) -c -o $@ $^
lib_prl_udev.o: lib_prl_udev.cpp
	g++ $(CXXFLAGS) -c -o $@ $^
lib_prl_nodes.o: lib_prl_nodes.cpp
	g++ $(CXXFLAGS) -c -o $@ $^
lib_splitpath.o: lib_splitpath.cpp
	g++ $(CXXFLAGS) -c -o $@ $^
lib_procmount.o: lib_procmount.cpp
	g++ $(CXXFLAGS) -c -o $@ $^
lib_fs_isrdonly.o: lib_fs_isrdonly.cpp
	g++ $(CXXFLAGS) -c -o $@ $^
lib_cpp_realpath.o: lib_cpp_realpath.cpp
	g++ $(CXXFLAGS) -c -o $@ $^
lib_path_rel_label.o: lib_path_rel_label.cpp
	g++ $(CXXFLAGS) -c -o $@ $^
libpermrecassetutil.a: lib_procmount.o lib_path_rel_label.o lib_prluuid.o lib_splitpath.o lib_prl_nodes.o lib_cpp_realpath.o lib_fs_isrdonly.o lib_prl_udev.o
	rm -f $@
	ar r $@ $^

pra-fs-browse: pra-fs-browse.o libpermrecassetutil.a
	g++ $(LDFLAGS) -o $@ $^ -luuid `pkg-config --libs sqlite3`
pra-fs-browse.o: pra-fs-browse.cpp
	g++ $(CXXFLAGS) -c -o $@ $^ `pkg-config --cflags sqlite3`

pra-fs-scan: pra-fs-scan.o libpermrecassetutil.a
	g++ $(LDFLAGS) -o $@ $^ -luuid `pkg-config --libs sqlite3`
pra-fs-scan.o: pra-fs-scan.cpp
	g++ $(CXXFLAGS) -c -o $@ $^ `pkg-config --cflags sqlite3`

mount-is-readonly: mount-is-readonly.o libpermrecassetutil.a
	g++ $(LDFLAGS) -o $@ $^
mount-is-readonly.o: mount-is-readonly.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

mount-rel-path: mount-rel-path.o libpermrecassetutil.a
	g++ $(LDFLAGS) -o $@ $^
mount-rel-path.o: mount-rel-path.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

