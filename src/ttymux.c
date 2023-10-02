/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   2023 (C) Stefan Bühler <source@stbuehler.de>
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
#include "ttymux.h"

static void ttymux_free_session_list(GList *sessions) {
    while (sessions) {
        GList *next = g_list_next(sessions);
        g_free((SessNode*) sessions->data);
        g_list_free1(sessions);
        sessions = next;
    }
}

gboolean ttymux_update_sessions(HostNode *n) {
    ttymux_free_session_list(n->screens);

    n->screens = TTYMUX_GET_SESSIONS(n);

    return (n->screens != NULL);
}
