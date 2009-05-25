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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <glib-2.0/glib.h>
#include <glib-2.0/glib/gstdio.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "screen.h"
#include "parsecmd.h"
#include "history.h"

static struct passwd *pw = NULL;

const static gchar *
screen_get_sdir() {
  static gchar sdir[256];

  if (!pw)
    pw = getpwuid(getuid());
  if (!pw)
    return NULL;

  g_snprintf(sdir, sizeof(sdir), SCREEN_SDFORMT, SCREEN_SOCKDIR, pw->pw_name);

  return sdir;
}

gboolean
screen_get_sessions(HostNode *n) {
  if (n->screens) {
    g_list_free(n->screens);
    n->screens = NULL;
  }

  const gchar *sdir = screen_get_sdir();
  if (!sdir)
    return FALSE;

  GDir *d = g_dir_open(sdir, 0, NULL);
  if (!d) {
   return FALSE;
  }

  gchar *search = g_strdup_printf(SCREEN_SOCKPRE"%s_%s_%d", n->ssh_user, n->hostname, n->ssh_port);

  const gchar *f;
  while ((f = g_dir_read_name(d))) {
    gchar *fn = g_strdup_printf("%s/%s", sdir, f);

    if (g_file_test(fn, G_FILE_TEST_EXISTS)) {
      gint pid = atoi(f);
      char *name = strchr(f, '.');

      if ((pid > 1) &&
	  (name) &&
	  (strcmp(name+1, search) == 0)) {

	SessNode *s = g_new0(SessNode, 1);
#ifndef NDEBUG
	s->_type = T_SESSNODE;
#endif
	s->pid = pid;
	stat(fn, &s->st);

	n->screens = g_list_prepend(n->screens, s);
      }
    }

    g_free(fn);
  }
  g_dir_close(d);

  g_free(search);

  return g_list_length(n->screens) > 0;
}

gchar *
screen_new(HostNode *n, const gboolean detached, const HistoryEntry *he) {
  if (!cfg->use_screen) {
#ifdef FEAT_HISTORY
    if(cfg->record_history && he) {
     gchar *hp = history_rec_path(n);
     gchar *fn_meta = g_strdup_printf("%s/meta", hp);
     history_write_meta(fn_meta, he);
     g_free(fn_meta);

     gchar *cmd = g_strdup_printf(PKGLIBDIR"/script+%s+%s+", hp, cfg->history_errpattern);
     g_free(hp);

     return cmd;
    }
    else
#endif
     return g_strdup("");
  }

  gchar *title = parse_string(cfg->screentitle, n);

  gchar *cmd;

#ifdef FEAT_HISTORY
  if(cfg->record_history && he) {
    gchar *hp = history_rec_path(n);
    gchar *fn_meta = g_strdup_printf("%s/meta", hp);
    history_write_meta(fn_meta, he);
    g_free(fn_meta);

    cmd = g_strdup_printf(SCREEN_BINARY"+-%sS+"SCREEN_SOCKPRE"%s_%s_%d"	\
			       "+-t+%s+-c+%s+"PKGLIBDIR"/script+%s+%s+",
			       detached ? "dm" : "",
			       n->ssh_user, n->hostname, n->ssh_port,
			       title,
			       cfg->screenrcfile,
			       hp, cfg->history_errpattern);

    g_free(hp);
  } else
#endif
    cmd = g_strdup_printf(SCREEN_BINARY"+-%sS+"SCREEN_SOCKPRE"%s_%s_%d"	\
			       "+-t+%s+-c+%s+",
			       detached ? "dm" : "",
			       n->ssh_user, n->hostname, n->ssh_port,
			       title,
			       cfg->screenrcfile);

  g_free(title);

  return cmd;
}

static gchar *
screen_attach_cmd(const SessNode *s, const gboolean shared) {
  return g_strdup_printf(SCREEN_BINARY"+-r%s+%d+", shared ? "x" : "", s->pid);
}

gboolean
screen_attach(HostNode *n, const SessNode *s, const gboolean shared) {
 gboolean r;
 GError *error = NULL;
 gchar *cmd = screen_attach_cmd(s, shared);
 gchar **argv = NULL;

 g_assert(n);

 argv = g_strsplit(cmd, "+", 0);

 r = g_spawn_sync(g_getenv ("HOME"), argv, NULL, 
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error);

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }

 g_free(cmd);
 g_strfreev(argv);

#ifdef FEAT_HISTORY
 if(n->parse_result && !screen_get_sessions(n)) {
    n->parse_result = FALSE;
    return history_ts_failed(n);
 }
#endif

 return FALSE;
}

static gchar *
screen_dump_cmd(const SessNode *s, const gchar *fn) {
  return g_strdup_printf(SCREEN_BINARY"+-S+%d+-X+hardcopy+%s", s->pid, fn);
}

gchar *
screen_get_dump(const SessNode *s) {
 gboolean r;
 GError *error = NULL;
 gchar **argv = NULL;

 gchar *dump_fn = g_strdup_printf("%s/dump-XXXXXX", g_get_tmp_dir());
 gint fd = g_mkstemp(dump_fn);

 if(fd == -1)
   return NULL;

 gchar *cmd = screen_dump_cmd(s, dump_fn);
 if(!cmd) {
  g_unlink(dump_fn);
  close(fd);
  return NULL;
 }

 argv = g_strsplit(cmd, "+", 0);

 r = g_spawn_sync(g_getenv ("HOME"), argv, NULL, 
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error);

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }

 g_free(cmd);
 g_strfreev(argv);

 gchar *c = NULL;
 g_file_get_contents(dump_fn, &c, NULL, NULL);

 close(fd);

 g_unlink(dump_fn);

 return c;
}
