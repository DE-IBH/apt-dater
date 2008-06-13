/* $Id$
 *
 */

#ifndef _SCREEN_H
#define _SCREEN_H

#include "apt-dater.h"

#define SCREEN_BINARY  "/usr/bin/screen"
#define SCREEN_SDFORMT "%s/S-%s"
#define SCREEN_SOCKDIR "/var/run/screen"
#define SCREEN_SOCKPRE "apt-dater_"

gboolean
screen_get_sessions(HostNode *n);

gchar *
screen_new_cmd(const gchar *host, const gchar *user, const gint port);

gboolean
screen_connect(const SessNode *s, const gboolean force);

#endif /* _SCREEN_H */
