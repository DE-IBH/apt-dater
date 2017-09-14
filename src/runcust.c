/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2017 (C) IBH IT-Service GmbH [https://www.ibh.de/apt-dater/]
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


#include <glib.h>

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef FEAT_RUNCUST

#include "apt-dater.h"
#include "colors.h"
#include "runcust.h"

void runCustom() {
 WINDOW *w = newwin(LINES-3, COLS, 2, 0);

 wattron(w, uicolors[UI_COLOR_QUERY]);
 mvwaddstr(w, 1, 0, _("Select command to run:"));
 wattroff(w, uicolors[UI_COLOR_QUERY]);

 waddstr(w, "\n");

 wattroff(w, uicolors[UI_COLOR_INPUT]);
}

#endif /* FEAT_RUNCUST */
