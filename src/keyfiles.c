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
#include "keyfiles.h"
#include "stats.h"
#include "lock.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <libxml/parser.h>
#include <libxml/xinclude.h>
#include <libxml/xpath.h>

#include "../conf/apt-dater.xml.inc"
#include "../conf/hosts.xml.inc"
#include "../conf/screenrc.inc"

void dump_config(const gchar *dir, const gchar *fn, const gchar *str, const unsigned int len) {
  gchar *pathtofile = g_strdup_printf("%s/%s", dir, (fn));

  if(!pathtofile)
    g_error(_("Out of memory."));

  if(g_file_test(pathtofile, G_FILE_TEST_IS_REGULAR|G_FILE_TEST_EXISTS) == FALSE) {
    FILE *fp = fopen(pathtofile, "wx");
    g_message(_("Creating default config file %s"), pathtofile);
    if(fp) {
      if(fwrite(str, len, 1, fp) != 1) {
	g_printerr(_("Could not write to file %s."), pathtofile);
	exit(1);
      }
      fclose(fp);
    }
  }
  g_free(pathtofile);
}

int chkForInitialConfig(const gchar *cfgdir, const gchar *cfgfile) {
  if(g_file_test(cfgdir, G_FILE_TEST_IS_DIR) == FALSE) {
    if(g_mkdir_with_parents (cfgdir, S_IRWXU)) return(1);
  }

  dump_config(cfgdir, "apt-dater.xml", (gchar *)apt_dater_xml, apt_dater_xml_len);
  dump_config(cfgdir, "hosts.xml", (gchar *)hosts_xml, hosts_xml_len);
  dump_config(cfgdir, "screenrc", (gchar *)screenrc, screenrc_len);

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

    lcfg->dump_screen = TRUE;

#ifdef FEAT_AUTOREF
    lcfg->auto_refresh = TRUE;
#endif

    lcfg->beep = TRUE;
    lcfg->flash = TRUE;

#ifdef FEAT_HISTORY
    lcfg->record_history = TRUE;
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

xmlXPathObjectPtr evalXPath(xmlXPathContextPtr context, const gchar *xpath) {
  xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST(xpath), context);
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
    val = xmlGetProp(nodes[i], BAD_CAST(attr));

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

  int ival = strtol(sval, NULL, 0);

  g_free(sval);

  return ival;
}

int getXPropBool(const xmlNodePtr nodes[], const gchar *attr, const gboolean defval) {
  gchar *val = getXPropStr(nodes, attr, NULL);
  if(!val)
    return defval;

  g_strstrip(val);

  if(g_ascii_strncasecmp(val, "false", 1) == 0 ||
     g_ascii_strncasecmp(val, "no", 1) == 0) {
    g_free(val);
    return FALSE;
  }

  if(g_ascii_strncasecmp(val, "true", 1) == 0 ||
     g_ascii_strncasecmp(val, "yes", 1) == 0) {
    g_free(val);
    return TRUE;
  }

  if(atoi(val)) {
    g_free(val);
    return TRUE;
  }

  g_free(val);
  return FALSE;
}

xmlNodeSetPtr getXNodes(xmlXPathContextPtr context, const gchar *xpath) {
  xmlXPathObjectPtr xobj = xmlXPathEvalExpression(BAD_CAST(xpath), context);
  xmlNodeSetPtr ret = xobj->nodesetval;
  xmlXPathFreeNodeSetList(xobj);

  return ret;
}

xmlNodePtr getXNode(xmlXPathContextPtr context, const gchar *xpath) {
  xmlNodeSetPtr set = getXNodes(context, xpath);
  xmlNodePtr ret = NULL;

  if(!xmlXPathNodeSetIsEmpty(set))
    ret = set->nodeTab[0];

  xmlXPathFreeNodeSet(set);
  return ret;
}

gboolean loadConfig(const gchar *filename, CfgFile *lcfg) {
    /* Parse hosts.xml document. */
    xmlDocPtr xcfg = xmlParseFile(filename);
    if(xcfg == NULL)
      return(FALSE);

    /* Handle Xincludes. */
    xmlXIncludeProcess(xcfg);

    /* Validate against DTD. */
    xmlValidCtxtPtr xval = xmlNewValidCtxt();
    if(xmlValidateDocument(xval, xcfg) == 0) {
      xmlFreeValidCtxt(xval);
      return(FALSE);
    }
    xmlFreeValidCtxt(xval);

    /* Allocate XPath context. */
    xmlXPathContextPtr xctx = xmlXPathNewContext(xcfg);
    if(!xctx) {
      g_error("%s: xmlXPathNewContext failed!\n", filename);
      return(FALSE);
    }

    xmlNodePtr s_ssh[2] = {getXNode(xctx, "/apt-dater/ssh"), NULL};
    xmlNodePtr s_path[2] = {getXNode(xctx, "/apt-dater/paths"), NULL};
    xmlNodePtr s_screen[2] = {getXNode(xctx, "/apt-dater/screen"), NULL};
    xmlNodePtr s_appearance[2] = {getXNode(xctx, "/apt-dater/appearance"), NULL};
    xmlNodePtr s_notify[2] = {getXNode(xctx, "/apt-dater/notify"), NULL};
    xmlNodePtr s_hooks[2] = {getXNode(xctx, "/apt-dater/hooks"), NULL};
#ifdef FEAT_AUTOREF
    xmlNodePtr s_autoref[2] = {getXNode(xctx, "/apt-dater/auto-ref"), NULL};
#endif
#ifdef FEAT_HISTORY
    xmlNodePtr s_history[2] = {getXNode(xctx, "/apt-dater/history"), NULL};
#endif
#ifdef FEAT_TCLFILTER
    xmlNodePtr s_tclfilter[2] = {getXNode(xctx, "/apt-dater/tcl-filter"), NULL};
#endif

    lcfg->ssh_optflags = getXPropStr(s_ssh, "opt-cmd-flags", "-t");
    lcfg->ssh_cmd = getXPropStr(s_ssh, "cmd", "/usr/bin/ssh");
    lcfg->sftp_cmd = getXPropStr(s_ssh, "sftp-cmd", "/usr/bin/sftp");

    lcfg->umask = getXPropInt(s_path, "umask", S_IRWXG | S_IRWXO);
    umask(lcfg->umask);

    lcfg->hostsfile = getXPropStr(s_path, "hosts-file", g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "hosts.xml"));
    lcfg->statsdir = getXPropStr(s_path, "stats-dir", g_strdup_printf("%s/%s/%s", g_get_user_cache_dir(), PROG_NAME, "stats"));
    g_mkdir_with_parents(lcfg->statsdir, S_IRWXU | S_IRWXG);

    lcfg->screenrcfile = getXPropStr(s_screen, "rc-file", g_strdup_printf("%s/%s/%s", g_get_user_config_dir(), PROG_NAME, "screenrc"));
    lcfg->screentitle = getXPropStr(s_screen, "title", g_strdup("%m # %U%H"));


    lcfg->ssh_agent = getXPropBool(s_ssh, "spawn-agent", FALSE);

    xmlNodeSetPtr s_addkeys = getXNodes(xctx, "/apt-dater/ssh/add-key");
    if(!xmlXPathNodeSetIsEmpty(s_addkeys)) {
      lcfg->ssh_add = g_new0(char*, s_addkeys->nodeNr + 1);
      int i;
      for(i = 0; i < s_addkeys->nodeNr; i++) {
	lcfg->ssh_add[i] = g_strdup((gchar *)xmlGetProp(s_addkeys->nodeTab[i], BAD_CAST("name")));
      }
    }
    xmlXPathFreeNodeSet(s_addkeys);

    lcfg->dump_screen = !getXPropBool(s_screen, "no-dumps", FALSE);
    lcfg->query_maintainer = getXPropBool(s_screen, "query-maintainer", FALSE);

    gchar *colors = getXPropStr(s_appearance, "colors", "menu brightgreen blue;status brightgreen blue;selector black red;");
    if(colors)
      lcfg->colors = g_strsplit(colors, ";", -1);

#ifdef FEAT_TCLFILTER
    lcfg->filterexp = getXPropStr(s_tclfilter, "filter-exp", NULL);
    lcfg->filterfile = getXPropStr(s_tclfilter, "filter-file", NULL);
#endif

#ifdef FEAT_AUTOREF
    lcfg->auto_refresh = getXPropBool(s_autoref, "enabled", TRUE);
#endif

    lcfg->beep = getXPropBool(s_notify, "beep", TRUE);
    lcfg->flash = getXPropBool(s_notify, "flash", TRUE);

#ifdef FEAT_HISTORY
    lcfg->record_history = getXPropBool(s_history, "record", TRUE);
    lcfg->history_errpattern = getXPropStr(s_history, "err-pattern", "((?<!no )error|warning|fail)");
    lcfg->history_dir = getXPropStr(s_path, "history-dir", g_strdup_printf("%s/%s/history", g_get_user_data_dir(), PACKAGE));
#endif

    lcfg->hook_pre_upgrade = getXPropStr(s_hooks, "pre-upgrade", "/etc/apt-dater/pre-upg.d");
    lcfg->hook_pre_refresh = getXPropStr(s_hooks, "pre-refresh", "/etc/apt-dater/pre-ref.d");
    lcfg->hook_pre_install = getXPropStr(s_hooks, "pre-install", "/etc/apt-dater/pre-ins.d");
    lcfg->hook_pre_connect = getXPropStr(s_hooks, "pre-connect", "/etc/apt-dater/pre-con.d");

    lcfg->hook_post_upgrade = getXPropStr(s_hooks, "post-upgrade", "/etc/apt-dater/post-upg.d");
    lcfg->hook_post_refresh = getXPropStr(s_hooks, "post-refresh", "/etc/apt-dater/post-ref.d");
    lcfg->hook_post_install = getXPropStr(s_hooks, "post-install", "/etc/apt-dater/post-ins.d");
    lcfg->hook_post_connect = getXPropStr(s_hooks, "post-connect", "/etc/apt-dater/post-con.d");

    lcfg->plugindir = getXPropStr(s_hooks, "plugin-dir", "/etc/apt-dater/plugins");

    return (TRUE);
}

GList *loadHosts (const gchar *filename) {
    /* Parse hosts.xml document. */
    xmlDocPtr xcfg = xmlParseFile(filename);
    if(xcfg == NULL)
      return(FALSE);

    /* Handle Xincludes. */
    xmlXIncludeProcess(xcfg);

    /* Validate against DTD. */
    xmlValidCtxtPtr xval = xmlNewValidCtxt();
    if(xmlValidateDocument(xval, xcfg) == 0) {
      xmlFreeValidCtxt(xval);
      return(FALSE);
    }
    xmlFreeValidCtxt(xval);

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

      xmlChar *groupname = xmlGetProp(group, BAD_CAST("name"));
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

	xmlChar *hostname = xmlGetProp(host, BAD_CAST("name"));
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
