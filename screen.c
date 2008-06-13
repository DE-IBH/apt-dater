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

  const gchar *f;
  while ((f = g_dir_read_name(d))) {
    gchar *fn = g_strdup_printf("%s/%s", sdir, f);
    
    if (g_file_test(fn, G_FILE_TEST_EXISTS)) {
      gint pid = atoi(f);
      gchar *name = strchr(f, '.');

      if ((pid > 1) &&
	  (name) &&
	  (strcmp(name+1, search) == 0)) {

	SessNode *s = g_new0(SessNode, 1);
	s->pid = pid;
	stat(fn, &s->st);

	n->screens = g_list_prepend(n->screens, s);
      }
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

static gchar *
screen_connect_cmd(const SessNode *s) {
  return g_strdup_printf(SCREEN_BINARY"+-rx+%d+", s->pid);
}

gboolean
screen_connect(const SessNode *s) {
 gboolean r;
 GError *error = NULL;
 gchar *cmd = screen_connect_cmd(s);
 gchar **argv = NULL;

 if(!cmd) return FALSE;

 argv = g_strsplit(cmd, "+", 0);

 r = g_spawn_sync(g_getenv ("HOME"), argv, NULL, 
		  G_SPAWN_CHILD_INHERITS_STDIN, NULL, NULL,
		  NULL, NULL, NULL, &error);

 if(r == FALSE) {
  g_warning("%s", error->message);
  g_clear_error (&error);
 }

 g_free(cmd);
 g_strfreev(argv);
 
 return r;
}
