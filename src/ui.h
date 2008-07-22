
#ifndef _UI_H
#define _UI_H

#include <glib-2.0/glib.h>

typedef enum {
 CATEGORY,
 GROUP,
 HOST,
 UPDATE,
 SESSION,
} DrawType;

typedef struct _drawnode {
 void     *p;
 DrawType type;
 gboolean extended;
 gboolean selected;
 guint    scrpos;
 guint    elements;
 int      attrs;
} DrawNode;

gboolean
ctrlKeyEnter(GList *hosts);

void doUI (GList *hosts);
void refreshUI();
void refreshDraw();
gboolean ctrlUI (GList *);
void cleanUI();
void injectKey(int);

extern gchar maintainer[48];

#endif /* _UI_H */
