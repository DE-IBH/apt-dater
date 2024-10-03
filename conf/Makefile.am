INCS = apt-dater.xml.inc hosts.xml.inc screenrc.inc tmux.conf.inc

EXTRA_DIST = apt-dater.xml hosts.xml screenrc tmux.conf

CLEANFILES=$(INCS)

all: $(INCS)

$(INCS): %.inc: %
	xxd --name "`basename $<`" -i $< > $@

clean:
	rm -f *.inc
