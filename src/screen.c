/* $Id$
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <glib-2.0/glib.h>
#include <glib-2.0/glib/gstdio.h>
#include "screen.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

static gchar *dump_fn = NULL;
static struct passwd *pw = NULL;

const static gchar *
screen_get_sdir() {
  static gchar sdir[256];

  if (!pw)
    pw = getpwuid(getuid());
  if (!pw)
    return NULL;

  g_snprintf(sdir, sizeof(sdir), SCREEN_SDFORMT, SCREEN_SOCKDIR, pw->pw_name);
  
  return sdir;
}

gboolean
screen_get_sessions(HostNode *n) {
  if (n->screens) {
    g_list_free(n->screens);
    n->screens = NULL;
  }

  const gchar *sdir = screen_get_sdir();
  if (!sdir)
    return FALSE;

  GDir *d = g_dir_open(sdir, 0, NULL);
  if (!d) {
   return FALSE;
  }

  gchar *search = g_strdup_printf(SCREEN_SOCKPRE"%s_%s_%d", n->ssh_user, n->hostname, n->ssh_port);

  const gchar *f;
  while ((f = g_dir_read_name(d))) {
    gchar *fn = g_strdup_printf("%s/%s", sdir, f);
    
    if (g_file_test(fn, G_FILE_TEST_EXISTS)) {
      gint pid = atoi(f);
      char *name = strchr(f, '.');

      if ((pid > 1) &&
	  (name) &&
	  (strcmp(name+1, search) == 0)) {

	SessNode *s = g_new0(SessNode, 1);
	s->pid = pid;
	stat(fn, &s->st);

	n->screens = g_list_prepend(n->screens, s);
      }
    }

    g_free(fn);
  }
  g_dir_close(d);

  g_free(search);

  return g_list_length(n->screens) > 0;
}

gchar *
screen_new(HostNode *n, const gboolean detached) {
  if (!cfg->use_screen)
    return g_strdup("");
  
  return g_strdup_printf(SCREEN_BINARY"+-%sS+"SCREEN_SOCKPRE"%s_%s_%d" \
			 "+-t+%s@%s:%d+-c+%s+",
			 detached ? "dm" : "",
			 n->ssh_user, n->hostname, n->ssh_port,
			 n->ssh_user, n->hostname, n->ssh_port,
			 cfg->screenrcfile);
}

static gchar *
screen_attach_cmd(const SessNode *s, const gboolean shared) {
  return g_strdup_printf(SCREEN_BINARY"+-r%s+%d+", shared ? "x" : "", s->pid);
}

gboolean
screen_attach(const SessNode *s, const gboolean shared) {
 gboolean r;
 GError *error = NULL;
 gchar *cmd = screen_attach_cmd(s, shared);
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

static gchar *
screen_dump_cmd(const SessNode *s, const gchar *fn) {
  return g_strdup_printf(SCREEN_BINARY"+-S+%d+-X+hardcopy+%s", s->pid, fn);
}

gchar *
screen_get_dump(const SessNode *s) {
 gboolean r;
 GError *error = NULL;
 gchar **argv = NULL;

 if(!dump_fn) {
   dump_fn = g_strdup_printf("/tmp/%d.dump", getpid());
 }

 g_unlink(dump_fn);

 gint fd = g_open(dump_fn, O_CREAT | O_EXCL | O_TRUNC |O_RDWR, S_IRUSR | S_IWUSR);

 if(fd == -1)
   return NULL;

 gchar *cmd = screen_dump_cmd(s, dump_fn);
 if(!cmd)
   return NULL;

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
 
 gchar *c = NULL;
 g_file_get_contents(dump_fn, &c, NULL, NULL);

 return c;
}
