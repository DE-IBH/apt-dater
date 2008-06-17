
#ifndef _EXEC_H
#define _EXEC_H

#include "ui.h"

gboolean
ssh_cmd_refresh(gchar *hostname, gchar *ssh_user, gint ssh_port, HostNode *n);;

gboolean
ssh_cmd_upgrade(gchar *hostname, gchar *ssh_user, gint ssh_port);

gboolean
ssh_cmd_install(gchar *hostname, gchar *ssh_user, gint ssh_port, gchar *package);

gboolean
ssh_connect(gchar *hostname, gchar *ssh_user, gint ssh_port);

#endif /* _EXEC_H */
