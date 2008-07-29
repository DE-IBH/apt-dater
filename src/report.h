#ifndef _REPORT_H
#define _REPORT_H

#include <glib-2.0/glib.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef FEAT_XMLREPORT
void initReport(GList *);
gboolean ctrlReport (GList *);
#endif

#endif /* _REPORT_H */
