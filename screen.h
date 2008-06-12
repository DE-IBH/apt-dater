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

gchar *
screen_new_cmd(const gchar *host, const gchar *user, const gint port);

gchar *
screen_connect_cmd(const gchar *host, const gchar *user, const gint port);

gchar *
screen_force_connect_cmd(const gchar *host, const gchar *user, const gint port);

#endif /* _SCREEN_H */
