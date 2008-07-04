SUBDIRS = clients man src

AUTOMAKE_OPTIONS = 							\
	1.9								\
	dist-bzip2

changelog:
	LANG=C svn log > ChangeLog

distclean-local:
	rm -rf *.cache *~ .project .settings .cdtproject
