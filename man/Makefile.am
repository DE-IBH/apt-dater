man_MANS=apt-dater.8

XSLTPROC=@XSLTPROC@
DBXSL=@DBXSL@

EXTRA_DIST=$(man_MANS)

apt-dater.8: apt-dater.xml
	$(XSLTPROC) -''-nonet $(DBXSL) apt-dater.xml
