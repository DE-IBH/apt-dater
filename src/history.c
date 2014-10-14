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

#include "apt-dater.h"
#include "history.h"
#include <sys/file.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef FEAT_HISTORY

HistoryEntry *history_read_meta(const gchar *fn, const gchar *tfn) {
    GKeyFile *kf = g_key_file_new();
    if(!g_key_file_load_from_file(kf, fn, G_KEY_FILE_NONE, NULL)) {
	g_key_file_free(kf);
	return NULL;
    }

    HistoryEntry *he = g_new(HistoryEntry, 1);

    he->ts = g_key_file_get_integer(kf, "Meta", "TS", NULL);
    he->maintainer = g_key_file_get_string(kf, "Meta", "Maintainer", NULL);
    he->action = g_key_file_get_string(kf, "Meta", "Action", NULL);
    he->data = g_key_file_get_string(kf, "Meta", "Data", NULL);
    he->duration = (gint) g_key_file_get_double(kf, "Meta", "Duration", NULL);
    he->errpattern = g_key_file_get_string(kf, "Meta", "ErrPattern", NULL);

    g_key_file_free(kf);

    return he;
}

void history_write_meta(const gchar *fn, const HistoryEntry *he) {
    GKeyFile *kf = g_key_file_new();

    g_key_file_set_integer(kf, "Meta", "TS", he->ts);
    if(he->maintainer)
     g_key_file_set_string(kf, "Meta", "Maintainer", he->maintainer);
    if(he->action)
     g_key_file_set_string(kf, "Meta", "Action", he->action);
    if(he->data)
     g_key_file_set_string(kf, "Meta", "Data", he->data);

    gchar *data = g_key_file_to_data(kf, NULL, NULL);
    g_key_file_free(kf);
    if(!data)
	return;

    GError *error = NULL;
    g_file_set_contents(fn, data, -1, &error);
    if (error) {
      g_error("%s", error->message);
      g_clear_error(&error);
    }

    g_free(data);
}

static gint cmp_he(gconstpointer a, gconstpointer b) {
    const HistoryEntry *ha = (HistoryEntry *)a;
    const HistoryEntry *hb = (HistoryEntry *)b;

    if(ha->ts < hb->ts)
     return -1;

    if(ha->ts == hb->ts)
     return 0;

    return 1;
}

GList *history_get_entries(const HostNode *n) {
    const gchar *fn;
    GList *hel = NULL;
    gchar *path = history_path(n);
    GDir *dir = g_dir_open(path, 0, NULL);

    if(!dir) {
     g_free(path);
     return NULL;
    }

    while((fn = g_dir_read_name(dir))) {
	gchar *meta = g_strdup_printf("%s/%s/meta", path, fn);
	gchar *timing = g_strdup_printf("%s/%s/timingfile", path, fn);
	HistoryEntry *he = history_read_meta(meta, timing);
	g_free(timing);
	g_free(meta);

	if(he) {
	 he->path = g_strdup_printf("%s/%s", path, fn);
	 hel = g_list_insert_sorted(hel, he, cmp_he);
	}
    }
    g_dir_close(dir);
    g_free(path);

    return hel;
}

HistoryEntry *history_recent_entry(const HostNode *n) {
    gchar *path = history_ts_path(n);
    gchar *meta = g_strdup_printf("%s/meta", path);
    gchar *timing = g_strdup_printf("%s/timingfile", path);

    HistoryEntry *he = history_read_meta(meta, timing);
    if(he) {
	he->path = path;
    }
    else
	g_free(path);

    g_free(timing);
    g_free(meta);

    return he;
}

static void free_hel(gpointer data, gpointer user_data) {
    HistoryEntry *he = (HistoryEntry *)data;

    g_free(he->path);
    g_free(he->maintainer);
    g_free(he->action);
    g_free(he->data);
    g_free(he->errpattern);

    g_free(data);
}

void history_free_he(HistoryEntry *he) {
    free_hel(he, NULL);
}

void history_free_hel(GList *hel) {
    g_list_foreach(hel, free_hel, NULL);
    g_list_free(hel);
}

static void history_show_cmd(gchar *cmd, gchar *param1, gchar *param2, HistoryEntry *he) {
 GError *error = NULL;
 gchar *argv[5] = {
  ENV_BINARY,
  cmd,
  param1,
  param2,
  NULL};

 if(g_spawn_sync(he->path, argv, NULL, 
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error) == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }
}

void history_show_less(HistoryEntry *he) {
 history_show_cmd("less", "-fr", "typescript", he);
}

void history_show_replay(HistoryEntry *he) {
 history_show_cmd("scriptreplay", "timingfile", NULL, he);
}

void history_show_less_search(HistoryEntry *he, gchar *pattern) {
 GError *error = NULL;
 gchar *argv[6] = {
  ENV_BINARY,
  "less",
  "-ifrp",
  pattern,
  "typescript",
  NULL};

 if(g_spawn_sync(he->path, argv, NULL, 
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error) == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }
}

gboolean history_ts_failed(HostNode *n) {
 gchar *p = history_ts_path(n);
 gchar *fn = g_strdup_printf("%s/failed", p);

 g_free(p);

 gboolean r = g_file_test(fn, G_FILE_TEST_EXISTS);

 g_free(fn);

 return r;
}

#endif
