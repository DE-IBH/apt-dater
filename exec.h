
#ifndef _EXEC_H
#define _EXEC_H

#include "ui.h"

gboolean
ssh_cmd_refresh(HostNode *n);

gboolean
ssh_cmd_upgrade(HostNode *n, const gboolean detached);

gboolean
ssh_cmd_install(HostNode *n, const gchar *package, const gboolean detached);

gboolean
ssh_connect(HostNode *n, const gboolean detached);

#endif /* _EXEC_H */
