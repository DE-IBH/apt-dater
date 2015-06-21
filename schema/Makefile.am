install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(XMLSCHEMADIR)
	for dtd in apt-dater.dtd hosts.dtd report.dtd; do \
		$(INSTALL) "$$dtd" "$(DESTDIR)$(XMLSCHEMADIR)/"; \
	done
