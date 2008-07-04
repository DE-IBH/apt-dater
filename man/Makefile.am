man_MANS=apt-dater.8
XSLTPROC=@XSLTPROC@

EXTRA_DIST=$(man_MANS)

apt-dater.8: apt-dater.xml
	$(XSLTPROC) -''-nonet /usr/share/xml/docbook/stylesheet/nwalsh/manpages/docbook.xsl apt-dater.xml
