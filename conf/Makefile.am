all: apt-dater.xml.inc hosts.xml.inc screenrc.inc

clean-local:
	rm -f *.blob

%.inc: %
	xxd -i $< > $@

clean:
	rm -f *.inc
