/* $Id$
 *
 */

#include "apt-dater.h"
#include "screen.h"
#include "keyfiles.h"
#include "stats.h"
#include "lock.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


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

void freeHostNode(HostNode *n)
{
 unsetLockForHost(n);
 g_free(n->hostname);
 g_free(n->group);
 g_free(n->ssh_user);
 g_free(n->lsb_distributor);
 g_free(n->lsb_release);
 g_free(n->lsb_codename);
 g_free(n->kernelrel);
 freePackages(n->packages);
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

   hostnode->group = g_strdup(groups[i]);
   getUpdatesFromStat(hostnode);

   if(hostnode->category != C_UPDATES_PENDING) {
    g_list_free(hostnode->packages);
    hostnode->packages = NULL;
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
