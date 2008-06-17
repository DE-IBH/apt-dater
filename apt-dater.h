/* $Id: ui.c 17 2008-06-13 08:16:29Z ellguth $
 *
 */

#ifndef _APT_DATER_H
#define _APT_DATER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <glib-2.0/glib.h>

#define PATH_CONFIG "conf/apt-dater.conf"
#define STATS_MAX_LINE_LEN 1000
#define BUF_MAX_LEN 256

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
 gboolean use_screen;
 gboolean dump_screen;
 gboolean query_maintainer;
 gchar **colors;
} CfgFile;

typedef struct _update {
 gchar *package;
 gchar *oldver;
 gchar *newver;
 gchar *section;
 gchar *dist;
} UpdNode;

typedef struct _session {
  gint pid;
  struct stat st;
} SessNode;

typedef enum {
 C_UPDATES_PENDING = 0,
 C_UP_TO_DATE = 1,
 C_NO_STATS = 2,
 C_REFRESH_REQUIRED = 3,
 C_REFRESH = 4,
 C_SESSIONS = 5,
 C_UNKNOW = 6
} Category;

#define HOST_STATUS_PKGKEPTBACK     1
#define HOST_STATUS_KERNELNOTMATCH  2
#define HOST_STATUS_KERNELSELFBUILD 4
#define HOST_STATUS_LOCKED          8

typedef struct _hostnode {
 gchar     *hostname;
 gchar     *group;
 gchar     *ssh_user;
 gint      ssh_port;
 guint     status;
 gboolean  keptback;
 Category  category;
 GList     *updates;
 GList     *screens;
 gint      fdlock;
} HostNode;

struct mapping_t
{
 gchar *name;
 gint value;
};

#include "extern.h"

#endif /* _APT_DATER_H */
