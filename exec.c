/* $Id$
 *
 */

#include "apt-dater.h"
#include "screen.h"
#include "exec.h"

gboolean ssh_cmd_refresh(HostNode *n)
{
 gboolean r;
 GError *error = NULL;
 gchar *cmd = NULL;
 gchar **argv = NULL;
 gchar *output = NULL;
 GPid  child_pid;
 gint  standard_output;
 GIOChannel *iocstdout;

 if(!n) return (FALSE);

 cmd = g_strdup_printf("%s+-n+-o+BatchMode=yes+-o+ConnectTimeout=5+-q+-l+%s+-p+%d+%s+unset LANG && %s",
		       cfg->ssh_cmd, n->ssh_user, n->ssh_port, n->hostname,
		       cfg->cmd_refresh);
 if(!cmd) return(FALSE);

 argv = g_strsplit(cmd, "+", 0);

 removeStatsFile(n->hostname);

 r = g_spawn_async_with_pipes(g_getenv ("HOME"), /* working_directory */
			      argv,
			      NULL,  /* envp */
			      G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_SEARCH_PATH, /* GSpawnFlags */
			      NULL,  /* GSpawnChildSetupFunc */
			      NULL,  /* user_data */
			      &child_pid,
			      NULL,  /* standard_input */
			      &standard_output, /* &standard_output */
			      NULL,  /* standard_error */
			      &error);

 if(r == TRUE) {
  iocstdout = g_io_channel_unix_new (standard_output);

  g_io_channel_set_buffer_size (iocstdout, 8192);

  g_io_add_watch_full (iocstdout, G_PRIORITY_DEFAULT,
		       G_IO_PRI | G_IO_HUP | G_IO_ERR | G_IO_NVAL, 
		       setStatsFileFromIOC, n, refreshStatsOfNode);
 }

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 } 

 g_free(output);
 g_free(cmd);
 g_strfreev(argv);
 
 return (r);
}


gboolean ssh_cmd_upgrade(HostNode *n, const gboolean detached)
{
 gboolean r;
 GError *error = NULL;
 gchar *cmd = NULL;
 gchar **argv = NULL;

 gchar *screen = screen_new(n, detached);

 if(strlen(cfg->ssh_optflags) > 0) {
  cmd = g_strdup_printf ("%s%s+-l+%s+-p+%d+%s+%s+unset LANG ; export MAINTAINER='%s' ; %s", 
			 screen,
			 cfg->ssh_cmd, n->ssh_user, n->ssh_port, 
			 cfg->ssh_optflags, n->hostname, 
			 maintainer, cfg->cmd_upgrade);
 } else {
  cmd = g_strdup_printf ("%s%s+-l+%s+-p+%d+%s+unset LANG ; export MAINTAINER='%s' ; %s", 
			 screen,
			 cfg->ssh_cmd, n->ssh_user, n->ssh_port, n->hostname,
			 maintainer, cfg->cmd_upgrade);
 }
 g_free(screen);

 if(!cmd) return(FALSE);

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

 return (r);
}


gboolean ssh_cmd_install(HostNode *n, const gchar *package, const gboolean detached)
{
 gboolean r;
 GError *error = NULL;
 gchar *cmd = NULL;
 gchar *buf = NULL;
 gchar **argv = NULL;

 gchar *screen = screen_new(n, detached);

 if(strlen(cfg->ssh_optflags) > 0) {
  cmd = g_strdup_printf ("%s%s+-l+%s+-p+%d+%s+%s+unset LANG ; export MAINTAINER='%s' ; %s", 
			 screen,
			 cfg->ssh_cmd, n->ssh_user, n->ssh_port, 
			 cfg->ssh_optflags, n->hostname,
			 maintainer, cfg->cmd_install);
 } else {
  cmd = g_strdup_printf ("%s%s+-l+%s+-p+%d+%s+unset LANG ; export MAINTAINER='%s' ; %s", 
			 screen,
			 cfg->ssh_cmd, n->ssh_user, n->ssh_port, n->hostname,
			 maintainer, cfg->cmd_install);
 }
 g_free(screen);

 buf = g_strdup_printf (cmd, package);
 if(!buf) return(FALSE);
 
 g_free(cmd);
 cmd = buf;
 
 if(!cmd) return(FALSE);

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
 
 return (r);
}

gboolean ssh_connect(HostNode *n, const gboolean detached)
{
 gboolean r;
 GError *error = NULL;
 gchar *cmd = NULL;
 gchar **argv = NULL;

 gchar *screen = screen_new(n, detached);

 if(strlen(cfg->ssh_optflags) > 0) {
  cmd = g_strdup_printf ("%s%s+-l+%s+-t+-p+%d+%s+%s+export MAINTAINER='%s' ; $SHELL",
			 screen,
			 cfg->ssh_cmd, n->ssh_user, n->ssh_port, 
			 cfg->ssh_optflags, n->hostname,
			 maintainer);
 } else {
  cmd = g_strdup_printf ("%s%s+-l+%s+-t+-p+%d+%s+export MAINTAINER='%s' ; $SHELL", 
			 screen,
			 cfg->ssh_cmd, n->ssh_user, n->ssh_port, n->hostname,
			 maintainer);
 }
 g_free(screen);

 if(!cmd) return(FALSE);

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
 
 return (r);
}
