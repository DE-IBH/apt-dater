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

#define STATS_MAX_LINE_LEN 1000
#define BUF_MAX_LEN 256
#define PROG_NAME PACKAGE
#define CFGFILENAME "apt-dater.conf"

typedef struct _cfgfile {
 gchar *hostsfile;
 gchar *screenrcfile;
 gchar *screentitle;
 gchar *statsdir;
 gchar *ssh_cmd;
 gchar *sftp_cmd;
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
 gchar *version;
 gint flag;
 gchar *data;
} PkgNode;

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
#ifdef HAVE_TCLLIB
 C_FILTERED =6,
#endif
 C_UNKNOW = 7,
} Category;

#define HOST_STATUS_PKGUPDATE        1
#define HOST_STATUS_PKGKEPTBACK      2
#define HOST_STATUS_PKGEXTRA         4
#define HOST_STATUS_KERNELNOTMATCH   8
#define HOST_STATUS_KERNELSELFBUILD 16
#define HOST_STATUS_LOCKED          32

typedef struct _hostnode {
 gchar     *hostname;
 gchar     *group;
 gchar     *ssh_user;
 gint      ssh_port;
 guint     status;
 gboolean  keptback;
 Category  category;
 GList     *packages;
 gint      nupdates;
 gint      nholds;
 gint      nextras;
 GList     *screens;
 gint      fdlock;
 gchar     *lsb_distributor;
 gchar     *lsb_release;
 gchar     *lsb_codename;
 gchar     *kernelrel;
} HostNode;

struct mapping_t
{
 gchar *name;
 gint value;
};

#include "extern.h"

#endif /* _APT_DATER_H */
