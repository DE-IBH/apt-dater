/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2012 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
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

#include <glib.h>
#include <popt.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "apt-dater.h"
#include "parsecmd.h"
#include "ui.h"

gchar *parse_string(const gchar *src, const HostNode *n) {
  if(!n)
    return g_strdup(src);
  
  gint i = 0;
  GString *h = g_string_sized_new(strlen(src));

  while(src[i]) {
    if((src[i] != '%') ||
       (src[i+1] == 0)) {
      g_string_append_c(h, src[i++]);
      continue;
    }

    i++;

    switch(src[i]) {
    case 'h':
      g_string_append(h, n->hostname);
      break;
    case 'H':
      g_string_append(h, n->hostname);
      if(n->ssh_port) {
        g_string_append(h, ":");
        g_string_append_printf(h, "%d", n->ssh_port);
      }
      break;
    case 'u':
      g_string_append(h, n->ssh_user);
      break;
    case 'U':
      if(n->ssh_user) {
        g_string_append(h, n->ssh_user);
        g_string_append(h, "@");
      }
      break;
    case 'p':
      g_string_append_printf(h, "%d", n->ssh_port);
      break;
    case 'm':
      g_string_append(h, maintainer);
      break;
    default:
      g_string_append_c(h, src[i]);
      continue;
    }

    i++;
  }

  return g_string_free(h, FALSE);
}

gboolean parse_cmdline(const char *s, int *argcPtr, char ***argvPtr, const HostNode *n) {
  gint i;

  const gchar **argv;

  if(poptParseArgvString(s, argcPtr, &argv) < 0)
    return FALSE;

  *argvPtr = g_new0(char*, *argcPtr+1);

  if(n)
    for(i=0;i<*argcPtr;i++)
      (*argvPtr)[i] = parse_string(argv[i], n);

  return TRUE;
}

