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
#include "ui.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>

typedef enum {
 COMPCMD_ALL,
 COMPCMD_CODENAME,
 COMPCMD_DISTRIBUTOR,
 COMPCMD_GROUP,
 COMPCMD_PACKAGE,
 COMPCMD_UPDATE,
 COMPCMD_HOSTNAME,
 COMPCMD_FLAG,
} COMPCMD;

struct ValidCompCmds {
 gchar c;
 gchar *name;
 COMPCMD cmd;
};

static struct ValidCompCmds compCmds[] = {
 {'A', "all",         COMPCMD_ALL},
 {'d', "distributor", COMPCMD_DISTRIBUTOR},
 {'c', "codename",    COMPCMD_CODENAME},
 {'g', "group",       COMPCMD_GROUP},
 {'h', "hostname",    COMPCMD_HOSTNAME},
 {'p', "package",     COMPCMD_PACKAGE},
 {'u', "update" ,     COMPCMD_UPDATE},
 {'f', "flag" ,       COMPCMD_FLAG},
 {  0,      NULL,     0},
};


static gboolean compStrWithPattern(const gchar *str, gchar *pattern, gsize s)
{
 gboolean    r = FALSE;

 if(!str || !pattern) return FALSE;
 
#if (GLIB_MAJOR_VERSION >= 2 && GLIB_MINOR_VERSION >= 14)
 GRegex *regex = g_regex_new (pattern, G_REGEX_CASELESS, 0, NULL);
 if(regex) {
  r = g_regex_match (regex, str, 0, NULL);
  g_regex_unref (regex);
 }
#else
 gsize    maxsize;
 gint     i;

 maxsize = s > strlen(pattern) ? strlen(pattern) : s;

 for(i=0; i<strlen(str)&&strlen(&str[i]) >= maxsize;i++)
  if(!g_ascii_strncasecmp (&str[i], pattern, maxsize)) {
   r = TRUE;
   break;
  }
#endif

 return r;
}


gboolean compHostWithPattern(HostNode *n, gchar *in, gsize s)
{
 gboolean    r = FALSE;
 gchar       *pattern = NULL;
 gint        i, j, compflags = 0;
 COMPCMD     compcmd =  COMPCMD_HOSTNAME;

 if(!in || !n || strlen(in) < 1) return FALSE;

 if(g_str_has_prefix(in, "~") == TRUE && strlen(in) >= 2) {
  /* Check if is a valid compare command identifier. */
  for(i=0; compCmds[i].name;i++) {
   if(compCmds[i].c == in[1]) {
    compcmd = compCmds[i].cmd;
    break;
   }
  }
  if(!compCmds[i].name) return FALSE; /* No valid compare command! */
  else pattern = g_strdup(&in[2]);
 }
 else pattern = g_strdup(in);

 if(!pattern) return FALSE;
 g_strchug(pattern);

 switch(compcmd) {
 case COMPCMD_UPDATE:
 case COMPCMD_PACKAGE: 
  {
   GList *p = g_list_first(n->packages);
   while(p) {
    PkgNode *pn = p->data;
    if((compcmd == COMPCMD_UPDATE ? pn->flag & HOST_STATUS_PKGUPDATE : TRUE) &&
       compStrWithPattern(pn->package, pattern, s) == TRUE) {
     r = TRUE;
     break;
    }
   
    p = g_list_next(p);
   }
  }
  break; /* case COMPCMD_UPDATE */
 case COMPCMD_ALL:
  r = TRUE;
  break;
 case COMPCMD_DISTRIBUTOR:
  r = n->lsb_distributor ? compStrWithPattern(n->lsb_distributor, pattern, s) : FALSE;
  break;
 case COMPCMD_CODENAME:
  r = n->lsb_codename ? compStrWithPattern(n->lsb_codename, pattern, s) : FALSE;
  break;
 case COMPCMD_GROUP:
  r = compStrWithPattern(n->group, pattern, s);
  break;
 case COMPCMD_FLAG:
  compflags=0;
  for(i = 0; i < strlen(pattern); i++) {
   j=0;
   while(hostFlags[j].code) {
    if(hostFlags[j].code[0] == pattern[i])
     compflags |= hostFlags[j].flag;
    j++;
   }
  }
  if(n->status & compflags) r = TRUE;
  break;
 case COMPCMD_HOSTNAME:
 default:
  r = compStrWithPattern(n->hostname, pattern, s);
 } /* switch */

 g_free(pattern);

 return r;
}
