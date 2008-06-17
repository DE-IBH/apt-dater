
#ifndef _STATS_H
#define _STATS_H

#include <glib-2.0/glib.h>
#include "apt-dater.h"

Category
getUpdatesFromStat(gchar *hostname, GList *updates, guint *stat);

void
getOldestMtime(GList *hosts);

void
freeUpdates(GList *updates);

#endif /* _STATS_H */
