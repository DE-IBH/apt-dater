/* $Id$
 *
 * Define the signal handler for apt-dater here.
 */

#include "apt-dater.h"
#include "ui.h"
#include "sighandler.h"
#include "lock.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

static volatile int sigintcnt = 0;

static void sigintSigHandler(int sig)
{
 switch(sig) {
  case SIGINT:
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

static void sigtermSigHandler() {
 cleanUI();
 refreshUI();
 g_main_loop_quit (loop);
}

void setSigHandler()
{
 signal(SIGINT, sigintSigHandler);
 signal(SIGTERM, sigtermSigHandler);
}
