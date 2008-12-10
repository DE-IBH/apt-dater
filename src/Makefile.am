bin_PROGRAMS = apt-dater

apt_dater_SOURCES =			\
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
	screen.c			\
	screen.h			\
	sighandler.c			\
	sighandler.h			\
	stats.c				\
	stats.h				\
	ui.c				\
	ui.h				\
	coopref.c			\
	coopref.h

apt_dater_LDADD = $(GLIB_LIBS) $(TCL_LIBS) $(LIBXML2_LIBS)

INCLUDES = $(GLIB_CFLAGS) $(TCL_CFLAGS) $(LIBXML2_CFLAGS)
