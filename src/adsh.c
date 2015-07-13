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

#ifndef SOURCE_DATE_UTC
#error SOURCE_DATE_UTC is undefined!
#endif

#define VERSTEXT PACKAGE_STRING " - " SOURCE_DATE_UTC "\n\n" \
  "Copyright Holder: IBH IT-Service GmbH [https://www.ibh.net/]\n\n" \
  "This program is free software; you can redistribute it and/or modify\n" \
  "it under the terms of the GNU General Public License as published by\n" \
  "the Free Software Foundation; either version 2 of the License, or\n" \
  "(at your option) any later version.\n\n" \
  "Send bug reports to " PACKAGE_BUGREPORT ".\n\n"


gint adsh_find(gconstpointer hdata, gconstpointer starget) {
  gchar **target = (gchar **)target;
  /*  gchar *target = tstring;
      return(1);*/
}

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
 gchar *adsh_target = NULL;

#ifdef HAVE_GETTEXT
 setlocale(LC_ALL, "");
 textdomain(PACKAGE);
#endif

#ifdef REQUIRE_GLIB_TYPE_INIT
 g_type_init();
#endif

 cfgdirname = g_strdup_printf("%s/%s", g_get_user_config_dir(), PACKAGE);
 if(!cfgdirname) g_error(_("Out of memory."));

 cfgfilename = g_strdup_printf("%s/apt-dater.xml", cfgdirname);
 if(!cfgfilename) g_error(_("Out of memory."));

 g_set_prgname(PACKAGE);
 g_set_application_name(PACKAGE_STRING);

 if(chkForInitialConfig(cfgdirname, cfgfilename))
  g_warning(_("Failed to create initial configuration file %s."), cfgfilename);

 while ((opts = getopt(argc, argv, "c:vs:")) != EOF) {
  switch(opts) {
  case 'c':
   if(cfgfilename) free(cfgfilename);
   cfgfilename = (char *) strdup(optarg);
   break;
  case 'v':
    g_print(VERSTEXT);
    exit(0);
   break;
  case 's':
    adsh_target = g_strdup(optarg);
    break;
  default:
   g_printerr(_("Usage: %s [-(c <conffile>|v)] -s <host>:<port>\n"), g_get_prgname());
   exit(EXIT_FAILURE);
  }
 }
 if(!cfgfilename) g_error(_("Out of memory."));

 cfg = initialConfig();

 if(!(loadConfig(cfgfilename, cfg))) {
  g_printerr(_("Error on loading config file %s\n"), cfgfilename);
  exit(EXIT_FAILURE);
 }

 if(!(hosts = (GList *) loadHosts(cfg->hostsfile))) {
  g_printerr(_("Error on loading config file %s\n"), cfg->hostsfile);
  exit(EXIT_FAILURE);
 }

 env_init(envp);

 fprintf(stderr, "adsh: %s\n", adsh_target);
 gchar *adsh_ssh[2] = { NULL, NULL };
 GList *adsh_host = g_list_find_custom(hosts, adsh_target, adsh_find);


 freeConfig(cfg);
 g_free(cfgfilename);
 g_free(cfgdirname);

 
 exit(1);

 exit(EXIT_SUCCESS);
}
