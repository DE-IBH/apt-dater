/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2012 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
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

#include "apt-dater.h"
#include "clusters.h"
#include <sys/file.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>

#ifdef FEAT_CLUSTERS

static gint findCluster(gconstpointer a, gconstpointer b) {
  return g_ascii_strcasecmp(a, b);
}

void cluster_host_add(HostNode *n, const gchar *c) {
    if(g_list_find_custom(n->clusters, c, findCluster) == NULL) {
	n->clusters = g_list_insert_sorted(n->clusters, g_strdup(c), findCluster);
	n->status = n->status | HOST_STATUS_CLUSTERED;
    }
}

static void freeCluster(gchar *data, gpointer *user_data) {
    if(data)
	g_free(data);
}

void cluster_host_reset(HostNode *n) {
    if(n && n->clusters) {
	g_list_foreach(n->clusters, (GFunc) freeCluster, NULL);
	g_list_free(n->clusters);
	n->clusters = NULL;
    }
}

#endif /* FEAT_CLUSTERS */
