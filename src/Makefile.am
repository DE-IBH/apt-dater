bin_PROGRAMS = apt-dater adsh

# https://reproducible-builds.org/docs/source-date-epoch/#makefile
SOURCE_DATE_EPOCH ?= $(shell date +%s)
SOURCE_DATE_UTC ?= $(shell LC_ALL=C date -u -d "@$(SOURCE_DATE_EPOCH)" 2>/dev/null || LC_ALL=C date -u -r "$(SOURCE_DATE_EPOCH)" 2>/dev/null || LC_ALL=C date -u)

if FEAT_RUNCUST
FEAT_RUNCUST_SOURCES=runcust.h runcust.c
else
FEAT_RUNCUST_SOURCES=
endif

if FEAT_TMUX
FEAT_TTYMUX_SOURCES=tmux.c tmux.h
else
FEAT_TTYMUX_SOURCES=screen.c screen.h
endif

SHARED_SOURCES =			\
	../include/adproto.h		\
	apt-dater.c			\
	apt-dater.h			\
	colors.c			\
	colors.h			\
	completion.c			\
	completion.h			\
	exec.c				\
	exec.h				\
	extern.h			\
	keyfiles.c			\
	keyfiles.h			\
	lock.c				\
	lock.h				\
	parsecmd.c			\
	parsecmd.h			\
	report.c			\
	report.h			\
	sighandler.c			\
	sighandler.h			\
	stats.c				\
	stats.h				\
	ui.c				\
	ui.h				\
	autoref.c			\
	autoref.h			\
	tag.c				\
	tag.h				\
	history.c			\
	history.h			\
	env.c				\
	env.h				\
	clusters.c			\
	clusters.h			\
	glue.c				\
	glue.h				\
	ttymux.c			\
	ttymux.h			\
	$(FEAT_RUNCUST_SOURCES) $(FEAT_TTYMUX_SOURCES)


apt_dater_SOURCES =			\
	main.c				\
	$(SHARED_SOURCES)

apt_dater_LDADD = $(GLIB_LIBS) $(TCL_LIBS) $(LIBXML2_LIBS) $(GIO_LIBS)


adsh_SOURCES =				\
	adsh.c				\
	$(SHARED_SOURCES)

adsh_LDADD = $(apt_dater_LDADD)


AM_CFLAGS = -Wall -DPKGLIBDIR='"'$(pkglibdir)'"' -DSOURCE_DATE_UTC="\"$(SOURCE_DATE_UTC)\""
if LINUX
AM_LDFLAGS = -Wl,--as-needed
else
AM_LDFLAGS = -Wl
endif


AM_CPPFLAGS = $(GLIB_CFLAGS) $(TCL_CFLAGS) $(LIBXML2_CFLAGS) $(GIO_CFLAGS)
