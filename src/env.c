/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2009-2012 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
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

#include "env.h"

GSList *base_env = NULL;

void
env_init(gchar **envp) {
    g_assert(base_env == NULL);

    int i;
    for(i=0; envp[i]; i++) {
	base_env = g_slist_prepend(base_env,  envp[i]);
    }

#define ADD_GENV(name, value) \
    base_env = g_slist_prepend(base_env, g_strdup_printf("AD_"name"=%s", value))

    ADD_GENV("HOSTSFILE"        , cfg->hostsfile);
    ADD_GENV("SCREENRCFILE"     , cfg->screenrcfile);
    ADD_GENV("STATSDIR"         , cfg->statsdir);
    ADD_GENV("SSH_CMD"          , cfg->ssh_cmd);
    ADD_GENV("SFTP_CMD"         , cfg->sftp_cmd);
    ADD_GENV("SSH_OPTFLAGS"     , cfg->ssh_optflags);
    ADD_GENV("CMD_REFRESH"      , cfg->cmd_refresh);
    ADD_GENV("CMD_UPGRADE"      , cfg->cmd_upgrade);
    ADD_GENV("CMD_INSTALL"      , cfg->cmd_install);
#ifdef FEAT_HISTORY
    ADD_GENV("HIST_RECORD"      , cfg->record_history ? "true" : "false");
    ADD_GENV("HIST_ERRPATTERN"  , cfg->history_errpattern);
#else
    ADD_GENV("HIST_RECORD"      , "false");
#endif
    ADD_GENV("HOOK_PRE_UPGRADE"  , cfg->hook_pre_upgrade);
    ADD_GENV("HOOK_PRE_REFRESH" , cfg->hook_pre_refresh);
    ADD_GENV("HOOK_PRE_INSTALL" , cfg->hook_pre_install);
    ADD_GENV("HOOK_PRE_CONNECT" , cfg->hook_pre_connect);
    ADD_GENV("HOOK_POST_UPGRADE" , cfg->hook_post_upgrade);
    ADD_GENV("HOOK_POST_REFRESH", cfg->hook_post_refresh);
    ADD_GENV("HOOK_POST_INSTALL", cfg->hook_post_install);
    ADD_GENV("HOOK_POST_CONNECT", cfg->hook_post_connect);

    ADD_GENV("PLUGINDIR", cfg->plugindir);
}

gchar **
env_build(HostNode *n, const gchar *action, const gchar *param, const HistoryEntry *he) {
    gchar **new_env = (gchar **) g_new0(gchar**, g_slist_length(base_env) + 21
#ifdef FEAT_CLUSTERS
    + 1 + g_list_length(n->clusters)
#endif
    );
    gint i = 0;

    GSList *p;
    for(p = base_env; p; p = p->next)
	new_env[i++] = g_strdup(p->data);

#define ADD_HENV(name, value) \
    new_env[i++] = g_strdup_printf("AD_"name"=%s", value)

    ADD_HENV("HOSTNAME"         , n->hostname);
    ADD_HENV("GROUP"            , n->group);
    if(n->ssh_user)
     ADD_HENV("SSH_USER"        , n->ssh_user);
    else
     ADD_HENV("SSH_USER"        , "");
    ADD_HENV("SSH_HOST"         , (n->ssh_host ? n->ssh_host : n->hostname));
    if(n->ssh_port)
     new_env[i++] = g_strdup_printf("AD_SSH_PORT=%d", n->ssh_port);
    else
     ADD_HENV("SSH_PORT"        , "");
    if(n->identity_file && strlen(n->identity_file) > 0)
     new_env[i++] = g_strdup_printf("AD_SSH_ID=-i %s", n->identity_file);
    else
     ADD_HENV("SSH_ID"          , "");
    ADD_HENV("STATSFILE"        , n->statsfile);
    ADD_HENV("KERNEL"           , n->kernelrel);
    ADD_HENV("LSB_DISTRI"       , n->lsb_distributor);
    ADD_HENV("LSB_RELEASE"      , n->lsb_release);
    ADD_HENV("LSB_CODENAME"     , n->lsb_codename);
    ADD_HENV("UNAME_KERNEL"     , n->uname_kernel);
    ADD_HENV("UNAME_MACHINE"    , n->uname_machine);
    ADD_HENV("VIRT"             , n->virt);
    ADD_HENV("TYPE"             , n->type);
    ADD_HENV("UUID"             , n->uuid);

#ifdef FEAT_HISTORY
    if(cfg->record_history && he) {
     gchar *hp = history_rec_path(n);
     gchar *fn_meta = g_strdup_printf("%s/meta", hp);

     ADD_HENV("HIST_PATH"       , hp);

     g_free(hp);

     history_write_meta(fn_meta, he);

     g_free(fn_meta);
    }
    else
#endif
     ADD_HENV("HIST_PATH"       , "");

#ifdef FEAT_CLUSTERS
    if(n->clusters != NULL) {
	new_env[i++] = g_strdup_printf("AD_CLUSTERS=%d", g_list_length(n->clusters));
	int j = 1;
	GList *c = n->clusters;
	while(c != NULL) {
	    new_env[i++] = g_strdup_printf("AD_CLUSTER%d=%s", j, (gchar *)c->data);
	    c = c->next;
	    j++;
	}
    }
    else
#endif
     ADD_HENV("CLUSTERS"        , "0");

    ADD_HENV("ACTION"           , action);
    if(param)
     ADD_HENV("PARAM"           , param);
    else
     ADD_HENV("PARAM"           , "");

    return new_env;
}
