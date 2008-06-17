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


int setLockForHost(const gchar *hostname, FILE *fd)
{
 int r;
 gchar *lockfile = NULL;
 
 if(!(lockfile = getLockFile(hostname))) {
  g_error("Can't get the name of the lock file");
  return(EXIT_FAILURE);
 }

 if(!(fd = fopen(lockfile, "w"))) {
  g_warning("%s: %s", lockfile, strerror(errno));
  g_free(lockfile);
  return(EXIT_FAILURE);
 }

#ifdef HAVE_FLOCK
 r = flock(fileno(fd), LOCK_EX | LOCK_NB);
#else
 g_warning("Can't lock to file %s because function flock() is missing!",
	   lockfile);
 g_free(lockfile);
 return(EXIT_FAILURE);
#endif

 g_free(lockfile);

 if(r == -1)
  fclose(fd);

 return(r);
}


int unsetLockForHost(const gchar *hostname, FILE *fd)
{
 int r;

 if(!fd) {
  return(EXIT_FAILURE);
 }

#ifdef HAVE_FLOCK
 r = flock(fileno(fd), LOCK_UN);
#endif

 fclose(fd);
  
 return(r);
}
