man_MANS=apt-dater.8 apt-dater.conf.5

XSLTPROC=@XSLTPROC@
DBXSL=@DBXSL@

EXTRA_DIST=$(man_MANS)

apt-dater.8: apt-dater.xml
	$(XSLTPROC) -''-nonet $(DBXSL) apt-dater.xml

apt-dater.conf.5: apt-dater.conf.xml
	$(XSLTPROC) -''-nonet $(DBXSL) apt-dater.conf.xml
