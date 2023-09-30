/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2015 (C) IBH IT-Service GmbH [https://www.ibh.de/apt-dater/]
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
#include <gio/gio.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef FEAT_TMUX

#include "tmux.h"
#include "parsecmd.h"
#include "history.h"
#include "stats.h"

static struct passwd *pw = NULL;

const static gchar *
tmux_get_sdir() {
  static gchar sdir[256];

  if (!pw)
    pw = getpwuid(getuid());
  if (!pw)
    return NULL;

  g_snprintf(sdir, sizeof(sdir), TMUX_SDFORMT, TMUX_SOCKPATH, pw->pw_name);

  return sdir;
}

void
tmux_changed(GFileMonitor     *monitor,
	     GFile            *file,
	     GFile            *other_file,
	     GFileMonitorEvent event_type,
	     gpointer          user_data) {
  HostNode *n = user_data;

  g_assert(n);

  GList *nl = g_list_append(NULL, n);
  refreshStats(nl);
  nl = g_list_remove(nl, n);
}

void
tmux_initialize(HostNode *n) {
  gchar *sp = g_strdup_printf("%s/%s_%s_%d", cfg->tmuxsockpath, n->ssh_user, n->hostname, n->ssh_port);
  GFile *path = g_file_new_for_path(sp);
  g_free(sp);
  n->mon_ttymux = g_file_monitor(path, G_FILE_MONITOR_SEND_MOVED, NULL, NULL);
  g_object_unref(path);

  g_signal_connect(n->mon_ttymux, "changed", G_CALLBACK(tmux_changed), n);
}

gboolean
tmux_get_sessions(HostNode *n) {
  g_assert(n);

  if (n->screens) {
    g_list_free(n->screens);
    n->screens = NULL;
  }

  const gchar *sdir = tmux_get_sdir();
  if (!sdir)
    return FALSE;

  gchar *sock = g_strdup_printf("%s/%s_%s_%d", cfg->tmuxsockpath, n->ssh_user, n->hostname, n->ssh_port);
  gchar *argv[7] = {TMUX_BINARY, "-S", sock, "list-session", "-F", "#{session_name}\t#{session_created}\t#{session_attached}", NULL};
  gchar *out = NULL;
  gint rc;
  GError *error = NULL;
  gboolean r = g_spawn_sync(NULL,  /* working_directory */
			    argv,
			    NULL,  /* envp */
			    G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_SEARCH_PATH, /* GSpawnFlags */
			    NULL,  /* GSpawnChildSetupFunc */
			    NULL,  /* user_data */
			    &out,  /* &standard_output */
			    NULL,  /* standard_error */
			    &rc,   /* return code */
			    &error);
  g_free(sock);

  if(r == FALSE) {
    g_warning("failed to run tmux: %s", error->message);
    g_clear_error (&error);

    return FALSE;
  }

  if(!g_spawn_check_exit_status(rc, &error)) {
    /*    g_warning("error on list-sessions: %s", error->message);
	  g_clear_error (&error);*/

    return FALSE;
  }

  gchar **lines = g_strsplit(out, "\n", 0xff);
  gint i = -1;
  while(lines[++i] && strlen(lines[i])) {
    SessNode *s = g_new0(SessNode, 1);
#ifndef NDEBUG
    s->_type = T_SESSNODE;
#endif
    gchar **line = g_strsplit(lines[i], "\t", 4);
    gint j = -1;
    while(line[++j] && j < 3) {
      if(j == 0) {
	sscanf(line[j], "%d", &(s->pid));
	continue;
      }

      if(j == 1) {
	s->st.st_mtime = strtoull(line[j], NULL, 10);
	continue;
      }

      if(j == 2) {
	gint h;
	sscanf(line[j], "%d", &h);
	s->attached = (h != 0);
	continue;
      }
    }
    g_strfreev(line);

    n->screens = g_list_append(n->screens, s);
  }

  g_strfreev(lines);
  return g_list_length(n->screens) > 0;
}

gchar **
tmux_new(HostNode *n, const gboolean detached) {
  gchar **_argv = (gchar **) g_malloc0(sizeof(gchar *) * 8);
  gchar *title = parse_string("%m # %U%H", n);

  _argv[0] = g_strdup(TMUX_BINARY);
  _argv[1] = g_strdup("-S");
  _argv[2] = g_strdup_printf("%s/%s_%s_%d", cfg->tmuxsockpath, n->ssh_user, n->hostname, n->ssh_port);
  _argv[3] = g_strdup("-f");
  _argv[4] = g_strdup(cfg->tmuxconffile);
  _argv[5] = g_strdup("new-session");
  _argv[6] = g_strdup_printf("-%sn", detached ? "d" : "");
  _argv[7] = title;
  _argv[8] = NULL;

  return _argv;
}

static gchar **
tmux_attach_cmd(const HostNode *n, const SessNode *s, const gboolean shared) {
  gchar **_argv = (gchar **) g_malloc0(sizeof(gchar *) * 7);

  _argv[0] = g_strdup(TMUX_BINARY);
  _argv[1] = g_strdup("-S");
  _argv[2] = g_strdup_printf("%s/%s_%s_%d", cfg->tmuxsockpath, n->ssh_user, n->hostname, n->ssh_port);
  _argv[3] = g_strdup("attach-session");
  _argv[4] = g_strdup("-t");
  _argv[5] = g_strdup_printf("%d", s->pid);
  _argv[6] = NULL;

  return _argv;
}

gboolean
tmux_attach(HostNode *n, const SessNode *s, const gboolean shared) {
 gboolean r;
 GError *error = NULL;
 gchar **argv = tmux_attach_cmd(n, s, shared);

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
 if(n->parse_result && !tmux_get_sessions(n)) {
    n->parse_result = FALSE;
    return history_ts_failed(cfg, n);
 }
#endif

 return FALSE;
}

gchar *
tmux_get_dump(const SessNode *s) {
  return NULL;
}

#endif /* FEAT_TMUX */
