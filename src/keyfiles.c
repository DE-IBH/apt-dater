/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2014 (C) IBH IT-Service GmbH [https://www.ibh.de/apt-dater/]
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
#include <libxml/parser.h>
#include <libxml/xpath.h>

#ifdef __linux
EXTLD(apt_dater_config);
EXTLD(hosts_config);
EXTLD(screenrc);
#endif

#ifdef __linux
#define DUMP_CONFIG(fn, bin)						\
  pathtofile = g_strdup_printf("%s/%s", cfgdir, (fn));			\
  if(!pathtofile) g_error(_("Out of memory."));				\
  if(g_file_test(pathtofile, G_FILE_TEST_IS_REGULAR|G_FILE_TEST_EXISTS) == FALSE) { \
    FILE *fp = NULL;							\
    fp = fopen(pathtofile, "wx");					\
    g_message(_("Creating default config file %s"), pathtofile);	\
    if(fp) {								\
      if(fwrite(LDVAR(bin), LDLEN(bin), 1, fp) != 1) g_error(_("Could not write to file %s."), pathtofile); \
      fclose(fp);							\
    }									\
  }									\
  g_free(pathtofile); 
#else
#define DUMP_CONFIG(fn, bin)						\
  pathtofile = g_strdup_printf("%s/%s", cfgdir, (fn));			\
  if(!pathtofile) g_error(_("Out of memory."));				\
  if(g_file_test(pathtofile, G_FILE_TEST_IS_REGULAR|G_FILE_TEST_EXISTS) == FALSE) \
    g_error(_("Mandatory config file %s does not exist!"), pathtofile)
#endif

int chkForInitialConfig(const gchar *cfgdir, const gchar *cfgfile) {
  if(g_file_test(cfgdir, G_FILE_TEST_IS_DIR) == FALSE) {
    if(g_mkdir_with_parents (cfgdir, 0700)) return(1);
  }
  
  gchar *pathtofile;
  DUMP_CONFIG("apt-dater.config", apt_dater_config);
  DUMP_CONFIG("hosts.config", hosts_config);
  DUMP_CONFIG("screenrc", screenrc);

 return(0);
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

    lcfg->hostsfile = g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "hosts.xml");
    lcfg->statsdir = g_strdup_printf("%s/%s/%s", g_get_user_cache_dir(), PROG_NAME, "stats");

    lcfg->screenrcfile = g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "screenrc");

    lcfg->dump_screen = TRUE;
    lcfg->query_maintainer = FALSE;

#ifdef FEAT_AUTOREF
    lcfg->auto_refresh = TRUE;
#endif

    lcfg->beep = TRUE;
    lcfg->flash = TRUE;

#ifdef FEAT_HISTORY
    lcfg->record_history = TRUE;
    lcfg->history_errpattern = "((?<!no )error|warning|fail)";
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
    if((setting)) \
        config_setting_lookup_bool((setting), (key), &(var));
#define CFG_GET_STRING_DEFAULT(setting,key,var,def) \
    var = (def); \
    if((setting)) \
        config_setting_lookup_string((setting), (key), (const char **) &(var)); \
    if(var != NULL) var = g_strdup(var);

gboolean loadConfig(char *filename, CfgFile *lcfg) {

    config_t hcfg;

    config_init(&hcfg);
    if(config_read_file(&hcfg, filename) == CONFIG_FALSE) {
#ifdef HAVE_LIBCONFIG_ERROR_MACROS
      const char *efn = config_error_file(&hcfg);
    g_printerr ("Error reading config file [%s:%d]: %s\n", (efn ? efn : filename), config_error_line(&hcfg), config_error_text(&hcfg));
#else
      g_printerr ("Error reading config file %s!\n", filename);
#endif
	config_destroy(&hcfg);
	return (FALSE);
    }

#define GETREQ_SETTING_T(varn, sectn) \
    config_setting_t *(varn) = config_lookup(&hcfg, (sectn)); \
    if(!(varn)) {					      \
        g_error ("Missing section %s in config file %s!", (sectn), filename); \
	config_destroy(&hcfg); \
	return (FALSE); \
    }

    //    config_setting_t *s_ssh = config_lookup(&hcfg, "SSH");
    GETREQ_SETTING_T(s_ssh, "SSH");
    config_setting_t *s_paths = config_lookup(&hcfg, "Paths");
    config_setting_t *s_screen = config_lookup(&hcfg, "Screen");
    config_setting_t *s_appearance = config_lookup(&hcfg, "Appearance");
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

    gchar *h;
    config_setting_lookup_string(s_ssh, "OptionalCmdFlags", (const char **) &h);
    lcfg->ssh_optflags = (h ? g_strdup(h) : NULL);

    CFG_GET_STRING_DEFAULT(s_paths, "HostsFile", lcfg->hostsfile, g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "hosts.xml"));
    CFG_GET_STRING_DEFAULT(s_paths, "StatsDir", lcfg->statsdir, g_strdup_printf("%s/%s/%s", g_get_user_cache_dir(), PROG_NAME, "stats"));
    g_mkdir_with_parents(lcfg->statsdir, S_IRWXU | S_IRWXG | S_IRWXO);

    CFG_GET_STRING_DEFAULT(s_screen, "RCFile", lcfg->screenrcfile, g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "screenrc"));
    CFG_GET_STRING_DEFAULT(s_screen, "Title", lcfg->screentitle, g_strdup("%m # %U%H"));

    h = NULL;
    if(config_setting_lookup_string(s_ssh, "Cmd", (const char **) &h) == CONFIG_FALSE) {
	g_printerr ("%s: Config option SSH.Cmd not set!", filename);
	return (FALSE);
    }
    lcfg->ssh_cmd = g_strdup(h);

    h = NULL;
    if(config_setting_lookup_string(s_ssh, "SFTPCmd", (const char **) &h) == CONFIG_FALSE) {
	g_printerr ("%s: Config option SSH.SFTPCmd not set!", filename);
	return (FALSE);
    }
    lcfg->sftp_cmd = g_strdup(h);

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
	    g_printerr ("%s: setting %s must be a single string or an array of strings", filename, config_setting_name(s_addkeys));
	}
    }

    CFG_GET_BOOL_DEFAULT(s_screen, "NoDumps", lcfg->dump_screen, FALSE);
    lcfg->dump_screen = !lcfg->dump_screen;

    CFG_GET_BOOL_DEFAULT(s_screen, "QueryMaintainer", lcfg->query_maintainer, FALSE);

    h = NULL;
    if(config_setting_lookup_string(s_appearance, "Colors", (const char **) &h) != CONFIG_FALSE)
      lcfg->colors = g_strsplit(h, ";", -1);

#ifdef FEAT_TCLFILTER
    config_setting_lookup_string(&s_tclfilter, "FilterExp", &(lcfg->filterexp));
    config_setting_lookup_string(&s_tclfilter, "FilterFile", &(lcfg->filterfile));
#endif

#ifdef FEAT_AUTOREF
    CFG_GET_BOOL_DEFAULT(s_autoref, "Enabled", lcfg->auto_refresh, TRUE);
#endif

    CFG_GET_BOOL_DEFAULT(s_notify, "Beep", lcfg->beep, TRUE);
    CFG_GET_BOOL_DEFAULT(s_notify, "Flash", lcfg->flash, TRUE);

#ifdef FEAT_HISTORY
    CFG_GET_BOOL_DEFAULT(s_history, "Record", lcfg->record_history, TRUE);
    CFG_GET_STRING_DEFAULT(s_history, "ErrPattern", lcfg->history_errpattern, "((?<!no )error|warning|fail)");
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

xmlDocPtr loadXFile(const char *filename) {
  xmlDocPtr xml = xmlParseFile(filename);

  if(xml == NULL) {
    g_printerr ("%s: Error parsing XML document.\n", filename);
    exit(1);
  }

  return xml;
}

xmlXPathObjectPtr evalXPath(xmlXPathContextPtr context, const gchar *xpath) {
  xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar *)xpath, context);
  if(result == NULL) {
    g_warning("xmlXPathEvalExpression '%s' failed!\n", xpath);
    exit(1);
  }

  return result;
}

char *getXPropStr(const xmlNodePtr nodes[], const gchar *attr, const gchar *defval) {
  int i;
  xmlChar *val = NULL;

  for(i = 0; nodes[i]; i++) {
    val = xmlGetProp(nodes[i], (xmlChar *) attr);

    if(val) {
      gchar *ret = g_strdup((gchar *)val);
      xmlFree(val);
      return ret;
    }
  }

  if(defval)
    return g_strdup(defval);

  return NULL;
}

int getXPropInt(const xmlNodePtr nodes[], const gchar *attr, const int defval) {
  gchar *sval = getXPropStr(nodes, attr, NULL);

  if(!sval)
    return defval;

  int ival = atoi(sval);

  g_free(sval);

  return ival;
}

GList *loadHosts (const gchar *filename) {
    /* Parse hosts.xml document. */
    xmlDocPtr xcfg = xmlParseFile(filename);
    if(xcfg == NULL) {
      g_printerr ("%s: Error parsing XML document.\n", filename);
      return(FALSE);
    }
    /* Allocate XPath context. */
    xmlXPathContextPtr xctx = xmlXPathNewContext(xcfg);
    if(!xctx) {
      g_error("%s: xmlXPathNewContext failed!\n", filename);
      return(FALSE);
    }

    /* Lookup global host template node. */
    xmlXPathObjectPtr defaults = evalXPath(xctx, "/hosts/default");
    xmlNodePtr defhost = NULL;
    if(!xmlXPathNodeSetIsEmpty(defaults->nodesetval))
      defhost = defaults->nodesetval->nodeTab[0];
    xmlXPathFreeObject(defaults);

    /* Iterate over /hosts/group nodes. */
    xmlXPathObjectPtr groups = evalXPath(xctx, "/hosts/group");
    int i;
    GList *hostlist = NULL;
    for(i = 0; i < groups->nodesetval->nodeNr; i++) {
      xmlNodePtr group = groups->nodesetval->nodeTab[i];

      xmlChar *groupname = xmlGetProp(group, (xmlChar *)"name");
      if(!groupname) {
	g_printerr("%s: The group element #%d does not have a name attribute!\n", filename, i+1);
	return(FALSE);
      }

      xctx->node = group;
      xmlXPathObjectPtr hosts = evalXPath(xctx, "host");
      if(xmlXPathNodeSetIsEmpty(hosts->nodesetval)) {
	xmlXPathFreeObject(hosts);
	xmlFree(groupname);
	g_warning("%s: The group '%s' is empty!\n", filename, groupname);
	continue;
      }

      /* Iterate over /hosts/group/host nodes. */
      int j;
      for(j = 0; j < hosts->nodesetval->nodeNr; j++) {
	xmlNodePtr host = hosts->nodesetval->nodeTab[j];
	xmlNodePtr cfgnodes[4] = {host, group, defhost, NULL};

	xmlChar *hostname = xmlGetProp(host, (xmlChar *)"name");
	if(!hostname) {
	  g_printerr("%s: The host element #%d of group '%s' does not have a name attribute!\n", filename, j+1, groupname);

	  xmlXPathFreeObject(hosts);
	  xmlXPathFreeObject(groups);
	  xmlFree(groupname);
	  return(FALSE);
	}

	HostNode *hostnode = g_new0(HostNode, 1);
#ifndef NDEBUG
	hostnode->_type = T_HOSTNODE;
#endif
	hostnode->hostname = g_strdup((gchar *)hostname);
	hostnode->comment = getXPropStr(cfgnodes, "comment", NULL);
	hostnode->type = getXPropStr(cfgnodes, "type", "generic-ssh");
	hostnode->ssh_user = getXPropStr(cfgnodes, "ssh-user", NULL);
	hostnode->ssh_host = getXPropStr(cfgnodes, "ssh-host", NULL);
	hostnode->ssh_port = getXPropInt(cfgnodes, "ssh-port", 0);
	hostnode->identity_file = getXPropStr(cfgnodes, "ssh-id", NULL);

	hostnode->group = g_strdup((gchar *)groupname);

	hostnode->statsfile = g_strdup_printf("%s/%s:%d.stat", cfg->statsdir, hostnode->hostname, hostnode->ssh_port);
	hostnode->fdlock = -1;
	hostnode->uuid[0] = 0;
	hostnode->tagged = FALSE;

	getUpdatesFromStat(hostnode);

	hostlist = g_list_append(hostlist, hostnode);

	xmlFree(hostname);
      }

      xmlXPathFreeObject(hosts);
      xmlFree(groupname);
    }

    xmlXPathFreeObject(groups);
    return hostlist;
}
