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

#include <glib.h>
#include <glib/gstdio.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "apt-dater.h"
#include "keyfiles.h"
#include "ui.h"
#include "stats.h"
#include "sighandler.h"
#include "lock.h"

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

int main(int argc, char **argv)
{
 char opts;
 char *cfgfilename = NULL;
 char *cfgdirname = NULL;
 GList *hosts = NULL;
#ifdef FEAT_XMLREPORT
 gboolean report = FALSE;
#endif

 cfgdirname = g_strdup_printf("%s/%s", g_get_user_config_dir(), PACKAGE);
 if(!cfgdirname) g_error("Out of memory\n");

 cfgfilename = g_strdup_printf("%s/%s", cfgdirname, CFGFILENAME);
 if(!cfgfilename) g_error("Out of memory\n");

 g_set_prgname(PACKAGE);
 g_set_application_name(PACKAGE_STRING);

 if(chkForInitialConfig(cfgdirname, cfgfilename))
  g_warning("Failed to create initial configuration file %s.", cfgfilename);

 while ((opts = getopt(argc, argv, "c:vr")) != EOF) {
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
    g_error("Sorry, "PACKAGE" was compiled w/o report feature!\n");
#endif
    break;
  default:
   g_printerr("Usage: %s [-(c config|v|r)]\n", g_get_prgname());
   exit(EXIT_FAILURE);
  }
 }
 if(!cfgfilename) g_error("Out of memory\n");

 if(!(cfg = (CfgFile *) loadConfig(cfgfilename))) {
  g_printerr("Error on loading config file %s\n", cfgfilename);
  exit(EXIT_FAILURE);
 }

 if(!(hosts = (GList *) loadHosts(cfg->hostsfile))) {
  g_printerr("Error on loading config file %s\n", cfg->hostsfile);
  exit(EXIT_FAILURE);
 }


#ifdef FEAT_XMLREPORT
 if(!report) {
#endif
   /* Test if we are the owner of the TTY or die. */
   if(g_access("/proc/self/fd/0", R_OK|W_OK)) {
     g_error("Cannot open your terminal /proc/self/fd/0 - please check.");
     exit(EXIT_FAILURE);
   }

   getOldestMtime(hosts);

   doUI(hosts);
   setSigHandler();
#ifdef FEAT_XMLREPORT
 }
 else
   initReport(hosts);
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
 free(cfgfilename);
 free(cfgdirname);

 exit(EXIT_SUCCESS);
}
