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

#include "apt-dater.h"
#include "screen.h"
#include "keyfiles.h"
#include "stats.h"
#include "lock.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <libconfig.h>

static char apt_dater_conf[] = "# Config file of apt-dater in the form of the"
 " glib GKeyFile required\n\n[Paths]\n# Default: $XDG_CONFIG_HOME/apt-dater/h"
 "osts.conf\n#HostsFile=path-to/hosts.conf\n\n# Default: $XDG_DATA_HOME/apt-d"
 "ater\n#StatsDir=path-to/stats\n\n[SSH]\n# SSH binary\nCmd=/usr/bin/ssh\nOpt"
 "ionalCmdFlags=-t\n\n# SFTP binary\nSFTPCmd=/usr/bin/sftp\n\n"
 "#[Screen]\n## Default: $XDG_CONFIG_HOME"
 "/apt-dater/screenrc\n#RCFile=path-to/screenrc\n#\n## Default: %m # %u@%h:%p"
 "\n#Title=%m # %U%H\n#\n\n# These values requires apt-dater-host to be i"
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
 "p=return [expr [string compare $lsb_distri \"Debian\"] == 0 && $lsb_rel < 5"
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
   if(!pathtofile) g_error(_("Out of memory."));
   fp = fopen(pathtofile, "wx");
   g_message(_("Creating file %s"), pathtofile);
   if(fp) {
    if(fwrite(hosts_conf, sizeof(hosts_conf)-1, 1, fp) != 1) g_error(_("Could not write to file %s."), pathtofile);
    fclose(fp);
   }
   g_free(pathtofile);

   /* Create a the example screenrc */
   pathtofile = g_strdup_printf("%s/screenrc", cfgdir);
   if(!pathtofile) g_error(_("Out of memory."));
   fp = fopen(pathtofile, "wx");
   g_message(_("Creating file %s"), pathtofile);
   if(fp) {
    if(fwrite(screenrc, sizeof(screenrc)-1, 1, fp) != 1) g_error(_("Could not write to file %s."), pathtofile);
    fclose(fp);
   }
   g_free(pathtofile);

   /* Create initial config file */
   fp = fopen(cfgfile, "wx");
   if(!fp) return(1);

   g_message(_("Creating file %s"), cfgfile);
   if(fwrite(apt_dater_conf, sizeof(apt_dater_conf)-1, 1, fp) != 1) g_error(_("Could not write to file %s."), cfgfile);
   fclose(fp);
  }
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
 g_free(cfg->sftp_cmd);
 g_free(cfg->cmd_refresh);
 g_free(cfg->cmd_upgrade);
 g_free(cfg->cmd_install);
 g_free(cfg->screenrcfile);
 g_free(cfg->screentitle);
 g_strfreev(cfg->colors);

 g_free(cfg);
}

CfgFile *initialConfig() {
    CfgFile *lcfg = g_new0(CfgFile, 1);
#ifndef NDEBUG
    lcfg->_type = T_CFGFILE;
#endif

    lcfg->hostsfile = g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "hosts.conf");
    lcfg->statsdir = g_strdup_printf("%s/%s/%s", g_get_user_cache_dir(), PROG_NAME, "stats");

    lcfg->screenrcfile = g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "screenrc");
    lcfg->screentitle = "%m # %U%H";

    lcfg->dump_screen = TRUE;
    lcfg->query_maintainer = FALSE;

#ifdef FEAT_AUTOREF
    lcfg->auto_refresh = TRUE;
#endif

    lcfg->beep = TRUE;
    lcfg->flash = TRUE;

#ifdef FEAT_HISTORY
    lcfg->record_history = TRUE;
    lcfg->history_errpattern = "(error|warning|fail)";
#endif

    lcfg->hook_pre_upgrade = "/etc/apt-dater/pre-upg.d";
    lcfg->hook_pre_refresh = "/etc/apt-dater/pre-ref.d";
    lcfg->hook_pre_install = "/etc/apt-dater/pre-ins.d";
    lcfg->hook_pre_connect = "/etc/apt-dater/pre-con.d";

    lcfg->hook_post_upgrade = "/etc/apt-dater/post-upg.d";
    lcfg->hook_post_refresh = "/etc/apt-dater/post-ref.d";
    lcfg->hook_post_install = "/etc/apt-dater/post-ins.d";
    lcfg->hook_post_connect = "/etc/apt-dater/post-con.d";

    lcfg->plugindir = "/etc/apt-dater/plugins";

    return lcfg;
}

#define CFG_GET_BOOL_DEFAULT(setting,key,var,def) \
    var = (def); \
    config_setting_lookup_bool((setting), (key), &(var));
#define CFG_GET_STRING_DEFAULT(setting,key,var,def) \
    var = (def); \
    config_setting_lookup_string((setting), (key), (const char **) &(var)); \
    if(var != NULL) var = g_strdup(var)

gboolean loadConfig(char *filename, CfgFile *lcfg) {

    config_t hcfg;

    config_init(&hcfg);
    if(config_read_file(&hcfg, filename) == CONFIG_FALSE) {
	g_error ("%s:%d %s", config_error_file(&hcfg), config_error_line(&hcfg), config_error_text(&hcfg));
	config_destroy(&hcfg);
	return (FALSE);
    }

    config_setting_t *s_ssh = config_lookup(&hcfg, "SSH");
    config_setting_t *s_paths = config_lookup(&hcfg, "Paths");
    config_setting_t *s_screen = config_lookup(&hcfg, "Screen");
    config_setting_t *s_notify = config_lookup(&hcfg, "Notify");
    config_setting_t *s_hooks = config_lookup(&hcfg, "Hooks");
#ifdef FEAT_AUTOREF
    config_setting_t *s_autoref = config_lookup(&hcfg, "AutoRef");
#endif
#ifdef FEAT_HISTORY
    config_setting_t *s_history = config_lookup(&hcfg, "History");
#endif
#ifdef FEAT_TCLFILTER
    config_setting_t *s_tclfilter = config_lookup(&hcfg, "TCLFilter");
#endif

    config_setting_lookup_string(s_ssh, "OptionalCmdFlags", (const char **) &(lcfg->ssh_optflags));

    CFG_GET_STRING_DEFAULT(s_paths, "HostsFile", lcfg->hostsfile, g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "hosts.conf"));
    CFG_GET_STRING_DEFAULT(s_paths, "StatsDir", lcfg->statsdir, g_strdup_printf("%s/%s/%s", g_get_user_cache_dir(), PROG_NAME, "stats"));
    g_mkdir_with_parents(lcfg->statsdir, S_IRWXU | S_IRWXG | S_IRWXO);

    CFG_GET_STRING_DEFAULT(s_screen, "RCFile", lcfg->screenrcfile, g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "screenrc"));
    CFG_GET_STRING_DEFAULT(s_screen, "Title", lcfg->screentitle, "%m # %U%H");

    if(config_setting_lookup_string(s_ssh, "Cmd", (const char **) &(lcfg->ssh_cmd)) == CONFIG_FALSE) {
	g_error ("%s: Cmd undefined", filename);
	return (FALSE);
    }

    if(config_setting_lookup_string(s_ssh, "SFTPCmd", (const char **) &(lcfg->sftp_cmd)) == CONFIG_FALSE) {
	g_error ("%s: SFTPCmd undefined", filename);
	return (FALSE);
    }

    config_setting_lookup_bool(s_ssh, "SpawnAgent", &(lcfg->ssh_agent));

    config_setting_t *s_addkeys = config_setting_get_member(s_ssh, "AddKeys");
    if(s_addkeys != NULL) {
	if(config_setting_type(s_addkeys) == CONFIG_TYPE_STRING) {
	    lcfg->ssh_add = g_new0(char*, 2);
	    lcfg->ssh_add[0] = g_strdup(config_setting_get_string(s_addkeys));
	}
	else if(config_setting_type(s_addkeys) == CONFIG_TYPE_ARRAY) {
	    int len = config_setting_length(s_addkeys);
	    int i;

	    lcfg->ssh_add = g_new0(char*, len + 1);
	    for(i = 0; i<len; i++) {
		config_setting_t *e = config_setting_get_elem(s_addkeys, i++);
		lcfg->ssh_add[i] = g_strdup(config_setting_get_string(e));
	    }
	}
	else {
	    g_error ("%s: setting %s must be a single string or an array of strings", filename, config_setting_name(s_addkeys));
	}
    }

    CFG_GET_BOOL_DEFAULT(s_screen, "NoDumps", lcfg->dump_screen, FALSE);
    lcfg->dump_screen = !lcfg->dump_screen;

    CFG_GET_BOOL_DEFAULT(s_screen, "QueryMaintainer", lcfg->query_maintainer, FALSE);

//TODO if(!(lcfg->colors = 
//     g_key_file_get_string_list(keyfile, "Appearance", "Colors", 
//				NULL, &error))) {
//
//  g_message ("%s: %s", filename, error->message);
// }

#ifdef FEAT_TCLFILTER
    config_setting_lookup_string(&s_tclfilter, "FilterExp", &(lcfg->lcfg->filterexp));
    config_setting_lookup_string(&s_tclfilter, "FilterFile", &(lcfg->lcfg->filterfile));
#endif

#ifdef FEAT_AUTOREF
    CFG_GET_BOOL_DEFAULT(s_autoref, "enabled", lcfg->auto_refresh, TRUE);
#endif

    CFG_GET_BOOL_DEFAULT(s_notify, "beep", lcfg->beep, TRUE);
    CFG_GET_BOOL_DEFAULT(s_notify, "flash", lcfg->flash, TRUE);

#ifdef FEAT_HISTORY
    CFG_GET_BOOL_DEFAULT(s_history, "record", lcfg->record_history, TRUE);
    CFG_GET_STRING_DEFAULT(s_history, "ErrPattern", lcfg->history_errpattern, "(error|warning|fail)");
#endif

    CFG_GET_STRING_DEFAULT(s_hooks, "PreUpgrade", lcfg->hook_pre_upgrade, "/etc/apt-dater/pre-upg.d");
    CFG_GET_STRING_DEFAULT(s_hooks, "PreRefresh", lcfg->hook_pre_refresh, "/etc/apt-dater/pre-ref.d");
    CFG_GET_STRING_DEFAULT(s_hooks, "PreInstall", lcfg->hook_pre_install, "/etc/apt-dater/pre-ins.d");
    CFG_GET_STRING_DEFAULT(s_hooks, "PreConnect", lcfg->hook_pre_connect, "/etc/apt-dater/pre-con.d");

    CFG_GET_STRING_DEFAULT(s_hooks, "PostUpgrade", lcfg->hook_post_upgrade, "/etc/apt-dater/post-upg.d");
    CFG_GET_STRING_DEFAULT(s_hooks, "PostRefresh", lcfg->hook_post_refresh, "/etc/apt-dater/post-ref.d");
    CFG_GET_STRING_DEFAULT(s_hooks, "PostInstall", lcfg->hook_post_install, "/etc/apt-dater/post-ins.d");
    CFG_GET_STRING_DEFAULT(s_hooks, "PostConnect", lcfg->hook_post_connect, "/etc/apt-dater/post-con.d");

    CFG_GET_STRING_DEFAULT(s_hooks, "PluginDir", lcfg->plugindir, "/etc/apt-dater/plugins");


    config_destroy(&hcfg);
    return (TRUE);
}

gboolean loadConfigLegacy (char *filename, CfgFile *lcfg)
{
 GKeyFile *keyfile;
 GKeyFileFlags flags;
 GError *error = NULL;

 keyfile = g_key_file_new ();
 flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

 if (!g_key_file_load_from_file (keyfile, filename, flags, &error)) {
  g_error ("%s: %s", filename, error->message);
  g_key_file_free(keyfile);
  return (FALSE);
 }

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
    lcfg->screentitle = g_strdup("%m # %U%H");

 if(!(lcfg->ssh_cmd = 
      g_key_file_get_string(keyfile, "SSH", "Cmd", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (FALSE);
 }

 if(!(lcfg->sftp_cmd = 
      g_key_file_get_string(keyfile, "SSH", "SFTPCmd", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (FALSE);
 }

 lcfg->ssh_agent = g_key_file_get_boolean(keyfile, "SSH", "SpawnAgent", NULL);
 lcfg->ssh_add = g_key_file_get_string_list(keyfile, "SSH", "AddKeys", &lcfg->ssh_numadd, NULL);

 if(!(lcfg->cmd_refresh = 
      g_key_file_get_string(keyfile, "Commands", "CmdRefresh", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (FALSE);
 }

 if(!(lcfg->cmd_upgrade = 
      g_key_file_get_string(keyfile, "Commands", "CmdUpgrade", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (FALSE);
 }

 if(!(lcfg->cmd_install = 
      g_key_file_get_string(keyfile, "Commands", "CmdInstall", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (FALSE);
 }

 lcfg->dump_screen = !g_key_file_get_boolean(keyfile, "Screen", "NoDumps", &error);
 if (error) {
   lcfg->dump_screen = TRUE;
   g_clear_error(&error);
 }

 lcfg->query_maintainer = g_key_file_get_integer(keyfile, "Screen", "QueryMaintainer", &error);
 if (error) {
   lcfg->query_maintainer = 0;
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

#ifdef FEAT_AUTOREF
 lcfg->auto_refresh = g_key_file_get_boolean(keyfile, "AutoRef", "enabled", &error);
 if (error) {
   lcfg->auto_refresh = TRUE;
   g_clear_error(&error);
 }
#endif

 lcfg->beep = g_key_file_get_boolean(keyfile, "Notify", "beep", &error);
 if (error) {
   lcfg->beep = TRUE;
   g_clear_error(&error);
 }
 lcfg->flash = g_key_file_get_boolean(keyfile, "Notify", "flash", &error);
 if (error) {
   lcfg->flash = TRUE;
   g_clear_error(&error);
 }

#ifdef FEAT_HISTORY
 lcfg->record_history = g_key_file_get_boolean(keyfile, "History", "record", &error);
 if (error) {
   lcfg->record_history = TRUE;
   g_clear_error(&error);
 }

 lcfg->history_errpattern = g_key_file_get_string(keyfile, "History", "ErrPattern", NULL);
 if(!lcfg->history_errpattern)
  lcfg->history_errpattern = "(error|warning|fail)";
#endif

#define KEY_FILE_GET_STRING_DEFAULT(var, sec, val, default) \
 (var) = g_key_file_get_string(keyfile, sec, val, NULL); \
 if(!(var)) \
  (var) = (default);

 KEY_FILE_GET_STRING_DEFAULT(lcfg->hook_pre_upgrade, "Hooks", "PreUpgrade" , "/etc/apt-dater/pre-upg.d");
 KEY_FILE_GET_STRING_DEFAULT(lcfg->hook_pre_refresh, "Hooks", "PreRefresh", "/etc/apt-dater/pre-ref.d");
 KEY_FILE_GET_STRING_DEFAULT(lcfg->hook_pre_install, "Hooks", "PreInstall", "/etc/apt-dater/pre-ins.d");
 KEY_FILE_GET_STRING_DEFAULT(lcfg->hook_pre_connect, "Hooks", "PreConnect", "/etc/apt-dater/pre-con.d");

 KEY_FILE_GET_STRING_DEFAULT(lcfg->hook_post_upgrade, "Hooks", "PostUpgrade" , "/etc/apt-dater/post-upg.d");
 KEY_FILE_GET_STRING_DEFAULT(lcfg->hook_post_refresh, "Hooks", "PostRefresh", "/etc/apt-dater/post-ref.d");
 KEY_FILE_GET_STRING_DEFAULT(lcfg->hook_post_install, "Hooks", "PostInstall", "/etc/apt-dater/post-ins.d");
 KEY_FILE_GET_STRING_DEFAULT(lcfg->hook_post_connect, "Hooks", "PostConnect", "/etc/apt-dater/post-con.d");

 KEY_FILE_GET_STRING_DEFAULT(lcfg->plugindir, "Hooks", "PluginDir", "/etc/apt-dater/plugins");

 g_clear_error(&error);
 g_key_file_free(keyfile);

 return (FALSE);
}

#define HCFG_GET_STRING(setting,var,def) \
    var = (def); \
    if(config_setting_lookup_string( cfghost, (setting), (const char **) &var) == CONFIG_FALSE) \
    if(config_setting_lookup_string(cfggroup, (setting), (const char **) &var) == CONFIG_FALSE) \
    config_setting_lookup_string(cfghosts, (setting), (const char **) &var); \
    if(var != NULL) var = g_strdup(var)

#define HCFG_GET_INT(setting,var,def) \
    var = (def); \
    if(config_setting_lookup_int( cfghost, (setting), &var) == CONFIG_FALSE) \
    if(config_setting_lookup_int(cfggroup, (setting), &var) == CONFIG_FALSE) \
    config_setting_lookup_int(cfghosts, (setting), &var)

GList *loadHostsNew (const char *filename) {
    config_t hcfg;

    config_init(&hcfg);
    if(config_read_file(&hcfg, filename) == CONFIG_FALSE) {
	g_error ("%s:%d %s", config_error_file(&hcfg), config_error_line(&hcfg), config_error_text(&hcfg));
	config_destroy(&hcfg);
	return (FALSE);
    }

    config_setting_t *cfghosts = config_lookup(&hcfg, "hosts");
    if(cfghosts == NULL) {
	g_error ("%s: No hosts entries found.", filename);
	config_destroy(&hcfg);
	return (FALSE);
    }

    GList *hosts = NULL;
    int i;
    config_setting_t *cfggroup;
    for(i=0; (cfggroup = config_setting_get_elem(cfghosts, i)); i++) {
	int j;
	config_setting_t *cfghost;
	for(j=0; (cfghost = config_setting_get_elem(cfggroup, j)); j++) {
	    HostNode *hostnode = g_new0(HostNode, 1);

#ifndef NDEBUG
	    hostnode->_type = T_HOSTNODE;
#endif
	    HCFG_GET_STRING("hostname", hostnode->hostname, NULL);
	    HCFG_GET_STRING("Type", hostnode->type, "generic-ssh");
	    HCFG_GET_STRING("ssh_user", hostnode->ssh_user, NULL);
	    HCFG_GET_INT("ssh_port", hostnode->ssh_port, 0);
	    HCFG_GET_STRING("ssh_identity", hostnode->identity_file, NULL);

	    hostnode->group = g_strdup(config_setting_name(cfggroup));

	    hostnode->statsfile = g_strdup_printf("%s/%s:%d.stat", cfg->statsdir, hostnode->hostname, hostnode->ssh_port);
	    hostnode->fdlock = -1;
	    hostnode->uuid[0] = 0;
	    hostnode->tagged = FALSE;

	    getUpdatesFromStat(hostnode);

	    hosts = g_list_append(hosts, hostnode);
	}
    }

    config_destroy(&hcfg);
    return hosts;
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
  g_key_file_free(keyfile);
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

  gchar *host_type = g_key_file_get_string(keyfile, groups[i], "Type", NULL);
  if(!host_type)
   host_type = "generic-ssh";

  for(j = 0; j < lenkey; j++) {
   hostnode = g_new0(HostNode, 1);
#ifndef NDEBUG
   hostnode->_type = T_HOSTNODE;
#endif

   *hostname = *ssh_user = 0; ssh_port = 0;

   if(sscanf(khosts[j], "%255[a-zA-Z0-9_-.]@%255[a-zA-Z0-9-.]:%d", ssh_user, hostname, &ssh_port) != 3) {
    *hostname = *ssh_user = 0; ssh_port = 0;
    if(sscanf(khosts[j], "%255[a-zA-Z0-9-.]:%d", hostname, &ssh_port) != 2) {
     *hostname = *ssh_user = 0; ssh_port = 0;
     if(sscanf(khosts[j], "%255[a-zA-Z0-9_-.]@%255[a-zA-Z0-9-.]", ssh_user, hostname) != 2) {
      *hostname = *ssh_user = 0; ssh_port = 0;
      sscanf(khosts[j], "%255s", hostname);
     }
    }
   }

   hostnode->hostname = g_strdup(hostname);
   hostnode->ssh_user = *ssh_user ? g_strdup(ssh_user) : NULL;
   hostnode->ssh_port = ssh_port ? ssh_port : 0;

   hostnode->statsfile = g_strdup_printf("%s/%s:%d.stat", cfg->statsdir, hostnode->hostname, hostnode->ssh_port);

   hostnode->fdlock = -1;
   hostnode->identity_file = g_key_file_get_string(keyfile, groups[i],
						  "IdentityFile", &error);
   hostnode->tagged = FALSE;
   g_clear_error(&error);

   hostnode->group = groups[i];
   hostnode->type = host_type;

   hostnode->uuid[0] = 0;

   getUpdatesFromStat(hostnode);

   hosts = g_list_append(hosts, hostnode);

   g_free(khosts[j]);
  }

  g_free(khosts);
 }

 g_free(groups);

 g_clear_error(&error);
 g_key_file_free(keyfile);

 return (hosts);
}
