/* $Id$
 *
 */

#include "apt-dater.h"


gchar *getStatsFile(gchar *hostname)
{
 GDir *dir;
 GError *error = NULL;
 const gchar *entry;
 GFileTest test;
 gchar *compare;
 gchar *statsfile = NULL;
 
 if(!(dir = g_dir_open(cfg->statsdir, 0, &error))) {
  g_error ("%s: %s", cfg->statsdir, error->message);
  return (NULL);
 }
 g_clear_error(&error);

 while(entry = g_dir_read_name(dir)) {
  compare = g_strdup_printf("%s.stat", hostname);
  if(!g_strcasecmp(entry, compare)) {
   statsfile = g_strdup_printf("%s/%s", cfg->statsdir, entry);

   test = G_FILE_TEST_IS_REGULAR | G_FILE_TEST_EXISTS;
   if(g_file_test (statsfile, test) == FALSE) {
    g_free(statsfile);
    statsfile = NULL;
   }
   
   g_free(compare);
   break;
  }
  g_free(compare);
 }

 g_dir_close(dir);

 return(statsfile);
}


gchar *getStatsFileName(gchar *hostname)
{
 gchar *statsfile = NULL;

 if(hostname) {
  statsfile = g_strdup_printf("%s/%s.stat", cfg->statsdir, hostname);
 }

 return(statsfile);
}


gboolean removeStatsFile(gchar *hostname)
{
 gboolean r = FALSE;
 gchar *statsfile;

 if(hostname) {
  statsfile = getStatsFileName(hostname);

  if(!g_unlink(statsfile)) r = TRUE;

  g_free(statsfile);  
 }

 return(r);

}


void refreshStatsOfNode(gpointer n)
{
 ((HostNode *) n)->updates = g_list_alloc();
 ((HostNode *) n)->category = getUpdatesFromStat(((HostNode *) n)->hostname, 
						 ((HostNode *) n)->updates,
						 &(((HostNode *) n)->status));
 if(((HostNode *) n)->category != C_UPDATES_PENDING) {
  g_list_free(((HostNode *) n)->updates);
  ((HostNode *) n)->updates = NULL;
 }

 rebuilddl = TRUE; /* Trigger a DrawList rebuild */
}


gboolean setStatsFileFromIOC(GIOChannel *ioc, GIOCondition condition,
			     gpointer n)
{
 gchar *statsfile = NULL;
 gchar *buf = NULL;
 GError *error = NULL;
 gsize bytes;
 gboolean r = TRUE;
 FILE *fp = NULL;
 GIOStatus iostatus =  G_IO_STATUS_NORMAL;

 if(!n) return(FALSE);

 statsfile = getStatsFileName(((HostNode *) n)->hostname);

 if (condition & (G_IO_HUP | G_IO_PRI))
  {
   buf = (gchar *) g_malloc0 (g_io_channel_get_buffer_size (ioc) + 1);

   r = FALSE;
   fp = fopen(statsfile, "a");

   while(iostatus == G_IO_STATUS_NORMAL && fp) {
    iostatus = g_io_channel_read_chars (ioc, buf,
					g_io_channel_get_buffer_size (ioc), 
					&bytes, NULL);

    if(iostatus == G_IO_STATUS_ERROR || iostatus == G_IO_STATUS_AGAIN)
     break;

    fwrite(buf, sizeof(gchar), bytes, fp);

    r = TRUE;
   }

   if(r == TRUE) fclose(fp);

   g_free(buf);
  }


 if (condition & (G_IO_HUP | G_IO_ERR | G_IO_NVAL))
  {
   g_io_channel_unref (ioc);
   if(g_io_channel_shutdown (ioc, TRUE, &error) == G_IO_STATUS_ERROR) {
    g_warning("%s", error->message);
    g_clear_error (&error);
   }

   r = FALSE;
  }

 g_free(statsfile);

 return(r);
}


Category getUpdatesFromStat(gchar *hostname, GList *updates, guint *stat)
{
 gchar *statsfile;
 char line[STATS_MAX_LINE_LEN];
 char package[BUF_MAX_LEN];
 char oldver[BUF_MAX_LEN];
 char newver[BUF_MAX_LEN];
 char section[BUF_MAX_LEN];
 char dist[BUF_MAX_LEN];
 FILE  *fp;
 gsize len;
 UpdNode *updnode = NULL;
 Category category = C_UNKNOW;
 gint i = 0;
 gint cnt_upgraded, cnt_newly_installed, cnt_remove, cnt_not_upgraded;
 gint status;

 if(!(statsfile = getStatsFile(hostname))) {
  category = C_NO_STATS;
  return(category);
 }

 if(!(fp = (FILE *) g_fopen(statsfile, "r"))) {
  g_free(statsfile);
  return(category);
 }

 cnt_upgraded = cnt_newly_installed = cnt_remove = cnt_not_upgraded = -1;
 *stat = 0;

 while(fgets(line, STATS_MAX_LINE_LEN, fp)) {
  line[strlen(line) - 1] = 0;

  if (sscanf((gchar *) line, "%d upgraded, %d newly installed, %d to remove and %d not upgraded.", &cnt_upgraded, &cnt_newly_installed, &cnt_remove, &cnt_not_upgraded)) continue;

  if (sscanf((gchar *) line, "KERNELINFO: %d", &status)) {
   switch(status){
   case 1:
    *stat = *stat | 2;
    break;
   case 2:
    *stat = *stat | 4;
    break;
   }
   continue;
  }

  if  (!strcmp(line,"The following packages have been kept back:")) {
   *stat = *stat | 1;
   continue;
  }

  *package = *oldver = *newver = *section = *dist = 0;
  sscanf((gchar *) line, "Inst %s [%[a-zA-Z0-9-+.:~]] (%s %[a-zA-Z0-9-+.]:%[a-zA-Z0-9-+.])", package, oldver, newver, section, dist);

  if(*package) {
   updnode = g_new0(UpdNode, 1);
   updnode->package = g_strdup(package);
   updnode->oldver = g_strdup(oldver);
   updnode->newver = g_strdup(newver);
   updnode->section = g_strdup(section);
   updnode->dist = g_strdup(dist);

   if(!i) {
    updates->data = updnode;
    i++;
   }
   else
    updates = g_list_append(updates, updnode);

   category = C_UPDATES_PENDING;
  }
 }

 if(cnt_upgraded == 0) {
  category = C_UP_TO_DATE;
 }

 fclose(fp);
 g_free(statsfile);

 return(category);
}


void freeUpdNode(UpdNode *n)
{
 if(n) {
  g_free(n->package);
  g_free(n->oldver);
  g_free(n->newver);
  g_free(n->section);
  g_free(n->dist);

  g_free(n);
 }
}


void freeUpdates(GList *updates)
{
 if(updates) {
  g_list_foreach (updates, (GFunc) freeUpdNode, NULL);
  g_list_free(updates);
 }
}


gboolean refreshStats(GList *hosts)
{
 GList *ho;
 HostNode *n;
 gboolean r = TRUE;

 ho = g_list_first(hosts);
 
 while(ho) {
  n = (HostNode *) ho->data;

  if(n->category == C_REFRESH_REQUIRED) {
   n->category = C_REFRESH;
   freeUpdates(n->updates);

   if(ssh_cmd_refresh(n->hostname, n->ssh_user, n->ssh_port, n) == FALSE) {
    n->category = C_NO_STATS;
   }
  }

  ho = g_list_next(ho);
 }

 return(r);
}
