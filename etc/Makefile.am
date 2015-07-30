
EXTRA_DISTS=apt-dater.xml hosts.xml

cmd: cmd

install-exec-hook:
	$(mkinstalldirs) -m 0750 $(DESTDIR)/etc/apt-dater/
	$(mkinstalldirs) -m 0750 $(DESTDIR)/etc/apt-dater/pre-con.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)/etc/apt-dater/post-con.d
	ln -s ../../../$(pkglibdir)/tmux-hint $(DESTDIR)/etc/apt-dater/post-con.d/
	$(mkinstalldirs) -m 0750 $(DESTDIR)/etc/apt-dater/pre-ins.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)/etc/apt-dater/post-ins.d
	ln -s ../../../$(pkglibdir)/tmux-hint $(DESTDIR)/etc/apt-dater/post-ins.d/
	$(mkinstalldirs) -m 0750 $(DESTDIR)/etc/apt-dater/pre-ref.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)/etc/apt-dater/post-ref.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)/etc/apt-dater/pre-upg.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)/etc/apt-dater/post-upg.d
	ln -s ../../../$(pkglibdir)/tmux-hint $(DESTDIR)/etc/apt-dater/post-upg.d/
	$(mkinstalldirs) -m 0710 $(DESTDIR)/etc/apt-dater/ssh
	$(INSTALL) -m 0640 $(EXTRA_DISTS) $(DESTDIR)/etc/apt-dater/
	$(mkinstalldirs) -m 02770 $(DESTDIR)/var/lib/apt-dater/history/
	$(mkinstalldirs) -m 02770 $(DESTDIR)/var/cache/apt-dater/stats/
	$(mkinstalldirs) -m 02770 $(DESTDIR)/var/cache/apt-dater/tmux/
