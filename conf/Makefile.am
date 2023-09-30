INCS = apt-dater.xml.inc hosts.xml.inc screenrc.inc tmux.conf.inc

all: $(INCS)

clean-local:
	rm -f *.blob

$(INCS): %.inc: %
	xxd -i $< > $@

clean:
	rm -f *.inc
