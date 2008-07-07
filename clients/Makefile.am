EXTRA_DIST=debian-pkg

DPKGBPKG=@DPKGBPKG@

distpkgs: $(EXTRA_DIST)

debian-pkg:
	cd debian && $(DPKGBPKG) -rfakeroot -b -uc
