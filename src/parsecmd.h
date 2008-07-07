
#ifndef _PARSECMD_H
#define _PARSECMD_H

#include "apt-dater.h"

gchar *parse_string(const gchar *src, const HostNode *n);
gboolean parse_cmdline(const char *s, int *argcPtr, char *** argvPtr, const HostNode *n);

#endif /* _PARSECMD_H */
