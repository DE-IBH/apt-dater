/* $Id$
 *
 */

#include "apt-dater.h"
#include "screen.h"

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
 g_free(cfg->cmd_refresh);
 g_free(cfg->cmd_upgrade);
 g_free(cfg->cmd_install);

 g_free(cfg);
}

void freeHostNode(HostNode *n)
{
 g_free(n->hostname);
 g_free(n->group);
 g_free(n->ssh_user);
 freeUpdates(n->updates);
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

 if(!(lcfg->hostsfile = 
      g_key_file_get_string(keyfile, "Paths", "HostsFile", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (NULL);
 }
 if(!(lcfg->statsdir =
      g_key_file_get_string(keyfile, "Paths", "StatsDir", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (NULL);
 }

 if(!(lcfg->ssh_cmd = 
      g_key_file_get_string(keyfile, "SSH", "Cmd", &error))) {
  g_error ("%s: %s", filename, error->message);
  return (NULL);
 }

 lcfg->ssh_optflags = NULL;
 if(!(lcfg->ssh_optflags = 
      g_key_file_get_string(keyfile, "SSH", "OptionalCmdFlags", &error))) {
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
 if (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND)
   lcfg->use_screen = g_file_test(SCREEN_BINARY, G_FILE_TEST_IS_EXECUTABLE);

 lcfg->dump_screen = !g_key_file_get_boolean(keyfile, "Screen", "NoDumps", &error);
 if (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND)
   lcfg->dump_screen = TRUE;

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

   hostnode->group = g_strdup(groups[i]);
   hostnode->updates = g_list_alloc();
   hostnode->category = getUpdatesFromStat(hostnode->hostname, hostnode->updates, &hostnode->status);

   if(hostnode->category != C_UPDATES_PENDING) {
    g_list_free(hostnode->updates);
    hostnode->updates = NULL;
   }

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
