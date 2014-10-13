if LINUX
all: apt-dater.blob hosts.blob screenrc.blob
else
all:
endif

apt-dater.blob: apt-dater.config
	$(LD) -r -b binary -o $@ $<

hosts.blob: hosts.config
	$(LD) -r -b binary -o $@ $<

screenrc.blob: screenrc
	$(LD) -r -b binary -o $@ $<

clean:
	rm -f apt-dater.blob hosts.blob screenrc.blob
