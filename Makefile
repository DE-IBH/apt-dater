# Makefile for apt-dater
#

CC = gcc
LIBS = -lcurses -lglib-2.0
LIBDIR = -L/usr/lib
#CFLAGS = $(INCDIR) -ggdb -O3 -fomit-frame-pointer -march=i686 -fforce-addr -funroll-loops
CFLAGS = $(INCDIR) -ggdb
INCDIR = -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
BIN = apt-dater
OBJ = apt-dater.o keyfiles.o ui.o stats.o exec.o
SRC = apt-dater.c keyfiles.c ui.c stats.c exec.c apt-dater.h extern.h ui.h

apt-dater: $(OBJ)
	   $(CC) $(CFLAGS) $(INCDIR) $(LIBDIR) $(LIBS) $(OBJ) -o $(BIN)

clean:
	   rm -f $(OBJ) $(BIN)
