EXTRA_DISTS=cmd hosts2xml ssh-addonce tmux-hint

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(pkglibdir)
	$(INSTALL) $(EXTRA_DISTS) $(DESTDIR)$(pkglibdir)
	$(INSTALL) adsh $(DESTDIR)$(bindir)
