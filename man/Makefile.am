HTMLS=apt-dater.html apt-dater.xml.html
MAN5S=apt-dater.xml.5
MAN8S=apt-dater.8

man_MANS=$(MAN8S) $(MAN5S) $(HTMLS)

XSLTPROC=@XSLTPROC@
DBXSL=@DBXSL@

EXTRA_DIST=$(man_MANS)

$(HTMLS): %.html: %.xml
	$(XSLTPROC) -o $@ -''-nonet $(DBXSL)/html/docbook.xsl $<

$(MAN5S): %.5: %.xml
	$(XSLTPROC) -''-nonet $(DBXSL)/manpages/docbook.xsl $<

$(MAN8S): %.8: %.xml
	$(XSLTPROC) -''-nonet $(DBXSL)/manpages/docbook.xsl $<
