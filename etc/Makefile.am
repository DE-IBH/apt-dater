
conffiles=apt-dater.xml hosts.xml

install-exec-hook:
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/pre-con.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/post-con.d
	ln -fs ../../../$(pkglibdir)/tmux-hint $(DESTDIR)$(sysconfdir)/apt-dater/post-con.d/
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/pre-ins.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/post-ins.d
	ln -fs ../../../$(pkglibdir)/tmux-hint $(DESTDIR)$(sysconfdir)/apt-dater/post-ins.d/
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/pre-ref.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/post-ref.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/pre-upg.d
	$(mkinstalldirs) -m 0750 $(DESTDIR)$(sysconfdir)/apt-dater/post-upg.d
	ln -fs ../../../$(pkglibdir)/tmux-hint $(DESTDIR)$(sysconfdir)/apt-dater/post-upg.d/
	$(mkinstalldirs) -m 0710 $(DESTDIR)$(sysconfdir)/apt-dater/ssh
	$(INSTALL) -m 0640 $(conffiles) $(DESTDIR)$(sysconfdir)/apt-dater/
	$(mkinstalldirs) -m 02770 $(DESTDIR)$(localstatedir)/lib/apt-dater/history/
	$(mkinstalldirs) -m 02770 $(DESTDIR)$(localstatedir)/cache/apt-dater/stats/
	$(mkinstalldirs) -m 02770 $(DESTDIR)$(localstatedir)/cache/apt-dater/tmux/

uninstall-hook:
	for conffile in $(conffiles); do \
		rm -f $(DESTDIR)$(sysconfdir)/apt-dater/"$$conffile"; \
	done
	rm -f $(DESTDIR)$(sysconfdir)/apt-dater/post-con.d/tmux-hint
	rm -f $(DESTDIR)$(sysconfdir)/apt-dater/post-ins.d/tmux-hint
	rm -f $(DESTDIR)$(sysconfdir)/apt-dater/post-upg.d/tmux-hint
	rm -f -d \
		$(DESTDIR)$(localstatedir)/cache/apt-dater/tmux/ \
		$(DESTDIR)$(localstatedir)/cache/apt-dater/stats/ \
		$(DESTDIR)$(localstatedir)/lib/apt-dater/history/ \
		$(DESTDIR)$(sysconfdir)/apt-dater/ssh \
		$(DESTDIR)$(sysconfdir)/apt-dater/post-upg.d \
		$(DESTDIR)$(sysconfdir)/apt-dater/pre-upg.d \
		$(DESTDIR)$(sysconfdir)/apt-dater/post-ref.d \
		$(DESTDIR)$(sysconfdir)/apt-dater/pre-ref.d \
		$(DESTDIR)$(sysconfdir)/apt-dater/post-ins.d \
		$(DESTDIR)$(sysconfdir)/apt-dater/pre-ins.d \
		$(DESTDIR)$(sysconfdir)/apt-dater/post-con.d \
		$(DESTDIR)$(sysconfdir)/apt-dater/pre-con.d \
		$(DESTDIR)$(sysconfdir)/apt-dater/
