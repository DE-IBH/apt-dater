clean:
	

install:
	mkdir -p $(DESTDIR)/usr/lib/apt-dater-host
	for i in lib/*; do \
	    cp "$$i" $(DESTDIR)/usr/lib/apt-dater-host; \
	done
	
	mkdir -p $(DESTDIR)/usr/bin
	cp apt-dater-host $(DESTDIR)/usr/bin
	
	mkdir -p $(DESTDIR)/etc
	cp apt-dater-host.conf $(DESTDIR)/etc
