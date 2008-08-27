/* apt-dater - terminal-based remote package update manager
 *
 * $Id$
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
 *
 * License:
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this package; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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
#ifdef FEAT_TCLFILTER
 gchar *filterexp;
 gchar *filterfile;
#endif
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
#ifdef FEAT_TCLFILTER
 C_FILTERED = 6,
 C_UNKNOW = 7,
#else
 C_UNKNOW = 6,
#endif
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
#ifdef FEAT_TCLFILTER
 gboolean  filtered;
#endif
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
