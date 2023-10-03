/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2015 (C) IBH IT-Service GmbH [https://www.ibh.de/apt-dater/]
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

#include "apt-dater.h"
#include "ui.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef FEAT_XMLREPORT

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlsave.h>

static xmlTextWriterPtr writer;
static xmlDocPtr doc;

#ifdef FEAT_HISTORY
#include "history.h"
#endif

void initReport(GList *hosts) {
  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION

  writer = xmlNewTextWriterDoc(&doc, 0);
  if (writer == NULL)
    g_error(_("Error creating the xml output."));
  xmlTextWriterStartDocument(writer, NULL, NULL, NULL);
  xmlTextWriterWriteDTD(writer, BAD_CAST("report"), NULL, BAD_CAST(XML_SCHEMA_URI"/report.dtd"), NULL);

  if(!hosts) return;

  fprintf(stderr, _("apt-dater is refreshing %d hosts, please standby...\n"), g_list_length(hosts));

  GList *n = hosts;
  while(n) {
    ((HostNode *)n->data)->category = C_REFRESH_REQUIRED;

    n = g_list_next(n);
  }
}

#ifdef FEAT_HISTORY
static void reportHistory(gpointer data, gpointer user_data) {
  HistoryEntry *h = (HistoryEntry *)data;

  xmlTextWriterStartElement(writer, BAD_CAST(h->action));

  xmlTextWriterWriteFormatElement(writer, BAD_CAST("timestamp"), "%d", (int)h->ts);
  xmlTextWriterWriteFormatElement(writer, BAD_CAST("duration"), "%d", h->duration);

  xmlTextWriterStartElement(writer, BAD_CAST("path"));
  xmlTextWriterWriteString(writer, BAD_CAST(h->path));
  xmlTextWriterEndElement(writer);

  if(h->maintainer && strlen(h->maintainer)) {
    xmlTextWriterStartElement(writer, BAD_CAST("maintainer"));
    xmlTextWriterWriteString(writer, BAD_CAST(h->maintainer));
    xmlTextWriterEndElement(writer);
  }

  xmlTextWriterEndElement(writer);
}
#endif

#ifdef FEAT_CLUSTERS
static void reportCluster(gpointer data, gpointer user_data) {
  xmlTextWriterWriteFormatElement(writer, BAD_CAST("member-of"), "%s", (gchar *)data);
}
#endif

static void reportPackage(gpointer data, gpointer user_data) {
  PkgNode *n = (PkgNode *)data;
  
  xmlTextWriterStartElement(writer, BAD_CAST("pkg"));
  xmlTextWriterWriteAttribute(writer, BAD_CAST("name"), BAD_CAST(n->package));
  xmlTextWriterWriteAttribute(writer, BAD_CAST("version"), BAD_CAST(n->version));

  if(n->flag & HOST_STATUS_PKGUPDATE)
    xmlTextWriterWriteAttribute(writer, BAD_CAST("hasupdate"), BAD_CAST("1"));

  if(n->flag & HOST_STATUS_PKGKEPTBACK)
    xmlTextWriterWriteAttribute(writer, BAD_CAST("onhold"), BAD_CAST("1"));

  if(n->flag & HOST_STATUS_PKGEXTRA)
    xmlTextWriterWriteAttribute(writer, BAD_CAST("extra"), BAD_CAST("1"));

  if(n->flag & HOST_STATUS_PKGBROKEN)
    xmlTextWriterWriteAttribute(writer, BAD_CAST("broken"), BAD_CAST("1"));

  if(n->data)
    xmlTextWriterWriteAttribute(writer, BAD_CAST("data"), BAD_CAST(n->data));

  xmlTextWriterEndElement(writer);
}

static void reportHost(gpointer data, gpointer lgroup) {
  HostNode *n = (HostNode *)data;
  
  if(*(char **)lgroup == NULL ||
     strcmp(*(char **)lgroup, n->group)) {
    if(*(char **)lgroup)
	xmlTextWriterEndElement(writer);

    xmlTextWriterStartElement(writer, BAD_CAST("group"));
    xmlTextWriterWriteAttribute(writer, BAD_CAST("name"), BAD_CAST(n->group));
    
    *(char **)lgroup = n->group;
  }

  /* Begin host element. */  
  xmlTextWriterStartElement(writer, BAD_CAST("host"));
  xmlTextWriterWriteAttribute(writer, BAD_CAST("hostname"), BAD_CAST(n->hostname));
  if(n->comment)
    xmlTextWriterWriteAttribute(writer, BAD_CAST("comment"), BAD_CAST(n->comment));
#ifdef FEAT_TCLFILTER
  if(n->filtered)
    xmlTextWriterWriteAttribute(writer, BAD_CAST("filtered"), BAD_CAST("1"));
#endif

  /* Status */
  xmlTextWriterStartElement(writer, BAD_CAST("status"));
  xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("status"), "%d", n->category);
  xmlTextWriterWriteString(writer, BAD_CAST(drawCategories[n->category]));
  xmlTextWriterEndElement(writer);

  /* SSH config */
  xmlTextWriterStartElement(writer, BAD_CAST("ssh"));
  if(n->ssh_user)
    xmlTextWriterWriteElement(writer, BAD_CAST("user"), BAD_CAST(n->ssh_user));
  if(n->ssh_host)
    xmlTextWriterWriteFormatElement(writer, BAD_CAST("host"), "%s", n->ssh_host);
  if(n->ssh_port)
    xmlTextWriterWriteFormatElement(writer, BAD_CAST("port"), "%d", n->ssh_port);
  xmlTextWriterEndElement(writer);

  /* Kernel info */
  xmlTextWriterStartElement(writer, BAD_CAST("kernel"));
  if(n->status & (HOST_STATUS_KERNELABIUPGR | HOST_STATUS_KERNELVERUPGR))
    xmlTextWriterWriteAttribute(writer, BAD_CAST("reboot"), BAD_CAST("1"));
  if(n->kernelrel)
    xmlTextWriterWriteString(writer, BAD_CAST(n->kernelrel));
  xmlTextWriterEndElement(writer);

  /* LSB info */
  xmlTextWriterStartElement(writer, BAD_CAST("lsb"));
  if(n->lsb_distributor)
    xmlTextWriterWriteElement(writer, BAD_CAST("distri"), BAD_CAST(n->lsb_distributor));
  if(n->lsb_release)
    xmlTextWriterWriteElement(writer, BAD_CAST("release"), BAD_CAST(n->lsb_release));
  if(n->lsb_codename)
    xmlTextWriterWriteElement(writer, BAD_CAST("codename"), BAD_CAST(n->lsb_codename));
  xmlTextWriterEndElement(writer);

  /* UNAME info */
  xmlTextWriterStartElement(writer, BAD_CAST("uname"));
  if(n->uname_kernel)
    xmlTextWriterWriteElement(writer, BAD_CAST("kernel"), BAD_CAST(n->uname_kernel));
  if(n->uname_machine)
    xmlTextWriterWriteElement(writer, BAD_CAST("machine"), BAD_CAST(n->uname_machine));
  xmlTextWriterEndElement(writer);

  /* virtualization info */
  if(n->virt)
    xmlTextWriterWriteElement(writer, BAD_CAST("virt"), BAD_CAST(n->virt));
  else
    xmlTextWriterWriteElement(writer, BAD_CAST("virt"), BAD_CAST("Unknown"));

  /* host UUID */
  if(n->uuid[0])
    xmlTextWriterWriteElement(writer, BAD_CAST("uuid"), BAD_CAST(n->uuid));

#ifdef FEAT_HISTORY
  /* history data */
  GList *hel = history_get_entries(cfg, n);
  if(hel) {
    xmlTextWriterStartElement(writer, BAD_CAST("history"));
    g_list_foreach(hel, reportHistory, NULL);
    xmlTextWriterEndElement(writer);

    history_free_hel(hel);
  }
#endif

#ifdef FEAT_CLUSTERS
  /* clusters data */
  if(n->clusters) {
    xmlTextWriterStartElement(writer, BAD_CAST("clusters"));
    g_list_foreach(n->clusters, reportCluster, NULL);
    xmlTextWriterEndElement(writer);
  }
#endif

  /* Packages */
  xmlTextWriterStartElement(writer, BAD_CAST("packages"));
  g_list_foreach(n->packages, reportPackage, NULL);
  xmlTextWriterEndElement(writer);

  xmlTextWriterEndElement(writer);
}

gboolean ctrlReport(GList *hosts) {
  gint torefresh = 0;
  char *lgroup = NULL;

  g_usleep(G_USEC_PER_SEC);

  GList *n = hosts;
  while(n) {
    if( (((HostNode *)n->data)->category == C_REFRESH_REQUIRED) ||
        (((HostNode *)n->data)->category == C_REFRESH) )
        torefresh++;

    n = g_list_next(n);
  }

  if(torefresh == 0) {
    /* Create root node. */
    xmlTextWriterStartElement(writer, BAD_CAST("report"));
    xmlTextWriterWriteFormatElement(writer, BAD_CAST("timestamp"), "%d", (int)time(NULL));

    /* Put node stats to file. */
    g_list_foreach(hosts, reportHost, &lgroup);

    /* Close open host element. */
    if(lgroup)
      xmlTextWriterEndElement(writer);

    /* Close root node and finalize document. */
    xmlTextWriterEndElement(writer);
    xmlTextWriterEndDocument(writer);
    xmlFreeTextWriter(writer);

    xmlSaveCtxtPtr save = xmlSaveToFd(fileno(stdout), NULL, XML_SAVE_FORMAT);
    xmlSaveDoc(save, doc);
    xmlSaveClose(save);

    xmlFreeDoc(doc);

    g_main_loop_quit (loop);
    fprintf(stderr, "\ndone\n");
  }

  return torefresh > 0;
}
#endif
