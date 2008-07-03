man_MANS=apt-dater.8
XSLTPROC=@XSLTPROC@

EXTRA_DIST=$(man_MANS)

apt-dater.8: apt-dater.xml
	$(XSLTPROC) -''-nonet /usr/share/xml/docbook/stylesheet/nwalsh/manpages/docbook.xsl apt-dater.xml

#apt_daterINSTALL-MAN:
#	for s in `seq 1 8`; do \
#	    for f in *.$s; do \
#		$(INSTALL_DATA) $f $(DESTDIR)$(mandir)/man$s/$f \
#		$(INSTALL) $i \
#	    done \
#	done
