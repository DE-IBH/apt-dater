/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2014 (C) IBH IT-Service GmbH [https://www.ibh.de/apt-dater/]
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

#ifndef _SCREEN_H
#define _SCREEN_H

#include "config.h"

#ifndef FEAT_TMUX

#include "apt-dater.h"
#include "history.h"

#define SCREEN_SDFORMT "%s/S-%s"
#define SCREEN_SOCKPRE "apt-dater_"

gboolean
screen_get_sessions(HostNode *n);

gchar **screen_new(HostNode *n, const gboolean detached);
gboolean screen_attach(HostNode *n, const SessNode *s, const gboolean shared);
gchar *screen_get_dump(const SessNode *s);

static inline gboolean
screen_is_attached(const SessNode *s) {
  if (s->st.st_mode & S_IXUSR)
    return TRUE;

  return FALSE;
}
#endif /* !FEAT_TMUX */

#endif /* _SCREEN_H */
