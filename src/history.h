/* apt-dater - terminal-based remote package update manager
 *
 * $Id$
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2009 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
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

#ifndef _HISTORY_H
#define _HISTORY_H

#include "apt-dater.h"

typedef struct _historyEntry {
    gint ts;
    gint duration;
    gchar *maintainer;
    gchar *action;
    gchar *data;
} HistoryEntry;


#ifdef FEAT_HISTORY
static inline gchar *history_path(const HostNode *n) {
    return g_strdup_printf("%s/%s/history/%s:%d", g_get_user_data_dir(), PACKAGE, n->hostname, n->ssh_port);
}

static inline gchar *history_rpath(const HostNode *n) {
    gchar *p = g_strdup_printf("%s/%s/history/%s:%d/%d-%d", g_get_user_data_dir(), PACKAGE, n->hostname, n->ssh_port, (int)time(NULL), getpid());

    g_mkdir_with_parents(p, S_IRWXU);

    return p;
}

GList *history_get_entries(const HostNode *);
void history_write_meta(const gchar *, const HistoryEntry *);
void history_free_hel(GList *);

#endif

#endif /* _HISTORY_H */
