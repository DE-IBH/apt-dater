/* $Id$
 *
 */

#include "screen.h"

const static gchar *
screen_get_sdir() {
  static gchar sdir[256];

  g_snprintf(sdir, sizeof(sdir), SCREEN_SDFORMT, SCREEN_SOCKDIR, g_get_user_name());
  
  return sdir;
}

gboolean
screen_get_sessions(HostNode *n) {
  if (n->screens) {
    g_list_free(n->screens);
    n->screens = NULL;
  }

  const gchar *sdir = screen_get_sdir();
  GDir *d = g_dir_open(sdir, 0, NULL);

  if (!d)
    return FALSE;

  gchar *search = g_strdup_printf(SCREEN_SOCKPRE"%s_%s_%d", n->ssh_user, n->hostname, n->ssh_port);

  gchar *f;
  while ((f = g_dir_read_name(d))) {
    gchar *fn = g_strdup_printf("%s/%s", sdir, f);
    
    if (g_file_test(fn, G_FILE_TEST_EXISTS)) {
      gint pid;
      gchar *name = g_strdup(f);

      sscanf(f, "%d.%s", &pid, name);
      
      if ((pid > 1) &&
	  (strcmp(name, search) == 0)) {

	struct stat buf;
	g_stat(fn, &buf);

	SessNode *s = g_new0(SessNode, 1);
	s->pid = pid;
	s->ts = buf.st_mtime;

	n->screens = g_list_prepend(n->screens, s);
      }

      g_free(name);
    }
  }
  g_dir_close(d);

  g_free(search);

  return g_list_length(n->screens) > 0;
}

gchar *
screen_new_cmd(const gchar *host, const gchar *user, const gint port) {
  if (!cfg->use_screen)
    return g_strdup("");
  
  return g_strdup_printf(SCREEN_BINARY"+-S+"SCREEN_SOCKPRE"%s_%s_%d" \
			 "+-t+%s@%s:%d+",
			 user, host, port,
			 user, host, port);
}

gchar *
screen_connect_cmd(const gchar *host, const gchar *user, const gint port) {
  if (!cfg->use_screen)
    return g_strdup("");
  
  return g_strdup_printf(SCREEN_BINARY"+-r+"SCREEN_SOCKPRE"%s_%s_%d+",
			 user, host, port);
}

gchar *
screen_force_connect_cmd(const gchar *host, const gchar *user, const gint port) {
  if (!cfg->use_screen)
    return g_strdup("");
  
  return g_strdup_printf(SCREEN_BINARY"+-rd+"SCREEN_SOCKPRE"%s_%s_%d+",
			 user, host, port);
}
