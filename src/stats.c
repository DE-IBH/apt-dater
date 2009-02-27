/* apt-dater - terminal-based remote package update manager
 *
 * $Id$
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2009 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
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

#include <glib-2.0/glib/gstdio.h>

#include "apt-dater.h"
#include "exec.h"
#include "screen.h"
#include "stats.h"
#include "lock.h"
#include "autoref.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

gchar *getStatsFile(const HostNode *n)
{
 gchar *statsfile = getStatsFileName(n);

 if(g_file_test(statsfile, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_EXISTS) == FALSE) {
   g_free(statsfile);
   statsfile = NULL;
 }

 return(statsfile);
}


gchar *getStatsFileName(const HostNode *n)
{
 return g_strdup_printf("%s/%s:%d.stat", cfg->statsdir, n->hostname, n->ssh_port);
}


gboolean prepareStatsFile(HostNode *n)
{
 gchar *statsfile = getStatsFileName(n);

 g_unlink(statsfile);

 n->fpstat = fopen(statsfile, "wx");

 g_free(statsfile);

 return n->fpstat != NULL;
}


void refreshStatsOfNode(gpointer n)
{
 if( ((HostNode *)n)->fpstat ) {
    fclose(((HostNode *)n)->fpstat);
    ((HostNode *)n)->fpstat = NULL;
 }

 getUpdatesFromStat(((HostNode *) n));

 unsetLockForHost((HostNode *) n);

 rebuilddl = TRUE; /* Trigger a DrawList rebuild */
}


gboolean setStatsFileFromIOC(GIOChannel *ioc, GIOCondition condition,
			     gpointer n)
{
 gchar *buf = NULL;
 GError *error = NULL;
 gsize bytes;
 gboolean r = TRUE;
 GIOStatus iostatus =  G_IO_STATUS_NORMAL;

 if(!n) return(FALSE);

 if (condition & (G_IO_HUP | G_IO_PRI | G_IO_IN))
  {
   buf = (gchar *) g_malloc0 (g_io_channel_get_buffer_size (ioc) + 1);

   r = FALSE;

   while(iostatus == G_IO_STATUS_NORMAL && ((HostNode *) n)->fpstat) {
    iostatus = g_io_channel_read_chars (ioc, buf,
					g_io_channel_get_buffer_size (ioc), 
					&bytes, NULL);

    if(iostatus == G_IO_STATUS_ERROR || iostatus == G_IO_STATUS_AGAIN)
     break;

    fwrite(buf, sizeof(gchar), bytes, ((HostNode *) n)->fpstat);

    r = TRUE;
   }

   if(r == TRUE)
    fflush(((HostNode *) n)->fpstat);

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

 return(r);
}


void freePkgNode(PkgNode *n)
{
 if(n) {
  g_free(n->package);
  g_free(n->version);
  if(n->data)
   g_free(n->data);
  g_free(n);
 }
}


static void freePackages(HostNode *n)
{
 if(n && n->packages) {
#ifdef FEAT_AUTOREF
  autoref_rem_host_info(n);
#endif
  g_list_foreach(n->packages, (GFunc) freePkgNode, NULL);
  g_list_free(n->packages);
  n->packages = NULL;
 }
}


static gint cmpPackages(gconstpointer a, gconstpointer b) {
 return((((PkgNode *)a)->flag != ((PkgNode *)b)->flag) ? 
	((PkgNode *)a)->flag - ((PkgNode *)b)->flag : 
	g_ascii_strcasecmp(((PkgNode *)a)->package, ((PkgNode *)b)->package));
}

gboolean getUpdatesFromStat(HostNode *n)
{
 gchar *statsfile;
 char line[STATS_MAX_LINE_LEN];
 char buf[256] = "\0";
 FILE  *fp;
 PkgNode *pkgnode = NULL;
 gint status=0, i;
 gchar **argv = NULL;

 if(!n) return (FALSE);

 if(!(statsfile = getStatsFile(n))) {
  n->category = C_UNKNOWN;
  return (TRUE);
 }

#ifdef FEAT_AUTOREF
 struct stat sbuf;
 if(!stat(statsfile, &sbuf))
  n->last_upd = sbuf.st_mtime;
#endif

 if(!(fp = (FILE *) g_fopen(statsfile, "r"))) {
  n->category = C_UNKNOWN;
  g_free(statsfile);
  return (TRUE);
 }

 n->status = 0;
 n->nupdates = 0;
 n->nextras = 0;
 n->nholds = 0;
 n->nbrokens = 0;
 n->forbid = 0;

 freePackages(n);

 if(n->lsb_distributor) {
  g_free(n->lsb_distributor);
  n->lsb_distributor = NULL;
 }
 if(n->lsb_release) {
  g_free(n->lsb_release);
  n->lsb_release = NULL;
 }
 if(n->lsb_codename) {
  g_free(n->lsb_codename);
  n->lsb_codename = NULL;
 }
 if(n->uname_kernel) {
  g_free(n->uname_kernel);
  n->uname_kernel = NULL;
 }
 if(n->uname_machine) {
  g_free(n->uname_machine);
  n->uname_kernel = NULL;
 }
 if(n->virt) {
  g_free(n->virt);
  n->virt = NULL;
 }
 if(n->kernelrel) {
  g_free(n->kernelrel);
  n->kernelrel = NULL;
 }

 int linesok = 0;
 while(fgets(line, STATS_MAX_LINE_LEN, fp)) {
  /* Remove any whitespace from the line. */
  g_strchomp(line);

  if (sscanf((gchar *) line, "KERNELINFO: %d %255s", &status, buf)) {
   n->kernelrel = g_strdup(buf);
   switch(status){
   case 1:
    n->status = n->status | HOST_STATUS_KERNELNOTMATCH;
    break;
   case 2:
    n->status = n->status | HOST_STATUS_KERNELSELFBUILD;
    break;
   }
   linesok++;
   continue;
  }

  if(!strncmp("STATUS: ", line, 8)) {
    argv = g_strsplit(&line[8], "|", 0);

    if(!argv) continue;

    i=0;
    while(argv[i]) i++;

    /* ignore invalid lines */
    if(i < 3) {
     g_strfreev(argv);
     continue;
    }

    pkgnode = g_new0(PkgNode, 1);
#ifndef NDEBUG
    pkgnode->_type = T_PKGNODE;
#endif
    pkgnode->package = g_strdup(argv[0]);
    pkgnode->version = g_strdup(argv[1]);

    if (strlen(argv[2]) > 3)
	pkgnode->data = g_strdup(&argv[2][2]);

    switch(argv[2][0]) {
	case 'u':
	    n->status = n->status | HOST_STATUS_PKGUPDATE;
	    pkgnode->flag = HOST_STATUS_PKGUPDATE;
	    n->nupdates++;
	    break;
	case 'h':
	    n->status = n->status | HOST_STATUS_PKGKEPTBACK;
	    pkgnode->flag = HOST_STATUS_PKGKEPTBACK;
	    n->nholds++;
	    break;
	case 'x':
	    n->status = n->status | HOST_STATUS_PKGEXTRA;
	    pkgnode->flag = HOST_STATUS_PKGEXTRA;
	    n->nextras++;
	    break;
	case 'b':
	    n->status = n->status | HOST_STATUS_PKGBROKEN;
	    pkgnode->flag = HOST_STATUS_PKGBROKEN;
	    n->nbrokens++;
	    break;
	case 'i':
	    break;
	default:
	    g_free(pkgnode);
	    g_strfreev(argv);
	    continue;
    }
    g_strfreev(argv);

    n->packages = g_list_insert_sorted(n->packages, pkgnode, cmpPackages);

    linesok++;
    continue;
  }

  if(!strncmp("LSBREL: ", line, 8)) {
   argv = g_strsplit(&line[8], "|", 0);

   if(!argv) continue;

   i=0;
   while(argv[i]) i++;

   /* ignore invalid lines */
   if(i < 3) {
    g_strfreev(argv);
    continue;
   }

   if(strlen(argv[0]) > 0) {
    n->lsb_distributor = g_strdup(argv[0]);
    n->lsb_release = g_strdup(argv[1]);
    n->lsb_codename = g_strdup(argv[2]);
   }

   g_strfreev(argv);

   linesok++;
   continue;
  }

  if (sscanf((gchar *) line, "VIRT: %255s", buf)) {
   n->virt = g_strdup(buf);

   if (strcmp(n->virt, "Unknown") && strcmp(n->virt, "Physical"))
    n->status = n->status | HOST_STATUS_VIRTUALIZED;

   linesok++;
   continue;
  }

  if (sscanf((gchar *) line, "UNAME: %255s", buf)) {
   gchar *s = strchr(buf, '|');
   if (s) {
    s[0] = 0;

    n->uname_kernel = g_strdup(buf);
    n->uname_machine = g_strdup(s+1);

    linesok++;
   }
   continue;
  }

  if (sscanf((gchar *) line, "FORBID: %d", &n->forbid))
   continue;
 }

 if(linesok>5) {
   if (n->status & HOST_STATUS_PKGBROKEN)
    n->category = C_BROKEN_PKGS;
   else if(n->status & HOST_STATUS_PKGUPDATE)
    n->category = C_UPDATES_PENDING;
   else
    n->category = C_UP_TO_DATE;
#ifdef FEAT_AUTOREF
    autoref_add_host_info(n);
#endif
 }
 else
    n->category = C_UNKNOWN;

 fclose(fp);
 g_free(statsfile);

 return(TRUE);
}


gboolean refreshStats(GList *hosts)
{
 GList *ho;
 gboolean r = TRUE;
 static gboolean upd_pending = FALSE;
 gboolean n_upd_pending = FALSE;

 ho = g_list_first(hosts);

#ifdef FEAT_AUTOREF
 static gboolean do_autoref = TRUE;
 gint num_in_refresh = 0;
#endif
 while(ho) {
  HostNode *n = (HostNode *) ho->data;

  if(screen_get_sessions(n)) {
   n->category = C_SESSIONS;
   rebuilddl = TRUE;
  }
  else {
   if(n->category == C_UPDATES_PENDING)
    n_upd_pending = TRUE;

   if (n->category == C_SESSIONS)
    n->category = C_REFRESH_REQUIRED;

   if(n->category == C_REFRESH_REQUIRED) {
    /* Try to get a lock for the host. */
    gint rsetlck = setLockForHost(n);

    /* We don't got the lock. */
    if(rsetlck == -1) {
     n->status|= HOST_STATUS_LOCKED;
     freePackages(n);
    }
    /* We got the lock. */
    else if (rsetlck == 0) {
     if(n->status & HOST_STATUS_LOCKED) {
      refreshStatsOfNode(n);
      /* n->status ^= HOST_STATUS_LOCKED; */
     } else {
      n->category = C_REFRESH;
      rebuilddl = TRUE;

      freePackages(n);

      if(ssh_cmd_refresh(n) == FALSE) {
       n->category = C_UNKNOWN;
      }
     }
    /* Something weird happend. */
    } else {
     n->category = C_UNKNOWN;
     rebuilddl = TRUE;
    }
   }
  }

#ifdef FEAT_AUTOREF
  if(n->category == C_REFRESH)
   num_in_refresh++;
#endif

  ho = g_list_next(ho);
 }

#ifdef FEAT_TCLFILTER
 applyFilter(hosts);
#endif

 if(n_upd_pending && !upd_pending)
  notifyUser();
 upd_pending = n_upd_pending;

#ifdef FEAT_AUTOREF
 if(cfg->auto_refresh) {
  if (num_in_refresh == 0) {
   if(do_autoref) {
    if (autoref_trigger_auto())
     drawStatus ("Auto refresh triggered...", FALSE);
    do_autoref = FALSE;
   }
  }
  else
   do_autoref = TRUE;
 }
#endif

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

  if((statsfile = getStatsFile(n)))
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
