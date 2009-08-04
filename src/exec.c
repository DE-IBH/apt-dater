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

#include "apt-dater.h"
#include "screen.h"
#include "exec.h"
#include "stats.h"
#include "parsecmd.h"
#include "history.h"
#include "env.h"
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

gboolean
ssh_cmd_refresh(HostNode *n)
{
 gboolean r;
 GError *error = NULL;
 gchar *cmd = NULL;
 gchar *argv[2] = {PKGLIBDIR"/cmd", NULL};
 gchar *output = NULL;
 gchar *identity_file = NULL;
 GPid  child_pid;
 gint  standard_output;
 GIOChannel *iocstdout;

 g_assert(n);

 prepareStatsFile(n);

 gchar **env = env_build(n, "refresh", NULL, NULL);

 r = g_spawn_async_with_pipes(g_getenv ("HOME"), /* working_directory */
			      argv,
			      env,  /* envp */
			      G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_SEARCH_PATH, /* GSpawnFlags */
			      NULL,  /* GSpawnChildSetupFunc */
			      NULL,  /* user_data */
			      &child_pid,
			      NULL,  /* standard_input */
			      &standard_output, /* &standard_output */
			      NULL,  /* standard_error */
			      &error);

 g_strfreev(env);

 if(r == TRUE) {
  iocstdout = g_io_channel_unix_new (standard_output);

  g_io_channel_set_flags(iocstdout, G_IO_FLAG_NONBLOCK, &error);

  g_io_channel_set_buffer_size (iocstdout, 8192);

  g_io_add_watch_full (iocstdout, G_PRIORITY_DEFAULT,
		       G_IO_PRI | G_IO_HUP | G_IO_ERR | G_IO_NVAL | G_IO_IN, 
		       setStatsFileFromIOC, n, refreshStatsOfNode);
 }

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }

 g_free(output);
 g_free(cmd);

 return r;
}


gboolean
ssh_cmd_upgrade(HostNode *n, const gboolean detached)
{
 gboolean r;
 GError *error = NULL;
 gchar *cmd = NULL;
 gchar **argv = NULL;

 g_assert(n);

 if(n->forbid & HOST_FORBID_UPGRADE)
    return FALSE;

 HistoryEntry he;
 he.ts = time(NULL);
 he.maintainer = maintainer;
 he.action = "upgrade";
 he.data = NULL;

 gchar *screen = screen_new(n, detached);

 cmd = g_strdup_printf ("%s%s",
			screen, PKGLIBDIR"/cmd");
 g_free(screen);

#ifdef FEAT_HISTORY
 n->parse_result = cfg->history_errpattern && strlen(cfg->history_errpattern);;
#endif

 argv = g_strsplit(cmd, "+", 0);

 g_free(cmd);

 gchar **env = env_build(n, "upgrade", NULL, &he);

 r = g_spawn_sync(g_getenv ("HOME"), argv, env,
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error);

 g_strfreev(env);
 g_strfreev(argv);

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }

#ifdef FEAT_HISTORY
 if(!detached && n->parse_result && !screen_get_sessions(n)) {
    n->parse_result = FALSE;
    return history_ts_failed(n);
 }
#endif

 return FALSE;
}

gboolean
ssh_cmd_install(HostNode *n, gchar *package, const gboolean detached)
{
 gboolean r;
 GError *error = NULL;
 gchar *cmd = NULL;
 gchar **argv = NULL;

 g_assert(n);

 if(n->forbid & HOST_FORBID_INSTALL)
    return FALSE;

 HistoryEntry he;
 he.ts = time(NULL);
 he.maintainer = maintainer;
 he.action = "install";
 he.data = package;

 gchar *screen = screen_new(n, detached);

 cmd = g_strdup_printf ("%s%s",
			screen, PKGLIBDIR"/cmd");

 g_free(screen);

#ifdef FEAT_HISTORY
 n->parse_result = cfg->history_errpattern && strlen(cfg->history_errpattern);
#endif

 argv = g_strsplit(cmd, "+", 0);

 g_free(cmd);

 gchar **env = env_build(n, "install", package, &he);

 r = g_spawn_sync(g_getenv ("HOME"), argv, env,
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error);

 g_strfreev(env);
 g_strfreev(argv);

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }

#ifdef FEAT_HISTORY
 if(!detached && n->parse_result && !screen_get_sessions(n)) {
    n->parse_result = FALSE;
    return history_ts_failed(n);
 }
#endif

 return FALSE;
}

void ssh_connect(HostNode *n, const gboolean detached)
{
 gboolean r;
 GError *error = NULL;
 gchar *cmd = NULL;
 gchar **argv;

 HistoryEntry he;
 he.ts = time(NULL);
 he.maintainer = maintainer;
 he.action = "connect";
 he.data = NULL;

 gchar *screen = screen_new(n, detached);

 cmd = g_strdup_printf ("%s%s",
			screen, PKGLIBDIR"/cmd");

 g_free(screen);

 argv = g_strsplit(cmd, "+", 0);

 g_free(cmd);

 gchar **env = env_build(n, "connect", NULL, &he);

 r = g_spawn_sync(g_getenv("HOME"), argv, env,
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error);

 g_strfreev(env);
 g_strfreev(argv);

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }
}

void sftp_connect(HostNode *n)
{
 gboolean r;
 GError *error = NULL;
 gint argc = 0;
 gchar **argv = NULL;

 g_assert(n);

 HistoryEntry he;
 he.ts = time(NULL);
 he.maintainer = maintainer;
 he.action = "sftp";
 he.data = NULL;

 gchar *cmd = screen_new(n, FALSE);

 cmd = g_realloc(cmd, strlen(cmd) + strlen(cfg->sftp_cmd) + 1);
 strcat(cmd, cfg->sftp_cmd);

 if (parse_cmdline(cmd, &argc, &argv, n) < 0) {
   g_free(cmd);
   return;
 }
 g_free(cmd);

 cmd = g_strjoinv("+", argv);
 g_strfreev(argv);

 argv = g_strsplit(cmd, "+", 0);

 gchar **env = env_build(n, "connect", NULL, &he);

 r = g_spawn_sync(g_getenv ("HOME"), argv, env,
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error);

 g_strfreev(env);

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }

 g_strfreev(argv);
}
