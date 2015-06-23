
EXTRA_DISTS=apt-dater.xml hosts.xml

cmd: cmd

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)/etc/apt-dater/
	$(INSTALL) -m 0640 $(EXTRA_DISTS) $(DESTDIR)/etc/apt-dater/
	$(mkinstalldirs) $(DESTDIR)/var/lib/apt-dater/history/
	$(mkinstalldirs) $(DESTDIR)/var/cache/apt-dater/stats/
	$(mkinstalldirs) $(DESTDIR)/var/cache/apt-dater/tmux/
