/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2014 (C) IBH IT-Service GmbH [https://www.ibh.de/apt-dater/]
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

#ifndef _STATS_H
#define _STATS_H

#include <glib.h>
#include "apt-dater.h"

void
getOldestMtime(GList *hosts);

void
freeUpdates(GList *updates);

void stats_initialize(HostNode *);
gboolean refreshStats(GList *);
gboolean setStatsFileFromIOC(GIOChannel *, GIOCondition, gpointer);
gchar *getStatsFileName(const HostNode *);
gboolean prepareStatsFile(HostNode *);
gboolean getUpdatesFromStat(HostNode *);
void refreshStatsOfNode(gpointer);
gchar *getStatsContent(const HostNode *);

#endif /* _STATS_H */
