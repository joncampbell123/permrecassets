
SUBDIR=util textsearch pdfsearch htmlsearch rtfsearch imagesearch videosearch

all:
	for i in $(SUBDIR); do make -C $$i || break; done

clean:
	for i in $(SUBDIR); do make -C $$i clean || break; done

install:
	for i in $(SUBDIR); do make -C $$i install || break; done

