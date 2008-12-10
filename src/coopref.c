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

static inline gchar *distri2str(const Distri *d, gchar *buf, const gint bsize) {
    snprintf(buf, bsize-1, "%s|%s|%s", d->lsb_distributor, d->lsb_release, d->lsb_codename);

    return buf;
}

static gboolean distri_equal(gconstpointer a, gconstpointer b) {
    gchar da[0x1ff];
    gchar db[0x1ff];

    return g_str_equal(distri2str(a, da, sizeof(da)), distri2str(b, db, sizeof(db)));
}

static guint distri_hash(gconstpointer key) {
    Distri *distri = (Distri *)key;
    gchar b[0x1ff];

    return g_str_hash(distri2str(distri, b, sizeof(b)));
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

    gchar *v;
    if(((pkg->flag & HOST_STATUS_PKGUPDATE) ||
        (pkg->flag & HOST_STATUS_PKGKEPTBACK)) &&
        (pkg->data))
     v = pkg->data;
    else
     v = pkg->version;

    if(!v)
     return;

    /* Create version list if needed. */
    Version *vers = g_hash_table_lookup(ht_versions, v);
    if(!vers) {
	vers = g_malloc(sizeof(Version));
	vers->ts_first = node->last_upd;
	vers->nodes = NULL;

//    fprintf(stderr, "pkg: %s (%d versions) gets new %s @ %d\n", pkg->package, g_hash_table_size(ht_versions), v, node->last_upd);

	g_hash_table_insert(ht_versions, strdup(v), vers);
    }
    else if (vers->ts_first > node->last_upd) {
	vers->ts_first = node->last_upd;
    }

    vers->nodes = g_list_prepend(vers->nodes, node);
}

void coopref_add_host_info(HostNode *node) {
    Distri distri;

    if (!ht_distris)
	ht_distris = g_hash_table_new(distri_hash, distri_equal);

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
	return;

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



static void trigger_refresh(gpointer data, gpointer user_data) {
    HostNode *node = (HostNode *)data;

    node->category = C_REFRESH_REQUIRED;
}

GList *refresh_nodes = NULL;

static void check_refresh(gpointer data, gpointer user_data) {
    HostNode *node = (HostNode *)data;
    int *newest = (int *)user_data;

    if((node->last_upd < *newest) &&
       (g_list_find(refresh_nodes, node) == NULL))
	refresh_nodes = g_list_prepend(refresh_nodes, node);
}

static void add_refresh(gpointer key, gpointer value, gpointer user_data) {
    Version *version = (Version *)value;

    g_list_foreach(version->nodes, check_refresh, user_data);
}

static void get_newest(gpointer key, gpointer value, gpointer user_data) {
    Version *version = (Version *)value;
    int *newest = (int *)user_data;

    if (version->ts_first > *newest)
	*newest = version->ts_first;
}

static void trigger_package(gpointer key, gpointer value, gpointer user_data) {
    GHashTable *ht_versions = (GHashTable *)value;

    /* Only one version known => nothing todo. */
    if(g_hash_table_size(ht_versions) <= 1)
	return;


    /* Find the newest one. */
    gint newest = 0;
    g_hash_table_foreach(ht_versions, get_newest, &newest);

//fprintf(stderr, "more than one, newest=%d\n", newest);
    /* Any host, which has last_upd < newest needs to be refreshed. */
    g_hash_table_foreach(ht_versions, add_refresh, &newest);

//fprintf(stderr, "more than one, # refresh=%d\n", g_list_length(refresh_nodes));
    g_list_foreach(refresh_nodes, trigger_refresh, NULL);
}

static void trigger_distri(gpointer key, gpointer value, gpointer user_data) {
    GHashTable *ht_packages = (GHashTable *)value;

    g_hash_table_foreach(ht_packages, trigger_package, NULL);
}

/**
* This function is called by refreshStats when no
* host remains in IN_REFRESH state. This is the time
* to start the auto_after timer to refresh outdated
* nodes.
**/
void coopref_trigger_auto() {
//fprintf(stderr, "coopref_trigger_auto: distris = %d\n", g_hash_table_size(ht_distris));
    if(cfg->auto_after < 0)
	return;

    g_hash_table_foreach(ht_distris, trigger_distri, NULL);
}
#endif
