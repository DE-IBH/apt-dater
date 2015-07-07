install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(XMLSCHEMADIR)
	for dtd in apt-dater.dtd hosts.dtd report.dtd; do \
		$(INSTALL) -m 0644 "$$dtd" "$(DESTDIR)$(XMLSCHEMADIR)/"; \
	done
