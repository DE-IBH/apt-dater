/* $Id$
 *
 * Report generation.
 */

#include "apt-dater.h"
#include "ui.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

void initReport(GList *hosts) {
  printf(PACKAGE" is refreshing %d hosts, please standby...\n\n", g_list_length(hosts));

  GList *n = hosts;
  while(n) {
    ((HostNode *)n->data)->category = C_REFRESH_REQUIRED;
    
    n = g_list_next(n);
  }
}

static void reportPackage(gpointer data, gpointer user_data) {
  PkgNode *n = (PkgNode *)data;
  
  if(n->flag == 0)
    return;
    
  printf("\t");
  switch(n->flag) {
    case HOST_STATUS_PKGUPDATE:
	printf("u: ");
	break;
    case HOST_STATUS_PKGKEPTBACK:
	printf("h: ");
	break;
    case HOST_STATUS_PKGEXTRA:
	printf("x: ");
	break;
  }
  printf("%s (%s", n->package, n->version);
  
  if(n->data)
    printf(" -> %s", n->data);

  printf(")\n");
}

static void reportHost(gpointer data, gpointer user_data) {
  HostNode *n = (HostNode *)data;
  
  printf("Group   : %s\n", n->group);
  printf("Hostname: %s\n", n->hostname);
  printf("Status  : %s\n", drawCategories[n->category]);
  if(g_list_length(n->packages)) {
    printf("Packages: i=%d, u=%d, h=%d, x=%d\n", g_list_length(n->packages),
           n->nupdates, n->nholds, n->nextras);
    g_list_foreach(n->packages, reportPackage, NULL);
  }
  printf("\n");
}

gboolean ctrlReport(GList *hosts) {
  gint torefresh = 0;
  
  g_usleep(G_USEC_PER_SEC);
  
  GList *n = hosts;
  while(n) {
    if( (((HostNode *)n->data)->category == C_REFRESH_REQUIRED) ||
        (((HostNode *)n->data)->category == C_REFRESH) )
        torefresh++;
        
    n = g_list_next(n);
  }
  
  if(torefresh == 0) {
   g_list_foreach(hosts, reportHost, NULL);
   g_main_loop_quit (loop);
  }
  
  return torefresh > 0;
}
