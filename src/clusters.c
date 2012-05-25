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

typedef struct _clusterEntry {
    gchar *name;
    gchar *fnlock;
    gint fdlock;
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

void cluster_host_add(HostNode *n, const gchar *c) {
    GList *s = g_list_find_custom(clusters, c, findCluster);
    if(s == NULL) {
	ClusterEntry *e = malloc(sizeof(ClusterEntry));
	s = clusters = g_list_prepend(clusters, g_strdup(c));
	s->data = e;
	e->name = g_strdup(c);;
	e->fnlock = g_strdup_printf("%s/cluster:%s.lck", cfg->statsdir, e->name);
	e->fdlock = -1;
	e->holdby = NULL;
    }
    n->clusters = g_list_append(n->clusters, s->data);
}

gint cluster_lock_acquire(HostNode *n, ClusterEntry *c) {
    int r;

    if(c->fdlock < 0) {
	if((c->fdlock = open(c->fnlock, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR))< 0) {
	    g_error("%s: %s", c->fnlock, strerror(errno));
	    return EXIT_FAILURE;
	}

	fcntl(c->fdlock, F_SETFD, FD_CLOEXEC | fcntl(c->fdlock, F_GETFD));
    }

#ifdef HAVE_FLOCK
    do {
	r = flock(c->fdlock, LOCK_EX | LOCK_NB);
    } while((r==-1) && (errno == EINTR));

    if((r==-1) && (errno != EWOULDBLOCK)) {
	g_error(_("Failed to get lockfile %s: %s"),
		c->fnlock, g_strerror(errno));
    }
#else
    g_warning(_("Can't lock to file %s because function flock() is missing!"),
		c->fnlock);
    return EXIT_FAILURE;
#endif

    if(r == -1) {
	close(c->fdlock);
	c->fdlock = -1;
    }
    else {
	c->holdby = n;
    }

    return r;
}

gboolean cluster_host_acquire(HostNode *n) {
    if(n->clusters == NULL)
	return TRUE;

    GList *c = n->clusters;
    while(c != NULL) {
	if(cluster_lock_acquire(n, c->data) != 0) {
	    cluster_host_release(n);
	    return FALSE;
	}
	c = c->next;
    }

    return TRUE;
}

void cluster_lock_release(HostNode *n, ClusterEntry *c) {
    if(c->holdby != n)
	return;

#ifdef HAVE_FLOCK
    flock(c->fdlock, LOCK_UN);
#endif

    close(c->fdlock);
    c->fdlock = -1;
}

void cluster_host_release(HostNode *n) {
    if(n->clusters == NULL)
	return;

    GList *c = n->clusters;
    while(c->next != NULL)
	c = c->next;

    while(c != NULL) {
	cluster_lock_release(n, c->data);
	c = c->prev;
    }
}

#endif /* FEAT_CLUSTERS */
