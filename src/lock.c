/* $Id$
 *
 * Lock file handling
 */

#include "apt-dater.h"
#include "lock.h"
#include "stats.h"
#include <sys/file.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

static GList *lockList = NULL;

static gchar *getLockFile(const gchar *hostname)
{
 gchar *lockfile = NULL;
 gchar *statsfile = NULL;
 
 statsfile = getStatsFileName(hostname);

 lockfile = g_strdup_printf("%s.lck", statsfile);
 g_free(statsfile);

 return(lockfile);
}


int setLockForHost(HostNode *n)
{
 int r;
 gchar *lockfile = NULL;
 
 if(!(lockfile = getLockFile(n->hostname))) {
  g_error("Can't get the name of the lock file!");
  return(EXIT_FAILURE);
 }

 if(n->fdlock < 0) {
   if((n->fdlock = open(lockfile, O_CREAT))< 0) {
     g_error("%s: %s", lockfile, strerror(errno));
     g_free(lockfile);
     return(EXIT_FAILURE);
   }
 }

#ifdef HAVE_FLOCK
 lockList = g_list_prepend(lockList, n);
 do {
   r = flock(n->fdlock, LOCK_EX | LOCK_NB);
 } while((r==-1) && (errno == EINTR));

 if((r==-1) && (errno != EWOULDBLOCK)) {
   g_error("Failed to get lockfile %s: %s",
	   lockfile, g_strerror(errno));
 }
#else
 g_warning("Can't lock to file %s because function flock() is missing!",
	   lockfile);
 g_free(lockfile);
 return(EXIT_FAILURE);
#endif

 g_free(lockfile);

 if(r == -1) {
   lockList = g_list_remove(lockList, n);
   close(n->fdlock);
   n->fdlock = -1;
 }

 return(r);
}


int unsetLockForHost(HostNode *n)
{
 int r;

 if(n->fdlock < 0) {
  return(EXIT_FAILURE);
 }

#ifdef HAVE_FLOCK
 r = flock(n->fdlock, LOCK_UN);
 lockList = g_list_remove(lockList, n);
#endif

 close(n->fdlock);
 n->fdlock = -1;
  
 return(r);
}

static void cleanupLock(gpointer data, gpointer user_data) {
    unsetLockForHost((HostNode *)data);
}

void cleanupLocks() {
    g_list_foreach(lockList, cleanupLock, NULL);
}
