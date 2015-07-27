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
#include "ttymux.h"

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

typedef struct _hostfind {
  gchar     *ssh_host;
  gint      ssh_port;
} HostFind;


gint adsh_find(gconstpointer phost, gconstpointer ptarget) {
  HostNode *host = (HostNode *)phost;
  HostFind *target = (HostFind *)ptarget;

  if(strcasecmp((host->ssh_host ? host->ssh_host : host->hostname), target->ssh_host) == 0 && host->ssh_port == target->ssh_port) {
    return 0;
  }

  return 1;
}

int main(int argc, char **argv, char **envp)
{
 int opts;
 gchar *cfgfilename = NULL;
 gchar *cfgdirname = NULL;
 GList *hosts = NULL;

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

 HostFind adsh_target;
 adsh_target.ssh_port = 0;
 opterr = 0;
 while ((opts = getopt(argc, argv, "1246ab:c:e:fgi:kl:m:no:p:qstvxACD:E:F:I:KL:MNO:PQ:R:S:TVw:W:XYy")) != EOF) {
   switch (opts) {
   case 'p':
     adsh_target.ssh_port = atoi(optarg);
     break;
   }
 }

 int ac = argc - optind;
 char **av = argv + optind;
 
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

 if(ac > 0) {
   adsh_target.ssh_host = av[0];
   if(strrchr(adsh_target.ssh_host, '@')) {
     adsh_target.ssh_host = strrchr(adsh_target.ssh_host, '@');
   }
   GList *adsh_host = g_list_find_custom(hosts, &adsh_target, adsh_find);

   if(adsh_host) {
     HostNode *n = adsh_host->data;
     
     HistoryEntry he;
     he.ts = time(NULL);
     he.maintainer = maintainer;
     he.action = "adsh";
     he.data = NULL;

     gchar **env = env_build(n, "adsh", NULL, &he);

     execve(PKGLIBDIR"/cmd", argv, env);
     perror("Failed to run "PKGLIBDIR"/cmd");
     exit(EXIT_FAILURE);
   }
 }

 execve("/usr/bin/ssh", argv, envp);
 exit(EXIT_FAILURE);
}
