man_MANS=apt-dater.8 apt-dater.html apt-dater.config.5 apt-dater.config.html apt-dater-host.1

XSLTPROC=@XSLTPROC@
DBXSL=@DBXSL@

EXTRA_DIST=$(man_MANS)

%.html: %.xml
	$(XSLTPROC) -o $@ -''-nonet $(DBXSL)/html/docbook.xsl $<

%.5: %.xml
	$(XSLTPROC) -''-nonet $(DBXSL)/manpages/docbook.xsl $<

%.8: %.xml
	$(XSLTPROC) -''-nonet $(DBXSL)/manpages/docbook.xsl $<
