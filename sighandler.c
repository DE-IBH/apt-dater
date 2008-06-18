/* $Id$
 *
 * Define the signal handler for apt-dater here.
 */

#include "apt-dater.h"
#include "ui.h"
#include "sighandler.h"

static int sigintcnt = 0;

static void sigintSigHandler()
{
 injectKey('q');

 if (sigintcnt++ > 1) {
  refreshUI();
  cleanUI();

  /* Force exit maybe if it hangs */
  exit(EXIT_FAILURE);
 }
}


void setSigHandler()
{
 signal(SIGINT, sigintSigHandler);
}
