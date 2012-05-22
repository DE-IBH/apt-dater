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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>

#ifdef FEAT_CLUSTERS

typedef struct _clusterEntry {
    gchar *name;
    HostNode *holdby;
} ClusterEntry;

static GList *clusters = NULL;

void cluster_host_reset(HostNode *n) {
    g_list_free(n->clusters);
    n->clusters = NULL;
}

static gint findCluster(gconstpointer a, gconstpointer b) {
  return g_ascii_strcasecmp(a, b);
}

void cluster_host_add(HostNode *n, gchar *c) {
    GList *s = g_list_find_custom(clusters, c, findCluster);
    if(s == NULL)
	clusters = g_list_append(clusters, g_strdup(c));
}

gboolean cluster_host_acquire(HostNode *n) {
    return TRUE;
}

void cluster_host_release(HostNode *n) {
}

#endif /* FEAT_CLUSTERS */
