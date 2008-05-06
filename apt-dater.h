
#ifndef _APT_DATER_H
#define _APT_DATER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <glib-2.0/glib.h>

#define PATH_CONFIG "conf/apt-dater.conf"
#define STATS_MAX_LINE_LEN 1000
#define BUF_MAX_LEN 255

typedef struct _cfgfile {
 gchar *hostsfile;
 gchar *statsdir;
 gchar *ssh_cmd;
 gchar *ssh_optflags;
 gchar *ssh_defuser;
 gint  ssh_defport;
 gchar *cmd_refresh;
 gchar *cmd_upgrade;
 gchar *cmd_install;
} CfgFile;

typedef struct _update {
 gchar *package;
 gchar *oldver;
 gchar *newver;
 gchar *section;
 gchar *dist;
} UpdNode;

typedef enum {
 C_UPDATES_PENDING = 0,
 C_UP_TO_DATE = 1,
 C_NO_STATS = 2,
 C_REFRESH_REQUIRED = 3,
 C_REFRESH = 4,
 C_UNKNOW = 5
} Category;

typedef struct _hostnode {
 gchar    *hostname;
 gchar    *group;
 gchar    *ssh_user;
 gint     ssh_port;
 gint     status;
 gboolean keptback;
 Category category;
 GList    *updates;
} HostNode;

#include "extern.h"

#endif /* _APT_DATER_H */
