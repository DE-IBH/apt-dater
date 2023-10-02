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
#include "completion.h"
#include "ui.h"

Completion *completion_init() {
  Completion *cmpl = NULL;

  cmpl = g_new0(Completion, 1);
  return cmpl;
}

static void completion_reset(Completion *cmpl) {
  g_list_free(cmpl->entries);
  cmpl->entries = NULL;

  g_free(cmpl->cached_prefix);
  cmpl->cached_prefix = NULL;

  g_list_free(cmpl->cached_entries);
  cmpl->cached_entries = NULL;
}

void completion_set_entries(Completion *cmpl, GList *entries) {
  completion_reset(cmpl);

  cmpl->entries = g_list_copy(entries);
}

static gboolean prefix_match(const gchar *entry_name, const gchar *prefix, gsize prefix_len) {
  return 0 == g_ascii_strncasecmp(entry_name, prefix, prefix_len);
}

static gboolean entry_match(GList *entry, const gchar *prefix, gsize prefix_len) {
  gchar *entry_name;

  entry_name = getStrFromDrawNode((DrawNode *) entry->data);
  return prefix_match(entry_name, prefix, prefix_len);
}

GList *completion_search(Completion *cmpl, const gchar *prefix) {
  GList *entry = NULL;
  gsize prefix_len;

  prefix_len = strlen(prefix);
  if (0 == prefix_len) {
    return cmpl->entries;
  }

  if (cmpl->cached_prefix) {
    gsize cached_len = strlen(cmpl->cached_prefix);

    if (cached_len <= prefix_len && prefix_match(prefix, cmpl->cached_prefix, cached_len)) {
      if (cached_len == prefix_len) {
        return cmpl->cached_entries;
      }

      for (entry = cmpl->cached_entries; entry; ) {
        GList *next = g_list_next(entry);
        if (!entry_match(entry, prefix, prefix_len)) {
          cmpl->cached_entries = g_list_delete_link(cmpl->cached_entries, entry);
        }
        entry = next;
      }

      goto search_done;
    }
  }

  /* no cache / cache prefix mismatch: */

  g_list_free(cmpl->cached_entries);
  cmpl->cached_entries = NULL;

  for (entry = cmpl->entries; entry; entry = g_list_next(entry)) {
    if (entry_match(entry, prefix, prefix_len)) {
      cmpl->cached_entries = g_list_prepend(cmpl->cached_entries, entry->data);
    }
  }

search_done:
  /* Modified cache; remember prefix (unless result is empty)*/

  if (cmpl->cached_prefix) {
    g_free(cmpl->cached_prefix);
    cmpl->cached_prefix = NULL;
  }

  if (cmpl->cached_entries) {
    cmpl->cached_prefix = g_strdup(prefix);
  }

  return cmpl->cached_entries;
}

void completion_free(Completion *cmpl) {
  if (!cmpl) return;

  completion_reset(cmpl);

  g_free(cmpl);
}
