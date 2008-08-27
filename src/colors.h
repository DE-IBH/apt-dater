/* apt-dater - terminal-based remote package update manager
 *
 * $Id$
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
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

#ifndef _COLOR_H
#define _COLOR_H

#define COLOR_DEFAULT -1

#define PREFIX_COLOR_BRIGHT "bright"

enum {
 UI_COLOR_DEFAULT = 0,
 UI_COLOR_MENU,
 UI_COLOR_STATUS,
 UI_COLOR_SELECTOR,
 UI_COLOR_BODY,
 UI_COLOR_QUERY,
 UI_COLOR_INPUT,
 UI_COLOR_HOSTSTATUS,
 UI_COLOR_MAX,
};

void
ui_start_color();

#endif /* _COLOR_H */
