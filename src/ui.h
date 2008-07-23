
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

typedef enum {
 VK_ATTACH=1,
 VK_CONNECT=2,
 VK_DUMP=4,
 VK_UPGRADE=8,
 VK_INSTALL=16,
 VK_REFRESH=32,
 VK_KILL=64,
} EVisKeyMask;

typedef enum {
 SC_KEY_LEFT = 0,
 SC_KEY_RIGHT,
 SC_KEY_UP,
 SC_KEY_DOWN,
 SC_KEY_HOME,
 SC_KEY_END,
 SC_KEY_PPAGE,
 SC_KEY_NPAGE,
 SC_KEY_SPACE,
 SC_KEY_RETURN,
 SC_KEY_PLUS,
 SC_KEY_QUIT,
 SC_KEY_HELP,
 SC_KEY_FIND,
 SC_KEY_FILTER,
 SC_KEY_ATTACH,
 SC_KEY_KILLSESS,
 SC_KEY_CONNECT,
 SC_KEY_FILETRANS,
 SC_KEY_TOGGLEDUMPS,
 SC_KEY_REFRESH,
 SC_KEY_INSTALL,
 SC_KEY_UPGRADE,
 SC_KEY_NEXTSESS,
 SC_KEY_CYCLESESS,
 SC_MAX,
} EShortCuts;

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
