/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2012 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
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
#include <glib.h>
#include <glib/gstdio.h>

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

  g_snprintf(sdir, sizeof(sdir), SCREEN_SDFORMT, SCREEN_SOCKPATH, pw->pw_name);

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

gchar **
screen_new(HostNode *n, const gboolean detached) {
  gchar **_argv = (gchar **) g_malloc0(sizeof(gchar *) * 8);
  gchar *title = parse_string(cfg->screentitle, n);

  _argv[0] = g_strdup(SCREEN_BINARY);
  _argv[1] = g_strdup_printf("-%sS", detached ? "dm" : "");
  _argv[2] = g_strdup_printf(SCREEN_SOCKPRE"%s_%s_%d", n->ssh_user,
			     n->hostname, 
			     n->ssh_port);
  _argv[3] = g_strdup("-t");
  _argv[4] = title;
  _argv[5] = g_strdup("-c");
  _argv[6] = g_strdup(cfg->screenrcfile);
  _argv[7] = NULL;

  return _argv;
}

static gchar **
screen_attach_cmd(const SessNode *s, const gboolean shared) {
  gchar **_argv = (gchar **) g_malloc0(sizeof(gchar *) * 5);

  _argv[0] = g_strdup(SCREEN_BINARY);
  _argv[1] = g_strdup_printf("-r%s", shared ? "x" : "");
  _argv[2] = g_strdup_printf("%d", s->pid);
  _argv[3] = g_strdup("");
  _argv[4] = NULL;

  return _argv;
}

gboolean
screen_attach(HostNode *n, const SessNode *s, const gboolean shared) {
 gboolean r;
 GError *error = NULL;
 gchar **argv = screen_attach_cmd(s, shared);

 g_assert(n);

 r = g_spawn_sync(g_getenv ("HOME"), argv, NULL, 
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error);

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }

 g_strfreev(argv);

#ifdef FEAT_HISTORY
 if(n->parse_result && !screen_get_sessions(n)) {
    n->parse_result = FALSE;
    return history_ts_failed(n);
 }
#endif

 return FALSE;
}

static gchar **
screen_dump_cmd(const SessNode *s, const gchar *fn) {
  gchar **_argv = (gchar **) g_malloc0(sizeof(gchar *) * 7);

  _argv[0] = g_strdup(SCREEN_BINARY);
  _argv[1] = g_strdup("-S");
  _argv[2] = g_strdup_printf("%d", s->pid);
  _argv[3] = g_strdup("-X");
  _argv[4] = g_strdup("hardcopy");
  _argv[5] = g_strdup(fn);
  _argv[6] = NULL;

  return _argv;
}

gchar *
screen_get_dump(const SessNode *s) {
 gboolean r;
 GError *error = NULL;

 gchar *dump_fn = g_strdup_printf("%s/dump-XXXXXX", g_get_tmp_dir());
 gint fd = g_mkstemp(dump_fn);

 gchar **argv = screen_dump_cmd(s, dump_fn);

 if(fd == -1)
   return NULL;

 r = g_spawn_sync(g_getenv ("HOME"), argv, NULL, 
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error);

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }

 g_strfreev(argv);

 gchar *c = NULL;
 g_file_get_contents(dump_fn, &c, NULL, NULL);

 close(fd);

 g_unlink(dump_fn);

 return c;
}
