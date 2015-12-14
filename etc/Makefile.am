
EXTRA_DISTS=apt-dater.xml hosts.xml

cmd: cmd

install-exec-hook:
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/pre-con.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/post-con.d
	ln -s ../../../$(pkglibdir)/tmux-hint $(DESTDIR)$(sysconfdir)/apt-dater/post-con.d/
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/pre-ins.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/post-ins.d
	ln -s ../../../$(pkglibdir)/tmux-hint $(DESTDIR)$(sysconfdir)/apt-dater/post-ins.d/
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/pre-ref.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/post-ref.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/pre-upg.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/post-upg.d
	ln -s ../../../$(pkglibdir)/tmux-hint $(DESTDIR)$(sysconfdir)/apt-dater/post-upg.d/
	$(mkinstalldirs) -m 0710 $(DESTDIR)$(sysconfdir)/apt-dater/ssh
	$(INSTALL) -m 0640 $(EXTRA_DISTS) $(DESTDIR)$(sysconfdir)/apt-dater/
	$(mkinstalldirs) -m 02770 $(DESTDIR)$(localstatedir)/lib/apt-dater/history/
	$(mkinstalldirs) -m 02770 $(DESTDIR)$(localstatedir)/cache/apt-dater/stats/
	$(mkinstalldirs) -m 02770 $(DESTDIR)$(localstatedir)/cache/apt-dater/tmux/
