/* $Id$
 *
 * Lock file handling
 */

#include "apt-dater.h"
#include "lock.h"
#include <sys/file.h>
#include <errno.h>

gchar *getLockFile(const gchar *hostname)
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
  g_error("Can't get the name of the lock file");
  return(EXIT_FAILURE);
 }

 if(n->fdlock <= 0) {
   if(!(n->fdlock = open(lockfile, O_CREAT|O_APPEND))) {
     g_warning("%s: %s", lockfile, strerror(errno));
     g_free(lockfile);
     return(EXIT_FAILURE);
   }
 }

#ifdef HAVE_FLOCK
 r = flock(n->fdlock, LOCK_EX | LOCK_NB);
#else
 g_warning("Can't lock to file %s because function flock() is missing!",
	   lockfile);
 g_free(lockfile);
 return(EXIT_FAILURE);
#endif

 g_free(lockfile);

 if(r == -1) {
   close(n->fdlock);
   n->fdlock = -1;
 }

 return(r);
}


int unsetLockForHost(HostNode *n)
{
 int r;

 if(n->fdlock <= 0) {
  return(EXIT_FAILURE);
 }

#ifdef HAVE_FLOCK
 r = flock(n->fdlock, LOCK_UN);
#endif

 close(n->fdlock);
 n->fdlock = -1;
  
 return(r);
}
