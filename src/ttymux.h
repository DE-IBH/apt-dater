/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2015 (C) IBH IT-Service GmbH [https://www.ibh.de/apt-dater/]
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

#ifndef _TTYMUX_H
#define _TTYMUX_H

#include "apt-dater.h"
#include "history.h"

#ifdef FEAT_TMUX

#include "tmux.h"

#define TTYMUX_INITIALIZE(n)         tmux_initialize((n))
#define TTYMUX_GET_SESSIONS(n)       tmux_get_sessions((n))
#define TTYMUX_NEW(n,detached)       tmux_new((n), (detached))
#define TTYMUX_ATTACH(n, s, shared)  tmux_attach((n), (s), (shared))
#define TTYMUX_GET_DUMP(s)           tmux_get_dump((s))
#define TTYMUX_IS_ATTACHED(s)        ((s)->attached)

#else

#include "screen.h"

#define TTYMUX_INITIALIZE(n)         
#define TTYMUX_GET_SESSIONS(n)       screen_get_sessions((n))
#define TTYMUX_NEW(n,detached)       screen_new((n), (detached))
#define TTYMUX_ATTACH(n, s, shared)  screen_attach((n), (s), (shared))
#define TTYMUX_GET_DUMP(s)           screen_get_dump((s))
#define TTYMUX_IS_ATTACHED(s)        ((s)->attached)

#endif

#endif /* _TTYMUX_H */
