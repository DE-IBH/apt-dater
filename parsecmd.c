/* $Id$
 *
 */

#include "apt-dater.h"
#include "parsecmd.h"
#include <glib.h>
#include <popt.h>

static gchar *parse_arg(const gchar *src, const HostNode *n) {
  if(!n)
    return g_strdup(src);
  
  gint i = 0;
  GString *h = g_string_sized_new(strlen(src));

  while(src[i]) {
    if((src[i] != '%') ||
       (src[i+1] == 0)) {
      g_string_append_c(h, src[i++]);
      continue;
    }

    i++;

    switch(src[i]) {
    case 'h':
      g_string_append(h, n->hostname);
      break;
    case 'u':
      g_string_append(h, n->ssh_user);
      break;
    case 'p':
      g_string_append_printf(h, "%d", n->ssh_port);
      break;
    default:
      g_string_append_c(h, src[i]);
      continue;
    }

    i++;
  }

  return g_string_free(h, FALSE);
}

gboolean parse_cmdline(const char *s, int *argcPtr, char ***argvPtr, const HostNode *n) {
  gint i;

  const gchar **argv;

  if(poptParseArgvString(s, argcPtr, &argv) < 0)
    return FALSE;

  *argvPtr = g_new0(char*, *argcPtr+1);

  if(n)
    for(i=0;i<*argcPtr;i++)
      (*argvPtr)[i] = parse_arg(argv[i], n);

  return TRUE;
}

