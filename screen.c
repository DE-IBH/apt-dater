/* $Id$
 *
 */

#include <curses.h>
#include "apt-dater.h"
#include "screen.h"

const static gchar *screen_get_sdir() {
  static gchar sdir[256];

  g_snprintf(sdir, sizeof(sdir), SCREEN_SDFORMT, SCREEN_SOCKDIR, g_get_user_name());
  
  return sdir;
}

GHashTable *screen_get_screens() {
  GDir *d = g_dir_open(screen_get_sdir(), 0, NULL);

  if (!d)
    return;

  GHashTable *slist = g_hash_table_new(g_int_hash, g_int_equal);

  gchar *f;
  while ((f = g_dir_read_name(d))) {
    if (g_file_test(f, G_FILE_TEST_IS_REGULAR)) {
      gint key;
      gchar *val = strdup(f);

      scanf("%d.%s", &key, val);
      
      if ((key > 1) &&
	  (strncmp(val, SCREEN_SOCKPRE, strlen(SCREEN_SOCKPRE))))
	g_hash_table_insert(slist, &key, &val);
    }
  }

  g_dir_close(d);

  return slist;
}

gchar *screen_new_cmd(const gchar *name) {
  // screen -S $SCREEN_SOCKPRE$name -t $name
}

gchar *screen_connect_cmd(const gchar *name) {
    // screen -r $SCREEN_SOCKPRE$name
}

gchar *screen_force_connect_cmd(const gchar *name) {
    // screen -rd $SCREEN_SOCKPRE$name
}
