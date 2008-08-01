/* $Id$
 *
 * Report generation.
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

void initReport(GList *hosts) {
  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION
                            
  fprintf(stderr, PACKAGE" is refreshing %d hosts, please standby...\n\n", g_list_length(hosts));

  writer = xmlNewTextWriterDoc(&doc, 0);
  if (writer == NULL) {
    fprintf(stderr, "Error creating the xml output.\n");
    exit(1);
  }
  xmlTextWriterStartDocument(writer, NULL, NULL, NULL);
                           
  GList *n = hosts;
  while(n) {
    ((HostNode *)n->data)->category = C_REFRESH_REQUIRED;
    
    n = g_list_next(n);
  }
}

static void reportPackage(gpointer data, gpointer user_data) {
  PkgNode *n = (PkgNode *)data;
  
  xmlTextWriterStartElement(writer, "pkg");
  xmlTextWriterWriteAttribute(writer, "name", n->package);
  xmlTextWriterWriteAttribute(writer, "version", n->version);

  if(n->flag & HOST_STATUS_PKGUPDATE)
    xmlTextWriterWriteAttribute(writer, "hasupdate", "1");

  if(n->flag & HOST_STATUS_PKGKEPTBACK)
    xmlTextWriterWriteAttribute(writer, "onhold", "1");

  if(n->flag & HOST_STATUS_PKGEXTRA)
    xmlTextWriterWriteAttribute(writer, "extra", "1");

  if(n->data)
    xmlTextWriterWriteAttribute(writer, "data", n->data);

  xmlTextWriterEndElement(writer);
}

static void reportHost(gpointer data, gpointer lgroup) {
  HostNode *n = (HostNode *)data;
  
  if(*(char **)lgroup == NULL ||
     strcmp(*(char **)lgroup, n->group)) {
    if(*(char **)lgroup)
	xmlTextWriterEndElement(writer);

    xmlTextWriterStartElement(writer, "group");
    xmlTextWriterWriteAttribute(writer, "name", n->group);
    
    *(char **)lgroup = n->group;
  }

  /* Begin host element. */  
  xmlTextWriterStartElement(writer, "host");
  xmlTextWriterWriteAttribute(writer, "hostname", n->hostname);
#ifdef FEAT_TCLFILTER
  if(n->filtered)
    xmlTextWriterWriteAttribute(writer, "filtered", "1");
#endif

  /* Status */
  xmlTextWriterStartElement(writer, "status");
  xmlTextWriterWriteFormatAttribute(writer, "status", "%d", n->category);
  xmlTextWriterWriteString(writer, drawCategories[n->category]);
  xmlTextWriterEndElement(writer);

  /* SSH config */
  xmlTextWriterStartElement(writer, "ssh");
  xmlTextWriterWriteElement(writer, "user", n->ssh_user);
  xmlTextWriterWriteFormatElement(writer, "port", "%d", n->ssh_port);
  xmlTextWriterEndElement(writer);

  /* Kernel info */
  xmlTextWriterStartElement(writer, "kernel");
  if(n->status & HOST_STATUS_KERNELNOTMATCH)
    xmlTextWriterWriteAttribute(writer, "reboot", "1");
  if(n->status & HOST_STATUS_KERNELSELFBUILD)
    xmlTextWriterWriteAttribute(writer, "custom", "1");
  if(n->kernelrel)
    xmlTextWriterWriteString(writer, n->kernelrel);
  xmlTextWriterEndElement(writer);

  /* LSB info */
  xmlTextWriterStartElement(writer, "lsb");
  if(n->lsb_distributor)
    xmlTextWriterWriteElement(writer, "distri", n->lsb_distributor);
  if(n->lsb_release)
    xmlTextWriterWriteElement(writer, "release", n->lsb_release);
  if(n->lsb_codename)
    xmlTextWriterWriteElement(writer, "codename", n->lsb_codename);
  xmlTextWriterEndElement(writer);

  /* Packages */
  xmlTextWriterStartElement(writer, "packages");
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
    xmlTextWriterStartElement(writer, "report");
    xmlTextWriterWriteFormatElement(writer, "timestamp", "%d", time(NULL));

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
