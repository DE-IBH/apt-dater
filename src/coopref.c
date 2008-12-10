/* apt-dater - terminal-based remote package update manager
 *
 * $Id$
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef FEAT_COOPREF

/**
* We record installed/updatable packages on the hosts. When a host has been
* refreshed, we look at each package and put it in the following tree (this
* will be used later to detect if some hosts are outdated running the same
* distri):
*
* [ht_distris]
*   |
*   +--[Debian/4.0/etch]
*   |    +--[package1]
*   |    |    +--[version1]->(host1, host2, host3)
*   |    |    +--[version2]->(host5, host6, ...)
*   |    |    |
*   |    |   ...
*   |    +--[package2]
*   |    |
*   |   ...
*   +--[CentOS/4.7/Final]
*   |
*  ...
*
* [ ]: GHashTable
* ( ): GList
*
**/

#include <glib.h>

typedef struct _distri {
 gchar     *lsb_distributor;
 gchar     *lsb_release;
 gchar     *lsb_codename;
} Distri;

typedef struct _version {
 time_t ts_first;
 GList *nodes;
} Version;

static GHashTable *ht_distris = NULL;

static gboolean cmp_distris(gconstpointer a, gconstpointer b) {
    int ret;

#define CMP_DIST(key, a, b) \
    ret = strcmp(((Distri *)a)->key, ((Distri *)b)->key); \
    if (ret) \
	return ret;

    CMP_DIST(lsb_distributor, a, b);
    CMP_DIST(lsb_release, a, b);
    CMP_DIST(lsb_codename, a, b);

    return 0;
}

static void add_pkg(gpointer data, gpointer user_data) {
    PkgNode *pkg = (PkgNode *)data;
    GHashTable *ht_packages = (GHashTable *)(((gpointer *)user_data)[0]);
    HostNode *node = (HostNode *)(((gpointer *)user_data)[1]);

    /* Create packages hashtable if needed. */
    GHashTable *ht_versions = g_hash_table_lookup(ht_packages, pkg->package);
    if(!ht_versions) {
	ht_versions = g_hash_table_new(g_str_hash, g_str_equal);

	g_hash_table_insert(ht_packages, strdup(pkg->package), ht_versions);
    }

    /* Create version list if needed. */
    Version *vers = g_hash_table_lookup(ht_versions, pkg->version);
    if(!vers) {
	vers = g_malloc(sizeof(Version));
	vers->ts_first = node->last_upd;
	vers->nodes = NULL;

	g_hash_table_insert(ht_versions, strdup(pkg->version), vers);
    }

    vers->nodes = g_list_prepend(vers->nodes, node);
}

void coopref_add_host_info(HostNode *node) {
    Distri distri;

    if (!ht_distris)
	ht_distris = g_hash_table_new(g_str_hash, cmp_distris);

#define ASSIGN_DIST(d, n, f) \
    (d).lsb_distributor = f((n)->lsb_distributor); \
    (d).lsb_release = f((n)->lsb_release); \
    (d).lsb_codename = f((n)->lsb_codename);

    ASSIGN_DIST(distri, node, );

    /* Create distri hashtable if needed. */
    GHashTable *ht_packages = g_hash_table_lookup(ht_distris, &distri);
    if(!ht_packages) {
	ht_packages = g_hash_table_new(g_str_hash, g_str_equal);

	Distri *ndistri = g_malloc(sizeof(Distri));
	ASSIGN_DIST(*ndistri, node, g_strdup);

	g_hash_table_insert(ht_distris, ndistri, ht_packages);
    }

    /* Traverse package list and count installed versions. */
    gpointer data[2];
    data[0] = ht_packages;
    data[1] = node;
    g_list_foreach(node->packages, add_pkg, data);
}

static void rem_pkg(gpointer data, gpointer user_data) {
    PkgNode *pkg = (PkgNode *)data;
    GHashTable *ht_packages = (GHashTable *)(((gpointer *)user_data)[0]);
    HostNode *node = (HostNode *)(((gpointer *)user_data)[1]);

    /* Lookup packages hashtable. */
    GHashTable *ht_versions = g_hash_table_lookup(ht_packages, pkg->package);
    if(!ht_versions)
	return;

    /* Lookup version list. */
    Version *vers = g_hash_table_lookup(ht_versions, pkg->version);
    if(!vers)
	return;

    vers->nodes = g_list_remove(vers->nodes, node);
}

void coopref_rem_host_info(HostNode *node) {
    Distri distri;

    if (!ht_distris)
	ht_distris = g_hash_table_new(g_str_hash, cmp_distris);

#define ASSIGN_DIST(d, n, f) \
    (d).lsb_distributor = f((n)->lsb_distributor); \
    (d).lsb_release = f((n)->lsb_release); \
    (d).lsb_codename = f((n)->lsb_codename);

    ASSIGN_DIST(distri, node, );

    /* Lookup distri hashtable. */
    GHashTable *ht_packages = g_hash_table_lookup(ht_distris, &distri);
    if(!ht_packages)
	return;

    /* Traverse package list and count installed versions. */
    gpointer data[2];
    data[0] = ht_packages;
    data[1] = node;
    g_list_foreach(node->packages, rem_pkg, data);
}

#endif
