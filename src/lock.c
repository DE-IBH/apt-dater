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

#include "apt-dater.h"
#include "lock.h"
#include "stats.h"
#include <sys/file.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

static GList *lockList = NULL;

static gchar *getLockFile(const HostNode *n)
{
 gchar *lockfile = NULL;
 gchar *statsfile = NULL;

 statsfile = getStatsFileName(n);

 lockfile = g_strdup_printf("%s.lck", statsfile);
 g_free(statsfile);

 return(lockfile);
}


int setLockForHost(HostNode *n)
{
 int r;
 gchar *lockfile = NULL;

 if(!(lockfile = getLockFile(n))) {
  g_error(_("Can't get the name of the lock file!"));
  return(EXIT_FAILURE);
 }

 if(n->fdlock < 0) {
  if((n->fdlock = open(lockfile, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR))< 0) {
     g_error("%s: %s", lockfile, strerror(errno));
     g_free(lockfile);
     return(EXIT_FAILURE);
   }

   fcntl(n->fdlock, F_SETFD, FD_CLOEXEC | fcntl(n->fdlock, F_GETFD));
 }

#ifdef HAVE_FLOCK
 lockList = g_list_prepend(lockList, n);
 do {
   r = flock(n->fdlock, LOCK_EX | LOCK_NB);
 } while((r==-1) && (errno == EINTR));

 if((r==-1) && (errno != EWOULDBLOCK)) {
   g_error(_("Failed to get lockfile %s: %s"),
	   lockfile, g_strerror(errno));
 }
#else
 g_warning(_("Can't lock to file %s because function flock() is missing!"),
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
