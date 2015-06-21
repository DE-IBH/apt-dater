
EXTRA_DISTS=cmd conf2xml

cmd: cmd

conf2xml:
	chmod +x conf2xml

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(pkglibdir)
	$(INSTALL) cmd $(DESTDIR)$(pkglibdir)
	$(INSTALL) conf2xml $(DESTDIR)$(pkglibdir)
