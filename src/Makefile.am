bin_PROGRAMS = apt-dater

apt_dater_SOURCES =			\
	adproto.h			\
	apt-dater.c			\
	apt-dater.h			\
	colors.c			\
	colors.h			\
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
if FEAT_TMUX				\
	tmux.h				\
	tmux.c				\
else					\
	screen.c			\
	screen.h			\
endif

apt_dater_LDADD = $(GLIB_LIBS) $(TCL_LIBS) $(LIBXML2_LIBS) $(GIO_LIBS)

AM_CFLAGS = -Wall -DPKGLIBDIR='"'$(pkglibdir)'"'
if LINUX
AM_LDFLAGS = -Wl,--as-needed
else
AM_LDFLAGS = -Wl
endif


INCLUDES = $(GLIB_CFLAGS) $(TCL_CFLAGS) $(LIBXML2_CFLAGS) $(GIO_CFLAGS)
