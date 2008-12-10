/* apt-dater - terminal-based remote package update manager
 *
 * $Id$
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
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
#include "keyfiles.h"
#include "stats.h"
#include "lock.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


static char apt_dater_conf[] = "# Config file of apt-dater in the form of the"
 " glib GKeyFile required\n\n[Paths]\n# Default: $XDG_CONFIG_HOME/apt-dater/h"
 "osts.conf\n#HostsFile=path-to/hosts.conf\n\n# Default: $XDG_DATA_HOME/apt-d"
 "ater\n#StatsDir=path-to/stats\n\n[SSH]\n# SSH binary\nCmd=/usr/bin/ssh\nOpt"
 "ionalCmdFlags=-t\n\n# SFTP binary\nSFTPCmd=/usr/bin/mc /#sh:%u@%h:C/\n\nDefau"
 "ltUser=apt-dater\nDefaultPort=22\n\n#[Screen]\n## Default: $XDG_CONFIG_HOME"
 "/apt-dater/screenrc\n#RCFile=path-to/screenrc\n#\n## Default: %m # %u@%h:%p"
 "\n#Title=%m # %u@%h:%p\n#\n\n# These values requires apt-dater-host to be i"
 "nstalled on the target host.\n# You could call apt/aptitude directly (see a"
 "pt-dater-host source),\n# but this is not recommended.\n[Commands]\nCmdRefr"
 "esh=apt-dater-host refresh\nCmdUpgrade=apt-dater-host upgrade\nCmdInstall=a"
 "pt-dater-host install %s\n\n[Appearance]\n# Colors      = (COMPONENT FG BG "
 "\';\')*\n# COMPONENT ::= \'default\' | \'menu\' | \'status\' | \'selector\'"
 " | \'hoststatus\' |\n#               \'query\' | \'input\'\n# FG        ::="
 " COLOR\n# BG        ::= COLOR\n# COLOR     ::= \'black\' | \'blue\' | \'cya"
 "n\' | \'green\' | \'magenta\' | \'red\' |\n#               \'white\' | \'ye"
 "llow\'\nColors=menu brightgreen blue;status brightgreen blue;selector black"
 " red;\n\n#[TCLFilter]\n#Predefined filter expression on startup.\n#FilterEx"
 "p=return [expr [string compare $lsb_distri \"Debian\"] == 0 && $lsb_rel < 4"
 ".0]\n#Load this TCL file on startup in the same context as the FilterExp.\n"
 "#FilterFile=/patch/to/file.tcl\n";

static char hosts_conf[] = "# Syntax:\n#\n#  [Customer Name]\n#  Hosts=([Opti"
 "onalUser@]host.domain[:OptionalPort];)*\n#\n\n[Localdomain]\nHosts=localhos"
 "t;\n\n[IBH]\nHosts=example1.ibh.net;example2.ibh.net;test@example3.ibh.net:"
 "62222;\n";

static char screenrc[] = "# -------------------------------------------------"
 "-----------------------------\n# SCREEN SETTINGS\n# -----------------------"
 "-------------------------------------------------------\n\nstartup_message "
 "off\n\n#defflow on # will force screen to process ^S/^Q\ndeflogin on\nautod"
 "etach on\n\n# turn visual bell on\nvbell on\n\n# define a bigger scrollback"
 ", default is 100 lines\ndefscrollback 2048\n\n# ---------------------------"
 "---------------------------------------------------\n# SCREEN KEYBINDINGS\n"
 "# -------------------------------------------------------------------------"
 "-----\n\n# Remove some stupid / dangerous key bindings\nbind ^k\n#bind L\nb"
 "ind ^\\\n# Make them better\nbind \\\\ quit\nbind K kill\nbind I login on\n"
 "bind O login off\nbind } history\n\n# Sessions should stay until destroyed "
 "by pressing q\nzombie \'q\'\n\n# --------------------------------------"
 "----------------------------------------\n# TERMINAL SETTINGS\n# ----------"
 "--------------------------------------------------------------------\n\n# T"
 "he vt100 description does not mention \"dl\". *sigh*\ntermcapinfo vt100 dl="
 "5\\E[M\n\n# Set the hardstatus prop on gui terms to set the titlebar/icon t"
 "itle\ntermcapinfo xterm*|rxvt*|kterm*|Eterm* hs:ts=\\E]0;:fs=\\007:ds=\\E]0"
 ";\\007:OP\n\n# set these terminals up to be \'optimal\' instead of vt100\n#"
 "termcapinfo xterm*|linux*|rxvt*|Eterm* OP\n\n# Change the xterm initializat"
 "ion string from is2=\\E[!p\\E[?3;4l\\E[4l\\E>\n# (This fixes the \"Aborted "
 "because of window size change\" konsole symptoms found\n#  in bug #134198)"
 "\ntermcapinfo xterm \'is=\\E[r\\E[m\\E[2J\\E[H\\E[?7h\\E[?1;4;6l\'\n\n# To "
 "get screen to add lines to xterm\'s scrollback buffer, uncomment the\n# fol"
 "lowing termcapinfo line which tells xterm to use the normal screen buffer\n"
 "# (which has scrollback), not the alternate screen buffer.\n#\ntermcapinfo "
 "xterm|xterms|xs|rxvt ti@:te@\n\n# Add caption line with clock, window title"
 " and window flags.\ncaption always \"%{b bG}%c%=%t%=%f\"\n\n# Catch zmodem "
 "file transfers\nzmodem catch\n";


int chkForInitialConfig(const gchar *cfgdir, const gchar *cfgfile)
{
 FILE *fp = NULL;
 gchar *pathtofile = NULL;

 if(g_file_test (cfgfile, G_FILE_TEST_IS_REGULAR|G_FILE_TEST_EXISTS) == FALSE) {
  if(g_file_test(cfgdir, G_FILE_TEST_IS_DIR) == FALSE) {
   if(g_mkdir_with_parents (cfgdir, 0700)) return(1);

   /* Create a example hosts.conf */
   pathtofile = g_strdup_printf("%s/hosts.conf", cfgdir);
   if(!pathtofile) g_error("Out of memory\n");
   fp = fopen(pathtofile, "w");
   g_message("Creating file %s", pathtofile);
   g_free(pathtofile);
   if(!fp) return(1);
   fwrite(hosts_conf, sizeof(hosts_conf)-1, 1, fp);
   fclose(fp);

   /* Create a the example screenrc */
   pathtofile = g_strdup_printf("%s/screenrc", cfgdir);
   if(!pathtofile) g_error("Out of memory\n");
   fp = fopen(pathtofile, "w");
   g_message("Creating file %s", pathtofile);
   g_free(pathtofile);
   if(!fp) return(1);
   fwrite(screenrc, sizeof(screenrc)-1, 1, fp);
   fclose(fp);
  }

  fp = fopen(cfgfile, "w");
  if(!fp) return(1);
  
  g_message("Creating file %s", cfgfile);
  fwrite(apt_dater_conf, sizeof(apt_dater_conf)-1, 1, fp);
  fclose(fp);
 }

 return(0);
}

static int cmpStringP(const void *p1, const void *p2)
{
 return(g_ascii_strcasecmp (*(gchar * const *) p1, *(gchar * const *) p2));
}

void freeConfig (CfgFile *cfg)
{
 g_free(cfg->hostsfile);
 g_free(cfg->statsdir);
 g_free(cfg->ssh_cmd);
 g_free(cfg->ssh_optflags);
 g_free(cfg->ssh_defuser);
 g_free(cfg->sftp_cmd);
 g_free(cfg->cmd_refresh);
 g_free(cfg->cmd_upgrade);
 g_free(cfg->cmd_install);
 g_free(cfg->screenrcfile);
 g_free(cfg->screentitle);
 g_strfreev(cfg->colors);

 g_free(cfg);
}

CfgFile *loadConfig (char *filename)
{
 GKeyFile *keyfile;
 GKeyFileFlags flags;
 GError *error = NULL;
 CfgFile *lcfg = NULL;

 keyfile = g_key_file_new ();
 flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

 if (!g_key_file_load_from_file (keyfile, filename, flags, &error)) {
  g_error ("%s: %s", filename, error->message);
  return (FALSE);
 }

 lcfg = g_new0(CfgFile, 1);

 lcfg->ssh_optflags = g_key_file_get_string(keyfile, "SSH", 
					    "OptionalCmdFlags", &error);
 g_clear_error(&error);

 if(!(lcfg->hostsfile = 
      g_key_file_get_string(keyfile, "Paths", "HostsFile", NULL)))
    lcfg->hostsfile = g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "hosts.conf");
 if(!(lcfg->statsdir =
      g_key_file_get_string(keyfile, "Paths", "StatsDir", NULL)))
    lcfg->statsdir = g_strdup_printf("%s/%s/%s", g_get_user_cache_dir(), PROG_NAME, "stats");
 g_mkdir_with_parents(lcfg->statsdir, S_IRWXU | S_IRWXG | S_IRWXO);

 if(!(lcfg->screenrcfile = 
      g_key_file_get_string(keyfile, "Screen", "RCFile", NULL)))
    lcfg->screenrcfile = g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "screenrc");
 if(!(lcfg->screentitle = 
      g_key_file_get_string(keyfile, "Screen", "Title", NULL)))
    lcfg->screentitle = g_strdup("%m # %u@%h:%p");

 if(!(lcfg->ssh_cmd = 
      g_key_file_get_string(keyfile, "SSH", "Cmd", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (NULL);
 }

 if(!(lcfg->ssh_defuser = 
      g_key_file_get_string(keyfile, "SSH", "DefaultUser", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (NULL);
 }

 if(!(lcfg->ssh_defport = 
      g_key_file_get_integer(keyfile, "SSH", "DefaultPort", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (NULL);
 }

 if(!(lcfg->sftp_cmd = 
      g_key_file_get_string(keyfile, "SSH", "SFTPCmd", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (NULL);
 }

 if(!(lcfg->cmd_refresh = 
      g_key_file_get_string(keyfile, "Commands", "CmdRefresh", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (NULL);
 }

 if(!(lcfg->cmd_upgrade = 
      g_key_file_get_string(keyfile, "Commands", "CmdUpgrade", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (NULL);
 }

 if(!(lcfg->cmd_install = 
      g_key_file_get_string(keyfile, "Commands", "CmdInstall", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (NULL);
 }

 lcfg->use_screen = g_key_file_get_integer(keyfile, "Screen", "Enabled", &error);
 if (error) {
   lcfg->use_screen = g_file_test(SCREEN_BINARY, G_FILE_TEST_IS_EXECUTABLE);
   g_clear_error(&error);
 }

 lcfg->dump_screen = !g_key_file_get_boolean(keyfile, "Screen", "NoDumps", &error);
 if (error) {
   lcfg->dump_screen = TRUE;
   g_clear_error(&error);
 }

 lcfg->query_maintainer = g_key_file_get_integer(keyfile, "Screen", "QueryMaintainer", &error);
 if (error) {
   lcfg->query_maintainer = 2;
   g_clear_error(&error);
 }

 if(!(lcfg->colors = 
     g_key_file_get_string_list(keyfile, "Appearance", "Colors", 
				NULL, &error))) {

  g_message ("%s: %s", filename, error->message);
 }

#ifdef FEAT_TCLFILTER
 lcfg->filterexp = g_key_file_get_string(keyfile, "TCLFilter", "FilterExp", NULL);
 lcfg->filterfile = g_key_file_get_string(keyfile, "TCLFilter", "FilterFile", NULL);
#endif

#ifdef FEAT_COOPREF
 lcfg->auto_after = g_key_file_get_integer(keyfile, "COOPRef", "auto_after", &error);
 if (error) {
   lcfg->auto_after = 60;
   g_clear_error(&error);
 }
#endif

 g_clear_error(&error);
 g_key_file_free(keyfile);

 return (lcfg);
}


GList *loadHosts (char *filename)
{
 GKeyFile *keyfile;
 GKeyFileFlags flags;
 GError *error = NULL;
 GList *hosts = NULL;
 gchar **groups = NULL;
 gchar **khosts = NULL;
 char  ssh_user[BUF_MAX_LEN];
 char  hostname[BUF_MAX_LEN];
 int   ssh_port = 0;
 gsize lengrp, lenkey;
 HostNode *hostnode;
 gint  i, j;

 keyfile = g_key_file_new ();
 flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

 if (!g_key_file_load_from_file (keyfile, filename, flags, &error)) {
  g_error ("%s: %s", filename, error->message);
  return (FALSE);
 }
 
 groups = g_key_file_get_groups (keyfile, &lengrp);
 qsort(groups, lengrp, sizeof(gchar *), cmpStringP);

 for(i = 0; i < lengrp; i++) {
  if(!(khosts = 
       g_key_file_get_string_list(keyfile, groups[i], "Hosts", 
				  &lenkey, &error))) {
   g_error ("%s: %s", filename, error->message);
   return(NULL);
  }

  qsort(khosts, lenkey, sizeof(gchar *), cmpStringP);

  for(j = 0; j < lenkey; j++) {
   hostnode = g_new0(HostNode, 1);

   *hostname = *ssh_user = 0; ssh_port = 0;

   if(sscanf(khosts[j], "%[a-zA-Z0-9_-.]@%[a-zA-Z0-9-.]:%d", ssh_user, hostname, &ssh_port) != 3) {
    *hostname = *ssh_user = 0; ssh_port = 0;
    if(sscanf(khosts[j], "%[a-zA-Z0-9-.]:%d", hostname, &ssh_port) != 2) {
     *hostname = *ssh_user = 0; ssh_port = 0;
     if(sscanf(khosts[j], "%[a-zA-Z0-9_-.]@%[a-zA-Z0-9-.]", ssh_user, hostname) != 2) {
      *hostname = *ssh_user = 0; ssh_port = 0;
      sscanf(khosts[j], "%s", hostname);
     }
    }
   }

   hostnode->hostname = g_strdup(hostname);
   hostnode->ssh_user = *ssh_user ? g_strdup(ssh_user) : g_strdup(cfg->ssh_defuser);
   hostnode->ssh_port = ssh_port ? ssh_port : cfg->ssh_defport;

   hostnode->fdlock = -1;
   hostnode->identity_file = g_key_file_get_string(keyfile, groups[i], 
						  "IdentityFile", &error);
   g_clear_error(&error);

   hostnode->group = g_strdup(groups[i]);
   getUpdatesFromStat(hostnode);

   hosts = g_list_append(hosts, hostnode);

   g_free(khosts[j]);
  }

  g_free(khosts);
  g_free(groups[i]);
 }

 g_free(groups);

 g_clear_error(&error);
 g_key_file_free(keyfile);

 return (hosts);
}
