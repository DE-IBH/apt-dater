
#ifndef _EXTERN_H
#define _EXTERN_H

extern CfgFile *cfg;
extern GMainLoop *loop;
extern gboolean rebuilddl;
extern time_t oldest_st_mtime;
extern int uicolors[];

/* Prototypes */
extern gboolean ctrlUI (GList *);
void cleanUI();
gboolean refreshStats(GList *);
gboolean setStatsFileFromIOC(GIOChannel *, GIOCondition, gpointer);
gchar *getStatsFileName(gchar *);
gboolean removeStatsFile(gchar *);
Category getUpdatesFromStat(gchar *, GList *, guint *);
void refreshStatsOfNode(gpointer);

#endif /* _EXTERN_H */
