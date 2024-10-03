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

#ifndef _COMPLETION_H
#define _COMPLETION_H

#include <glib.h>

/* Find entries in a GList that match a given prefix.
 *
 * Based on deprecated glib GCompletion
 */

typedef struct _completion {
  GList *entries;

  /* remember last prefix and the matched entries */
  gchar* cached_prefix;
  GList* cached_entries;
} Completion;

Completion *completion_init();

/* entries: list of `DrawNode*` */
void completion_set_entries(Completion *cmpl, GList *entries);

/* returns non-owned list; returned list gets invalidated with any other completion_* call on this instance */
GList *completion_search(Completion *cmpl, const gchar *prefix);

void completion_free(Completion *cmpl);

#endif /* _COMPLETION_H */
