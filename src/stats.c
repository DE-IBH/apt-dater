/* $Id$
 *
 */

#include <glib-2.0/glib/gstdio.h>

#include "apt-dater.h"
#include "exec.h"
#include "screen.h"
#include "stats.h"
#include "lock.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

gchar *getStatsFile(const gchar *hostname)
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

 while(((entry = g_dir_read_name(dir)))) {
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


gchar *getStatsFileName(const gchar *hostname)
{
 gchar *statsfile = NULL;

 if(hostname) {
  statsfile = g_strdup_printf("%s/%s.stat", cfg->statsdir, hostname);
 }

 return(statsfile);
}


gboolean removeStatsFile(const gchar *hostname)
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
 ((HostNode *) n)->category = getUpdatesFromStat(((HostNode *) n)->hostname, 
						 &((HostNode *) n)->updates,
						 &(((HostNode *) n)->status));

 unsetLockForHost((HostNode *) n);

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


static gint cmpUpdates(gconstpointer a, gconstpointer b) {
    return strcmp(((UpdNode *)a)->package, ((UpdNode *)b)->package);
}

Category getUpdatesFromStat(gchar *hostname, GList **updates, guint *stat)
{
 gchar *statsfile;
 char line[STATS_MAX_LINE_LEN];
 FILE  *fp;
 UpdNode *updnode = NULL;
 Category category = C_UNKNOW;
 gint status;

 if(!(statsfile = getStatsFile(hostname))) {
  category = C_NO_STATS;
  return(category);
 }

 if(!(fp = (FILE *) g_fopen(statsfile, "r"))) {
  g_free(statsfile);
  return(category);
 }

 *stat = 0;

 if(*updates) {
    g_list_free(*updates);
    *updates = NULL;
 }

 while(fgets(line, STATS_MAX_LINE_LEN, fp)) {
  line[strlen(line) - 1] = 0;

  if (sscanf((gchar *) line, "KERNELINFO: %d", &status)) {
   switch(status){
   case 1:
    *stat = *stat | HOST_STATUS_KERNELNOTMATCH;
    break;
   case 2:
    *stat = *stat | HOST_STATUS_KERNELSELFBUILD;
    break;
   }
   continue;
  }
  
  if(!strncmp("STATUS: ", line, 8)) {
    gchar **argv = g_strsplit(&line[8], "|", 0);
    
    /* ignore invalid lines */
    if(!argv[0] || !argv[1] || !argv[2])
	continue;
    
    switch(argv[2][0]) {
	case 'u':
	    updnode = g_new0(UpdNode, 1);
	    updnode->package = g_strdup(argv[0]);
	    updnode->oldver = g_strdup(argv[1]);
	    if (strlen(argv[2]) > 3)
		updnode->newver = g_strdup(&argv[2][2]);
	    else
		updnode->newver = g_strdup("???");

	    *updates = g_list_insert_sorted(*updates, updnode, cmpUpdates);

	    break;
	case 'h':
	    *stat = *stat | HOST_STATUS_PKGKEPTBACK;
	    break;
    }
    g_strfreev(argv);

    continue;
  }
 }

 if(g_list_length(*updates))
  category = C_UPDATES_PENDING;
 else
  category = C_UP_TO_DATE;
  
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
 gboolean r = TRUE;

 ho = g_list_first(hosts);
 
 while(ho) {
  HostNode *n = (HostNode *) ho->data;

  if(screen_get_sessions(n)) {
   n->category = C_SESSIONS;
   rebuilddl = TRUE;
  }
  else {
   if (n->category == C_SESSIONS)
    n->category = C_REFRESH_REQUIRED;

   if(n->category == C_REFRESH_REQUIRED) {
    /* Try to get a lock for the host. */
    gint rsetlck = setLockForHost(n);

    /* We don't got the lock. */
    if(rsetlck == -1) {
     n->status|= HOST_STATUS_LOCKED;
     if(n->updates) {
      freeUpdates(n->updates);
      n->updates = NULL;
     }
    }
    /* We got the lock. */
    else if (rsetlck == 0) {
     if(n->status & HOST_STATUS_LOCKED) {
      refreshStatsOfNode(n);
      //      n->status ^= HOST_STATUS_LOCKED;
     } else {
      n->category = C_REFRESH;
      rebuilddl = TRUE;

      freeUpdates(n->updates);
      n->updates = NULL;

      if(ssh_cmd_refresh(n) == FALSE) {
       n->category = C_NO_STATS;
      }
     }
    /* Something weird happend. */
    } else {
     n->category = C_UNKNOW;
     rebuilddl = TRUE;
    }
   }
  }

  ho = g_list_next(ho);
 }

 return(r);
}


void getOldestMtime(GList *hosts)
{
 GList *ho;
 gchar *statsfile;
 struct stat stat_p;
 HostNode *n;

 oldest_st_mtime = time(NULL);

 ho = g_list_first(hosts);
 
 while(ho) {
  n = (HostNode *) ho->data;

  if((statsfile = getStatsFile(n->hostname)))
   {
    if(!stat(statsfile, &stat_p)) {
     if(difftime(stat_p.st_mtime, oldest_st_mtime) < 0)
      oldest_st_mtime = stat_p.st_mtime;
    }

    g_free(statsfile);
   }

  ho = g_list_next(ho);
 }
}
