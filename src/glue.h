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

#ifndef _GLUE_H
#define _GLUE_H

#include <glib.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if !GLIB_CHECK_VERSION(2, 14, 0)
guint
g_timeout_add_seconds (guint interval, GSourceFunc function, gpointer data);
#endif

#if !GLIB_CHECK_VERSION(2, 34, 0)
gboolean
g_spawn_check_exit_status (gint exit_status, GError **error);
#endif

#endif /* _GLUE_H */
