
SUBDIR=textsearch pdfsearch htmlsearch rtfsearch

all:
	for i in $(SUBDIR); do make -C $$i || break; done

clean:
	for i in $(SUBDIR); do make -C $$i clean || break; done

install:
	for i in $(SUBDIR); do make -C $$i install || break; done

