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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <errno.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include "apt-dater.h"
#include "keyfiles.h"
#include "ui.h"
#include "stats.h"
#include "sighandler.h"
#include "lock.h"
#include "env.h"

#ifdef FEAT_XMLREPORT
#include "report.h"
#endif

#define VERSTEXT PACKAGE_STRING " - " __DATE__ " " __TIME__ "\n\n" \
  "Copyright Holder: IBH IT-Service GmbH [http://www.ibh.net/]\n\n" \
  "This program is free software; you can redistribute it and/or modify\n" \
  "it under the terms of the GNU General Public License as published by\n" \
  "the Free Software Foundation; either version 2 of the License, or\n" \
  "(at your option) any later version.\n\n" \
  "Send bug reports to " PACKAGE_BUGREPORT ".\n\n"


CfgFile *cfg = NULL;
GMainLoop *loop = NULL;
gboolean rebuilddl = FALSE;
time_t oldest_st_mtime;

int main(int argc, char **argv, char **envp)
{
 int opts;
 gchar *cfgfilename = NULL;
 gchar *cfgdirname = NULL;
 GList *hosts = NULL;
#ifdef FEAT_XMLREPORT
 gboolean report = FALSE;
 gboolean refresh = TRUE;
#endif

#ifdef HAVE_GETTEXT
 setlocale(LC_ALL, "");
 textdomain(PACKAGE);
#endif

 cfgdirname = g_strdup_printf("%s/%s", g_get_user_config_dir(), PACKAGE);
 if(!cfgdirname) g_error(_("Out of memory."));

 cfgfilename = g_strdup_printf("%s/%s", cfgdirname, CFGFILENAME);
 if(!cfgfilename) g_error(_("Out of memory."));

 g_set_prgname(PACKAGE);
 g_set_application_name(PACKAGE_STRING);

 if(chkForInitialConfig(cfgdirname, cfgfilename))
  g_warning(_("Failed to create initial configuration file %s."), cfgfilename);

 while ((opts = getopt(argc, argv, "c:vrn")) != EOF) {
  switch(opts) {
  case 'c':
   if(cfgfilename) free(cfgfilename);
   cfgfilename = (char *) strdup(optarg);
   break;
  case 'v':
    g_print(VERSTEXT);
    exit(0);
   break;
  case 'r':
#ifdef FEAT_XMLREPORT
    report = TRUE;
#else
    g_error(_("Sorry, apt-dater was compiled w/o report feature!"));
#endif
    break;
#ifdef FEAT_XMLREPORT
  case 'n':
    refresh = FALSE;
    break;
#endif
  default:
#ifdef FEAT_XMLREPORT
   g_printerr(_("Usage: %s [-(c config|v|[n]r)]\n"), g_get_prgname());
#else
   g_printerr(_("Usage: %s [-(c config|v)]\n"), g_get_prgname());
#endif
   exit(EXIT_FAILURE);
  }
 }
 if(!cfgfilename) g_error(_("Out of memory."));

 if(!(cfg = (CfgFile *) loadConfig(cfgfilename))) {
  g_error(_("Error on loading config file %s\n"), cfgfilename);
  exit(EXIT_FAILURE);
 }

 if(!(hosts = (GList *) loadHosts(cfg->hostsfile))) {
  g_printerr(_("Error on loading config file %s\n"), cfg->hostsfile);
  exit(EXIT_FAILURE);
 }

 if(cfg->ssh_agent
#ifdef FEAT_XMLREPORT
 && (!report || refresh)
#endif
 ) {

  /* Spawn ssh-agent if needed */
  if(getenv("SSH_AGENT_PID") == NULL) {
    gint i;
    gchar **agent_argv = g_new0(gchar *, argc+2);

    agent_argv[0] = "ssh-agent";

    for(i=0; i<argc; i++)
     agent_argv[i+1] = argv[i];

    execvp(agent_argv[0], agent_argv);
    g_warning("Could not spawn ssh-agent: %s", g_strerror(errno));
  }
  /* Add keys */
  else {
    gint i;
    gchar **add_argv = g_new0(gchar *, cfg->ssh_numadd+2);

    add_argv[0] = "ssh-add";

    for(i=0; i<cfg->ssh_numadd; i++)
     add_argv[i+1] = cfg->ssh_add[i];

    GError *error;
    if(g_spawn_sync(NULL, add_argv, NULL, 
         G_SPAWN_SEARCH_PATH | G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
         NULL, NULL, NULL, &error) == FALSE) {
      g_warning("%s", error->message);
      g_clear_error (&error);
    }

    g_free(add_argv);
  }
 }

 env_init(envp);

#ifdef FEAT_XMLREPORT
 if(!report) {
#endif
   /* Test if we are the owner of the TTY or die. */
   if(g_access("/proc/self/fd/0", R_OK|W_OK)) {
     g_error(_("Cannot open your terminal /proc/self/fd/0 - please check."));
     exit(EXIT_FAILURE);
   }

   getOldestMtime(hosts);

   doUI(hosts);
   setSigHandler();
#ifdef FEAT_XMLREPORT
 }
 else {
   if(refresh)
    initReport(hosts);
   else
    initReport(NULL);
 }
#endif

 loop = g_main_loop_new (NULL, FALSE);

#ifdef HAVE_GLIB_TIMEOUT_ADD_SECONDS
 g_timeout_add_seconds(1, (GSourceFunc) refreshStats, hosts);
#else
 g_timeout_add(1000, (GSourceFunc) refreshStats, hosts);
#endif
 
#ifdef FEAT_XMLREPORT
 if(report)
  g_idle_add ((GSourceFunc) ctrlReport, hosts);
 else
#endif
  g_idle_add ((GSourceFunc) ctrlUI, hosts);

 /* Startup the main loop */
 g_main_loop_run (loop);

 g_main_loop_unref (loop);
 cleanUI();
 cleanupLocks();

 freeConfig(cfg);
 g_free(cfgfilename);
 g_free(cfgdirname);

 exit(EXIT_SUCCESS);
}
