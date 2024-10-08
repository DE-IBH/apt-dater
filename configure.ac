#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

PACKAGE=apt-dater

AC_PREREQ([2.71])
AC_INIT([apt-dater],[1.0.5],[apt-dater@ibh.de])

# include local macros
m4_include([m4lib/adl_recursive_eval.m4])

# Recursive eval some vars
adl_RECURSIVE_EVAL([$sysconfdir], sysconfdir)
adl_RECURSIVE_EVAL([$localstatedir], localstatedir)
adl_RECURSIVE_EVAL([$datarootdir], datarootdir)

# Environment
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([-Wno-portability])


case "${host_os}" in
linux*)
        build_linux="yes"
        ;;
esac
AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])

AC_CONFIG_SRCDIR([src/apt-dater.c])
AC_CONFIG_SRCDIR([src/adsh.c])
AC_CONFIG_HEADERS([config.h])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_REQUIRE_VERSION([0.19.3])
AC_CHECK_HEADERS(locale.h)

glib_module="glib-2.0 >= 2.3.6"
PKG_CHECK_MODULES(GLIB, "$glib_module", enable_glib_=yes, enable_glib_=no)
gio_module="gio-2.0 >= 2.15.0"
PKG_CHECK_MODULES(GIO, "$gio_module", enable_gio_=yes, enable_gio_=no)

AC_MSG_CHECKING(--enable-debug argument)
AC_ARG_ENABLE(debug,
	[  --enable-debug      Enable debug code.], [enable_debug="yes"],
	[enable_debug="no"])
AC_MSG_RESULT($enable_debug)
if test "$enable_debug" = "no"; then
    AC_DEFINE(NDEBUG, 1, [ Define if you want no debug code included. ])
fi

AC_MSG_CHECKING(--enable-xmlreport argument)
AC_ARG_ENABLE(xmlreport,
	[  --enable-xmlreport      Include XML reports.], [enable_xmlreport="yes"],
	[enable_xmlreport="no"])
AC_MSG_RESULT($enable_xmlreport)
PKG_CHECK_MODULES(LIBXML2, "libxml-2.0")
if test "$enable_xmlreport" = "yes"; then
	AC_DEFINE(FEAT_XMLREPORT, 1, [ Define if you want XML report support included. ])
fi

AC_MSG_CHECKING(--enable-autoref argument)
AC_ARG_ENABLE(autoref,
	[  --enable-autoref      Include 'auto refresh' support.], [enable_autoref="yes"],
	[enable_autoref="no"])
AC_MSG_RESULT($enable_autoref)
if test "$enable_autoref" = "yes"; then
    AC_DEFINE(FEAT_AUTOREF, 1, [ Define if you want 'auto refresh' support included. ])
fi

AC_MSG_CHECKING(--enable-history argument)
AC_ARG_ENABLE(history,
	[  --enable-history      Include 'history' support.], [enable_history="yes"],
	[enable_history="no"])
AC_MSG_RESULT($enable_history)
if test "$enable_history" = "yes"; then
    AC_DEFINE(FEAT_HISTORY, 1, [ Define if you want 'history' support included. ])
fi

AC_MSG_CHECKING(--enable-clusters argument)
AC_ARG_ENABLE(clusters,
	[  --enable-clusters      Include 'clusters' support.], [enable_clusters="yes"],
	[enable_clusters="no"])
AC_MSG_RESULT($enable_clusters)
if test "$enable_clusters" = "yes"; then
    AC_DEFINE(FEAT_CLUSTERS, 1, [ Define if you want 'clusters' support included. ])
fi

AC_MSG_CHECKING(--enable-runcust argument)
AC_ARG_ENABLE(runcust,
	[  --enable-runcust      Include 'run custom commands' support.], [enable_runcust="yes"],
	[enable_runcust="no"])
AC_MSG_RESULT($enable_runcust)
if test "$enable_runcust" = "yes"; then
    AC_DEFINE(FEAT_RUNCUST, 1, [ Define if you want 'run custom commands' support included. ])
fi
AM_CONDITIONAL(FEAT_RUNCUST, [test "$enable_runcust" = "yes"])

AC_SUBST(VERSION)
AC_SUBST(PACKAGE)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GIO_CFLAGS)
AC_SUBST(GIO_LIBS)
AC_SUBST(LIBXML2_CFLAGS)
AC_SUBST(LIBXML2_LIBS)
AC_PROG_INSTALL

# Some additional definitions
AC_DEFINE(HAVE_COLOR,1,[ Define if your curses library supports color. ])
AC_DEFINE(HAVE_USE_DEFAULT_COLORS,1,
	[ Define if your curses library supports function use_default_colors. ])
AC_DEFINE(HAVE_FLOCK,1,[ Define if funtion flock avaible. ])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
if eval "test x$CURSES_LIB_NAME = x"; then
    AC_CHECK_HEADERS(ncurses.h)

    if test x$enable_utf8 != xno; then
	AC_CHECK_LIB(ncursesw, get_wch, [CURSES_LIB="-lncursesw" CPPFLAGS="-I/usr/include/ncursesw $CPPFLAGS" CURSES_LIB_NAME=ncursesw CURSES_LIB_WIDE=yes])
    fi

    if eval "test x$CURSES_LIB_NAME = x"; then
	AC_CHECK_LIB(ncurses, initscr, [CURSES_LIB="-lncurses" CURSES_LIB_NAME=ncurses])
    fi
fi

if eval "test x$CURSES_LIB_NAME = x"; then
    # only to log result:
    AC_CHECK_HEADER(curses.h)

    if test x$enable_utf8 != xno; then
	AC_CHECK_LIB(curses, get_wch, [CURSES_LIB="-lcurses" CURSES_LIB_NAME=curses CURSES_LIB_WIDE=yes])
    fi

    if eval "test x$CURSES_LIB_NAME = x"; then
	 AC_CHECK_LIB(curses, initscr, [CURSES_LIB="-lcurses" CURSES_LIB_NAME=curses])
    fi
fi

if eval "test x$CURSES_LIB_NAME = x"; then
    AC_MSG_ERROR([
*** No curses lib available.  Consider getting the official ncurses
*** distribution from ftp://ftp.gnu.org/pub/gnu/ncurses if you get
*** errors compiling nano.])
else
    AC_MSG_RESULT([Using $CURSES_LIB_NAME as the curses library])
fi

LIBS="$LIBS $CURSES_LIB"

AC_CHECK_HEADER([popt.h], [], [AC_MSG_ERROR([Missing popt.h])])
AC_CHECK_LIB([popt], [poptParseArgvString], [], [AC_MSG_ERROR([Missing library popt with poptParseArgvString])])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# Where to put schema definitions
AC_MSG_CHECKING(XMLSCHEMADIR)
AC_ARG_VAR([XMLSCHEMADIR], [XML schema directory (default: DATAROOTDIR/xml/schema/apt-dater)])
if test -z "$XMLSCHEMADIR"; then
   XMLSCHEMADIR="${datarootdir}/xml/schema/apt-dater"
fi
AC_DEFINE_UNQUOTED(XML_SCHEMA_DIR,"$XMLSCHEMADIR",[ Where apt-dater's DTD schema files are stored. ])
AC_MSG_RESULT($XMLSCHEMADIR)

# URI for schema definitions
AC_MSG_CHECKING(XMLSCHEMAURI)
AC_ARG_VAR([XMLSCHEMAURI], [XML schema URI (default: file://XMLSCHEMADIR)])
if test -z "$XMLSCHEMAURI"; then
   XMLSCHEMAURI="file://${XMLSCHEMADIR}"
fi
AC_DEFINE_UNQUOTED(XML_SCHEMA_URI,"$XMLSCHEMAURI",[ URI to apt-dater's DTD schema files. ])
AC_MSG_RESULT($XMLSCHEMAURI)

# Check for TCL
AC_MSG_CHECKING(--enable-tclfilter argument)
AC_ARG_ENABLE(tclfilter,
	[  --enable-tclfilter      Include Tcl interpreter.], ,
	[enable_tclfilter="no"])
AC_MSG_RESULT($enable_tclfilter)

if test "$enable_tclfilter" = "yes"; then

  dnl on FreeBSD tclsh is a silly script, look for tclsh8.[420]
  AC_MSG_CHECKING(--with-tclsh argument)
  AC_ARG_WITH(tclsh, [  --with-tclsh=PATH       which tclsh to use (default: tclsh8.5)],
	tclsh_name="$withval"; AC_MSG_RESULT($tclsh_name),
	tclsh_name="tclsh8.5"; AC_MSG_RESULT(no))
  AC_PATH_PROG(cv_path_tcl, $tclsh_name)
  AC_SUBST(cv_path_tcl)

  dnl when no specific version specified, also try 8.2 and 8.0
  if test "X$cv_path_tcl" = "X" -a $tclsh_name = "tclsh8.4"; then
    tclsh_name="tclsh8.2"
    AC_PATH_PROG(cv_path_tcl, $tclsh_name)
  fi
  if test "X$cv_path_tcl" = "X" -a $tclsh_name = "tclsh8.2"; then
    tclsh_name="tclsh8.0"
    AC_PATH_PROG(cv_path_tcl, $tclsh_name)
  fi
  dnl still didn't find it, try without version number
  if test "X$cv_path_tcl" = "X"; then
    tclsh_name="tclsh"
    AC_PATH_PROG(cv_path_tcl, $tclsh_name)
  fi
  if test "X$cv_path_tcl" != "X"; then
    AC_MSG_CHECKING(Tcl version)
    if echo 'exit [[expr [info tclversion] < 8.0]]' | $cv_path_tcl - ; then
      tclver=`echo 'puts [[info tclversion]]' | $cv_path_tcl -`
      AC_MSG_RESULT($tclver - OK);
      tclloc=`echo 'set l [[info library]];set i [[string last lib $l]];incr i -2;puts [[string range $l 0 $i]]' | $cv_path_tcl -`

      AC_MSG_CHECKING(for location of Tcl include)
      if test "x$MACOSX" != "xyes"; then
	tclinc="$tclloc/include $tclloc/include/tcl $tclloc/include/tcl$tclver /usr/local/include /usr/include /usr/local/include/tcl$tclver /usr/include/tcl$tclver"
      else
	dnl For Mac OS X 10.3, use the OS-provided framework location
	tclinc="/System/Library/Frameworks/Tcl.framework/Headers"
      fi
      for try in $tclinc; do
	if test -f "$try/tcl.h"; then
	  AC_MSG_RESULT($try/tcl.h)
	  TCL_INC=$try
	  break
	fi
      done
      if test -z "$TCL_INC"; then
	AC_MSG_RESULT(<not found>)
	SKIP_TCL=YES
      fi
      if test -z "$SKIP_TCL"; then
	AC_MSG_CHECKING(for location of tclConfig.sh script)
	if test "x$MACOSX" != "xyes"; then
	  tclcnf=`echo $tclinc | sed s/include/lib/g`" $tclinc"
	else
	  dnl For Mac OS X 10.3, use the OS-provided framework location
	  tclcnf="/System/Library/Frameworks/Tcl.framework"
	fi
	for try in $tclcnf; do
	  if test -f $try/tclConfig.sh; then
	    AC_MSG_RESULT($try/tclConfig.sh)
	    . $try/tclConfig.sh
	    dnl use eval, because tcl 8.2 includes ${TCL_DBGX}
	    TCL_LIBS=`eval echo "$TCL_LIB_SPEC $TCL_LIBS"`
	    dnl Use $TCL_DEFS for -D_THREAD_SAFE et al.  But only use the
	    dnl "-D_ABC" items.  Watch out for -DFOO=long\ long.
	    TCL_DEFS=`echo $TCL_DEFS | sed -e 's/\\\\ /\\\\X/g' | tr ' ' '\012' | sed -e '/^-[[^D]]/d' -e '/-D[[^_]]/d' -e 's/-D_/ -D_/' | tr '\012' ' ' | sed -e 's/\\\\X/\\\\ /g'`
	    break
	  fi
	done
	if test -z "$TCL_LIBS"; then
	  AC_MSG_RESULT(<not found>)
	  AC_MSG_CHECKING(for Tcl library by myself)
	  tcllib=`echo $tclinc | sed s/include/lib/g`
	  for ext in .so .a ; do
	    for ver in "" $tclver ; do
	      for try in $tcllib ; do
		trylib=tcl$ver$ext
		if test -f $try/lib$trylib ; then
		  AC_MSG_RESULT($try/lib$trylib)
		  TCL_LIBS="-L$try -ltcl$ver"
		  if test "`(uname) 2>/dev/null`" = SunOS &&
					 uname -r | grep '^5' >/dev/null; then
		    TCL_LIBS="$TCL_LIBS -R $try"
		  fi
		  break 3
		fi
	      done
	    done
	  done
	  if test -z "$TCL_LIBS"; then
	    AC_MSG_RESULT(<not found>)
	    SKIP_TCL=YES
	  fi
	fi
	if test -z "$SKIP_TCL"; then
	  AC_DEFINE(FEAT_TCLFILTER, 1, [ Define if you want TCL filter support included. ])
	  TCL_CFLAGS="-I$TCL_INC $TCL_DEFS"
	fi
      fi
    else
      AC_MSG_RESULT(too old; need Tcl version 8.0 or later)
    fi
  fi
fi
AC_SUBST(TCL_CFLAGS)
AC_SUBST(TCL_LIBS)

# Check if `screen' is installed on your system
echo -n "checking for screen... "
if test -x /usr/bin/screen; then
	screen_binary="/usr/bin/screen"
elif test -x /usr/local/bin/screen; then
	screen_binary="/usr/local/bin/screen"
elif test -x /bin/screen; then
	screen_binary="/bin/screen"
else
	echo "not found at configure time."
	echo "Using mostly common systemdefault /usr/bin/screen."
	screen_binary="/usr/bin/screen"
fi
AC_DEFINE_UNQUOTED(SCREEN_BINARY,"$screen_binary",[ Where is the command screen located. ])
echo "$screen_binary"

# Try to find out where screen stores the sockets
echo -n "checking for screen socket path... "
if test -x $screen_binary; then
    screen_sockpath=`build/screen_sockpath "$screen_binary"`
    if test ! -d "$screen_sockpath"; then
	echo "Got invalid path '$screen_sockpath' - fallback to common systemdefault."
	screen_sockpath="/var/run/screen"
    fi
else
    echo "No screen binary available during build - fallback to common systemdefault."
    screen_sockpath="/var/run/screen"
fi
AC_DEFINE_UNQUOTED(SCREEN_SOCKPATH,"$screen_sockpath",[ Where screen stores the sockets. ])
echo "$screen_sockpath"


# Check if `tmux' support is enabled.
AC_MSG_CHECKING(--enable-tmux argument)
AC_ARG_ENABLE(tmux,
	[  --enable-tmux      Enable tmux terminal multiplexer (replaces screen).], [enable_tmux="yes"],
	[enable_tmux="no"])
AC_MSG_RESULT($enable_tmux)
if test "$enable_tmux" = "yes"; then
    # Check if `tmux' is installed on your system
    echo -n "checking for tmux... "
    if test -x /usr/bin/tmux; then
	tmux_binary="/usr/bin/tmux"
    elif test -x /usr/local/bin/tmux; then
	tmux_binary="/usr/local/bin/tmux"
    elif test -x /bin/tmux; then
	tmux_binary="/bin/tmux"
    else
	echo "not found at configure time."
	echo "Using mostly common systemdefault /usr/bin/tmux."
	tmux_binary="/usr/bin/tmux"
    fi
    AC_DEFINE_UNQUOTED(TMUX_BINARY,"$tmux_binary",[ Where is the command tmux located. ])
    echo "$tmux_binary"
    AC_DEFINE(FEAT_TMUX, 1, [ Define if you want to use tmux terminal multiplexer. ])
fi
AM_CONDITIONAL(FEAT_TMUX, [test "${enable_tmux}" = "yes"])


# Check if `env' is installed on your system (used by history feature)
if test "$enable_history" = "yes"; then
    echo -n "checking for env... "
    if test -x /usr/bin/env; then
	AC_DEFINE(ENV_BINARY,"/usr/bin/env",[ Where is the command env located. ])
	echo "/usr/bin/env"
    elif test -x /usr/local/bin/env; then
	AC_DEFINE(ENV_BINARY,"/usr/local/bin/env",[ Where is the command env located. ])
	echo "/usr/local/bin/env"
    elif test -x /bin/env; then
	AC_DEFINE(ENV_BINARY,"/bin/env",[ Where is the command env located. ])
	echo "/bin/env"
    else
	echo "not found at configure time."
	echo "Using mostly common systemdefault /usr/bin/env."
	AC_DEFINE(ENV_BINARY,"/usr/bin/env",[ Where is the command env located. ])
	echo "/usr/bin/env"
    fi
fi

# Check if `xsltproc' is installed on your system
echo -n "checking for xsltproc... "
if test -x /usr/bin/xsltproc ; then
	XSLTPROC="/usr/bin/xsltproc"
	echo $XSLTPROC
elif test -x /usr/local/bin/xsltproc; then
	XSLTPROC="/usr/local/bin/xsltproc"
	echo $XSLTPROC
elif test -x /bin/xsltproc; then
	XSLTPROC="/bin/xsltproc"
	echo $XSLTPROC
else
	XSLTPROC="/bin/true"
	echo "not found."
fi
AC_SUBST(XSLTPROC)

# Check if `docbook-xsl' is installed on your system
echo -n "checking for docbook-xsl... "
if test -d /usr/share/xml/docbook/stylesheet/nwalsh ; then
    DBXSL="/usr/share/xml/docbook/stylesheet/nwalsh"
    echo $DBXSL
else
    DBXSL=
    echo "not found."
fi
AC_SUBST(DBXSL)

AC_CONFIG_FILES([ po/Makefile.in
Makefile
etc/Makefile
lib/Makefile
lib/hosts2xml
man/Makefile
schema/Makefile
src/Makefile
conf/Makefile
conf/apt-dater.xml
conf/hosts.xml
etc/apt-dater.xml
etc/hosts.xml
build/apt-dater.spec
])
AC_OUTPUT
