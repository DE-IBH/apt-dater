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
#include "ui.h"
#include "sighandler.h"
#include "lock.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

static volatile int sigintcnt = 0;
static gboolean ignsigint = FALSE;

static RETSIGTYPE sigintSigHandler(int sig)
{
 switch(sig) {
  case SIGINT:
   if(ignsigint == TRUE) break;
   if (sigintcnt++ > 1) {
    cleanUI();

    cleanupLocks();
    exit(EXIT_FAILURE);
   }
   else {
    cleanUI();
    refreshUI();
    g_main_loop_quit (loop);
   }

   break;
 } /* switch(sig) */
}


static RETSIGTYPE sigtermSigHandler() {
 cleanUI();
 refreshUI();
 g_main_loop_quit (loop);
}

void setSigHandler()
{
 signal(SIGINT, sigintSigHandler);
 signal(SIGTERM, sigtermSigHandler);
}


void ignoreSIGINT(gboolean ign)
{
 /* Disable SIGINT for before use exec. */
 ignsigint=ign;
}
