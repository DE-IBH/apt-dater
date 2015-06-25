/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2015 (C) IBH IT-Service GmbH [https://www.ibh.de/apt-dater/]
 *
 * License:
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this package; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */


#ifndef _UI_H
#define _UI_H

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <glib.h>

typedef enum {
 CATEGORY,
 GROUP,
 HOST,
 PKG,
 SESSION,
} DrawType;

typedef enum {
 VK_ATTACH=1,
 VK_CONNECT=2,
 VK_DUMP=4,
 VK_UPGRADE=8,
 VK_INSTALL=16,
 VK_REFRESH=32,
 VK_KILL=64,
 VK_ERRDIAG=128,
#ifdef FEAT_HISTORY
 VK_PLAY=256,
 VK_LESS=512,
#endif
} EVisKeyMask;

typedef enum {
 SC_KEY_LEFT = 0,
 SC_KEY_LEFT2,
 SC_KEY_RIGHT,
 SC_KEY_RIGHT2,
 SC_KEY_UP,
 SC_KEY_UP2,
 SC_KEY_DOWN,
 SC_KEY_DOWN2,
 SC_KEY_HOME,
 SC_KEY_END,
 SC_KEY_PPAGE,
 SC_KEY_NPAGE,
 SC_KEY_SPACE,
 SC_KEY_RETURN,
 SC_KEY_ENTER,
 SC_KEY_PLUS,
 SC_KEY_QUIT,
 SC_KEY_HELP,
 SC_KEY_FIND,
 SC_KEY_FILTER,
 SC_KEY_ATTACH,
 SC_KEY_KILLSESS,
 SC_KEY_CONNECT,
 SC_KEY_FILETRANS,
 SC_KEY_TOGGLEDUMPS,
 SC_KEY_REFRESH,
 SC_KEY_TAGACTION,
 SC_KEY_INSTALL,
 SC_KEY_UPGRADE,
 SC_KEY_MORE,
 SC_KEY_ERRDIAG,
#ifdef FEAT_HISTORY
 SC_KEY_HISTORY,
 SC_KEY_PLAY,
 SC_KEY_LESS,
#endif
 SC_KEY_NEXTSESS,
 SC_KEY_CYCLESESS,
 SC_KEY_TAG,
 SC_KEY_TAGMATCH,
 SC_KEY_UNTAGMATCH,
 SC_KEY_UNUSED,
 SC_MAX,
} EShortCuts;

#define TAGGED_MASK 100000

typedef struct _drawnode {
#ifndef NDEBUG
 etype            _type;
#endif
 void             *p;
 DrawType         type;
 gboolean         extended;
 gboolean         selected;
 guint            scrpos;
 guint            elements;
 guint            etagged;
 int              attrs;
 struct _drawnode *parent;
} DrawNode;

struct HostFlag {
  gint flag;
  gchar *code;
  gchar *descr;
};

gboolean
ctrlKeyEnter(GList *hosts);

void doUI (GList *hosts);
void refreshUI();
void refreshDraw();
gboolean ctrlUI (GList *);
void cleanUI();
void injectKey(int);
void applyFilter(GList *hosts);
void disableInput();
void enableInput();
void reorderScrpos(guint);
gchar *getStrFromDrawNode (DrawNode *);
void freeDrawNode (DrawNode *);
void notifyUser();
void drawStatus (char *str, gboolean drawoldest);

#include "apt-dater.h"

extern gchar maintainer[48];
extern gchar *drawCategories[];
extern const struct HostFlag hostFlags[];

#ifndef KEY_RETURN
# define KEY_RETURN   13
#endif
#ifndef KEY_ESC
# define KEY_ESC      27
#endif
#ifndef KEY_FWWORD
# define KEY_FWWORD  102
#endif
#ifndef KEY_BWWORD
# define KEY_BWWORD   98
#endif
#ifndef KEY_KILLEOW
# define KEY_KILLEOW 100
#endif
#ifndef KEY_KILLBOW
# define KEY_KILLBOW  23
#endif

#define ctrl(c)             ((c)-'@')
#define remln(cols)         hline(' ', cols);
#define mvremln(y, x, cols) mvhline(y, x, ' ', cols);
#define mvwremln(win, y, x, cols) mvwhline(win, y, x, ' ', cols);

#endif /* _UI_H */
