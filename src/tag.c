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
#include "tag.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>

gboolean compHostWithPattern(HostNode *n, const gchar *pattern, gsize s)
{
 gboolean r = FALSE;

 if(!pattern || !n || strlen(pattern) < 1) return FALSE;

#if (GLIB_MAJOR_VERSION >= 2 && GLIB_MINOR_VERSION >= 14)
 GRegex *regex = g_regex_new (pattern, G_REGEX_CASELESS, 0, NULL);
 if(regex) {
  r = g_regex_match (regex, n->hostname, 0, NULL);
  g_regex_unref (regex);
 }
#else
 gint     i;
 gsize    maxsize;

 maxsize = s > strlen(pattern) ? strlen(pattern) : s;

 for(i=0; i<strlen(n->hostname)&&strlen(&n->hostname[i]) >= maxsize;i++)
  if(!g_ascii_strncasecmp (&n->hostname[i], pattern, 
			   maxsize)) {
   r = TRUE;
   break;
  }
#endif

 return r;
}
