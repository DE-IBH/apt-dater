
#ifndef _UI_H
#define _UI_H

#define MENU_TEXT "q:Quit"

typedef enum {
 CATEGORY,
 GROUP,
 HOST,
 UPDATE
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

gboolean ctrlKeyEnter(GList *hosts);
#endif /* _UI_H */
