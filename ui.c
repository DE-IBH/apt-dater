/* $Id$
 *
 */

#include <curses.h>
#include "apt-dater.h"
#include "ui.h"


GList *drawlist = NULL;
char *drawCategories[] = {"Updates pending", "Up to date", "Status file missing", "Refresh required", "In refresh", "Sessions", "Unknown", 0};
gchar *incategory = NULL;
gchar *ingroup = NULL;
HostNode *inhost = NULL;

void freeDrawNode (DrawNode *n)
{
 g_free(n);
}

DrawNode *getSelectedDrawNode()
{
 GList *dl;
 DrawNode *n = NULL;

 dl = g_list_first(drawlist);
 while (dl && (((DrawNode *) dl->data)->selected != TRUE)) dl = g_list_next(dl);

 if (dl) return ((DrawNode *) dl->data);
 else return NULL;
}

guint getCategoryNumber(gchar *category)
{
 guint i = 0;

 while(*(drawCategories+i)) {
  if(*(drawCategories+i) == category) break;
  i++;
 }
 return(i);
}

void setHostsCategory(GList *hosts, Category matchCategory, gchar *matchGroup, Category setCategory)
{
 GList *ho;

 ho = g_list_first(hosts);

 while(ho) {
  if(matchGroup) {
   if((((HostNode *) ho->data)->category == matchCategory) && (!g_strcasecmp(((HostNode *) ho->data)->group, matchGroup))) ((HostNode *) ho->data)->category = setCategory;
  }
  else if((((HostNode *) ho->data)->category == matchCategory)) ((HostNode *) ho->data)->category = setCategory;
  ho = g_list_next(ho);
 }
}

guint getHostGrpCatCnt(GList *hosts, gchar *group, Category category)
{
 guint cnt = 0;
 GList *ho;

 ho = g_list_first(hosts);

 while(ho) {
  if((((HostNode *) ho->data)->category == category) && (!g_strcasecmp(((HostNode *) ho->data)->group, group))) cnt++;
  ho = g_list_next(ho);
 }
 return(cnt);
}

void disableInput() {
 noecho();
 curs_set(0);
 leaveok(stdscr, TRUE);
 nodelay(stdscr, TRUE);
}

void enableInput() {
 echo();
 curs_set(1);
 leaveok(stdscr, FALSE);
 nodelay(stdscr, FALSE);
}

void initUI ()
{
 initscr();
 start_color();
 use_default_colors();
 cbreak();
 nonl();
 disableInput();
 intrflush(stdscr, FALSE);
 keypad(stdscr, TRUE);
 clear();
 refresh();
}

void cleanUI ()
{
 endwin();
}

void drawMenu (char *str)
{
 int cy, cx;

 attron(A_REVERSE);
 move(0,0);
 hline(' ',COLS);
 addnstr(str, COLS);
 attroff(A_REVERSE);
}

void drawStatus (char *str)
{
 char strmtime[30];
 struct tm *tm_mtime;
 int mtime_pos;

 tm_mtime = localtime(&oldest_st_mtime);
 strftime(strmtime, sizeof(strmtime), " Oldest: %D %H:%M", tm_mtime);

 attron(A_REVERSE);
 move(LINES - 2, 0);
 hline( ' ', COLS);
 if(str) {
  addnstr(str, COLS);
 }

 mtime_pos = COLS - strlen(strmtime) - 1;

 move(LINES - 2, mtime_pos);
 addnstr(strmtime, COLS-mtime_pos);

 attroff(A_REVERSE);
}

void queryString(const gchar *query, gchar *in, const gint size)
{
 gint i;

 enableInput();
 mvaddstr(LINES - 1, 0, query);
 move(LINES - 1, strlen(query));

 for(i = strlen(in)-1; i>=0; i--)
  ungetch(in[i]);

 getnstr(in, size);
 disableInput();

 move(LINES - 1, 0);
 hline(' ', COLS);
}

void drawCategoryEntry (DrawNode *n)
{
 char statusln[BUF_MAX_LEN];
 char menuln[BUF_MAX_LEN];

 attron(n->attrs); 
 mvhline(n->scrpos, 0, ' ', COLS);

 mvaddstr(n->scrpos, 0, " [");
 if(n->elements > 0)
  addch(n->elements > 0 && n->extended == FALSE ? '+' : '-');
 else
  addch(' ');
 addstr("] ");
 addnstr((char *) n->p, COLS);
 attroff(n->attrs);

 if(n->selected == TRUE) {
  sprintf(statusln, "%d %s in status \"%s\"", n->elements, n->elements > 1 || n->elements == 0 ? "Hosts" : "Host", (char *) n->p);
  sprintf(menuln, "%s  g:Check for new updates", MENU_TEXT);

  drawMenu(menuln);
  drawStatus(statusln);
 }
}

void drawGroupEntry (DrawNode *n)
{
 char statusln[BUF_MAX_LEN];
 char menuln[BUF_MAX_LEN];

 attron(n->attrs);
 mvhline(n->scrpos, 0, ' ', COLS); 
 mvaddstr(n->scrpos, 2, " [");
 addch(n->elements > 0 && n->extended == FALSE ? '+' : '-');
 addstr("] ");
 addnstr((char *) n->p, COLS);
 attroff(n->attrs);

 if(n->selected == TRUE) {
  sprintf(statusln, "%d %s is in status \"%s\"", n->elements, n->elements > 1 || n->elements == 0 ? "Hosts" : "Host", incategory);
  sprintf(menuln, "%s  g:Check for new updates", MENU_TEXT);
  drawMenu(menuln);
  drawStatus(statusln);
 }
}

void drawHostEntry (DrawNode *n)
{
 char statusln[BUF_MAX_LEN];
 char menuln[BUF_MAX_LEN];

 attron(n->attrs);
 mvhline(n->scrpos, 0, ' ', COLS);
 if (((HostNode *) n->p)->status != 0) {
  attron(A_BOLD);
  mvaddstr(n->scrpos, 1, "!");
  attroff(A_BOLD);
 }
 mvaddstr(n->scrpos, 4, " [");

 if(n->elements > 0)
  addch(n->elements > 0 && n->extended == FALSE ? '+' : '-');
 else
  addch(' ');
 addstr("] ");
 addnstr((char *) ((HostNode *) n->p)->hostname, COLS);
 attroff(n->attrs);

 if(n->selected == TRUE) {
  switch(((HostNode *) n->p)->category) {
  case C_UPDATES_PENDING:
   sprintf(menuln, "%s  c:ssh-Connect  u:Upgrade host  g:Check for new updates  i:Install Package",MENU_TEXT);
   sprintf(statusln, "%d %s required", n->elements, n->elements > 1 || n->elements == 0 ? "Updates" : "Update");
   break;
  case C_UP_TO_DATE:
   sprintf(menuln, "%s  c:ssh-Connect  g:Check for new updates  i:Install Package" , MENU_TEXT);
   sprintf(statusln, "No update required");
   break;
  case C_NO_STATS:
   sprintf(menuln, "%s  c:ssh-Connect  g:Check for new updates  i:Install Package", MENU_TEXT);
   sprintf(statusln, "Statusfile is missing");
   break;
  case C_REFRESH_REQUIRED:
   sprintf(menuln, "%s  c:ssh-Connect  g:Check for new updates  i:Install Package", MENU_TEXT);
   sprintf(statusln, "Refresh required");
   break;
  case C_REFRESH:
   sprintf(menuln, "%s", MENU_TEXT);
   sprintf(statusln, "In refresh");
   break;
  case C_SESSIONS:
   sprintf(menuln, "%s  r:reconnect  R:force reconnect", MENU_TEXT);
   sprintf(statusln, "Session running");
   break;
  default:
   sprintf(menuln, "%s  c:ssh-Connect  g:Check for new updates  i:Install Package", MENU_TEXT);
   sprintf(statusln, "Status is unknown");
   break;
  }

  if (((HostNode *) n->p)->status & HOST_STATUS_PKGKEPTBACK) 
   strcat(statusln," - packages kept back");
  if (((HostNode *) n->p)->status & HOST_STATUS_KERNELNOTMATCH) 
   strcat(statusln," - running Kernel is not the latest");
  if (((HostNode *) n->p)->status & HOST_STATUS_KERNELSELFBUILD) 
   strcat(statusln," - a selfbuilt kernel is running");
  drawMenu(menuln);
  drawStatus(statusln);
 }
}

void drawUpdateEntry (DrawNode *n)
{
 char statusln[BUF_MAX_LEN];
 char menuln[BUF_MAX_LEN];

 attron(n->attrs);
 mvhline(n->scrpos, 0, ' ', COLS);
 mvaddnstr(n->scrpos, 7, (char *) ((UpdNode *) n->p)->package, COLS);
 attroff(n->attrs);
 if(n->selected == TRUE) {
  sprintf(menuln, "%s  i:Install package %s", MENU_TEXT, ((UpdNode *) n->p)->package);
  sprintf(statusln, "%s -> %s (%s %s)", ((UpdNode *) n->p)->oldver, ((UpdNode *) n->p)->newver, ((UpdNode *) n->p)->dist, ((UpdNode *) n->p)->section);
  drawMenu(menuln);
  drawStatus(statusln);
 }
}

void drawSessionEntry (DrawNode *n)
{
 char statusln[BUF_MAX_LEN];
 char menuln[BUF_MAX_LEN];
 char h[BUF_MAX_LEN];
 struct tm *tm_mtime;

 snprintf(h, sizeof(h), "[%d] since ", ((SessNode *) n->p)->pid);

 tm_mtime = localtime(&((SessNode *) n->p)->ts);
 strftime(&h[strlen(h)], sizeof(h)-strlen(h), "%D %H:%M", tm_mtime);

 attron(n->attrs);
 mvhline(n->scrpos, 0, ' ', COLS);
 mvaddnstr(n->scrpos, 7, h, COLS);
 attroff(n->attrs);
 if(n->selected == TRUE) {
  sprintf(menuln, "%s  r:reconnect  R:force reconnect", MENU_TEXT);
  //  sprintf(statusln, "%s -> %s (%s %s)", ((UpdNode *) n->p)->oldver, ((UpdNode *) n->p)->newver, ((UpdNode *) n->p)->dist, ((UpdNode *) n->p)->section);
  drawMenu(menuln);
  //  drawStatus(statusln);
 }
}

void detectPos()
{
 GList *dl;

 dl = g_list_first(drawlist);

 while(dl) {
  switch(((DrawNode *) dl->data)->type) {
  case CATEGORY:
   incategory = ((DrawNode *) dl->data)->p;
   break;
  case GROUP:
   ingroup = ((DrawNode *) dl->data)->p;
   break;
  case HOST:
   inhost = ((DrawNode *) dl->data)->p;
   break;
  default:
   break;
  }
  if(((DrawNode *) dl->data)->selected) return;
  dl = g_list_next(dl);
 }
}

void drawEntry (DrawNode *n)
{
 if(n->scrpos == 0 || n->scrpos >= LINES-2) return;

 switch(n->type) {
 case CATEGORY:
  drawCategoryEntry(n);
  break;
 case GROUP:
  drawGroupEntry(n);
  break;
 case HOST:
  drawHostEntry(n);
  break;
 case UPDATE:
  drawUpdateEntry(n);
  break;
 case SESSION:
  drawSessionEntry(n);
  break;
 default:
  break;
 }
}

void refreshDraw()
{
 clear();
 detectPos();
 g_list_foreach(drawlist, (GFunc) drawEntry, NULL);
 refresh();
}

void setEntryActiveStatus(DrawNode *n, gboolean active)
{
 if(!n) return;

 if(active == TRUE) {
  n->selected = TRUE;
  n->attrs|=A_REVERSE;
 }
 else {
  n->selected = FALSE;
  if(n->attrs & A_REVERSE)
   n->attrs^=A_REVERSE;
 }
}

guint getHostCatCnt(GList *hosts, Category category)
{
 guint cnt = 0;
 GList *ho;

 ho = g_list_first(hosts);
 while(ho) {
  if(((HostNode *) ho->data)->category == category) cnt++;
  ho = g_list_next(ho);
 }
 return(cnt);
}

void checkSelected()
{
 DrawNode *dn;
 GList *dl;

 dn = getSelectedDrawNode();
 if (!dn) {
  dl = g_list_first(drawlist);
  while (dl && (((DrawNode *) dl->data)->scrpos != LINES-2)) 
   dl = g_list_next(dl);
  if (!dl) dl = g_list_last(drawlist);
  setEntryActiveStatus((DrawNode *) dl->data, TRUE);
  return;
 }
 if (dn -> scrpos == 0 || dn -> scrpos > LINES-2) {
  setEntryActiveStatus(dn, FALSE);
  dl = g_list_first(drawlist);
  while (dl && (((DrawNode *) dl->data)->scrpos != LINES-2)) 
   dl = g_list_next(dl);
  if (!dl) dl = g_list_last(drawlist);
  setEntryActiveStatus((DrawNode *) dl->data, TRUE);
  return;
 }
}

void reorderScrpos(guint startat)
{
 guint count = 0;
 GList *dl;

 dl = g_list_first(drawlist);

 while(dl) {
  if (((DrawNode *) dl->data)->scrpos == startat)
   while (++count <= LINES-2) {
    if(!dl) return;
    ((DrawNode *) dl->data)->scrpos = count;
    dl = g_list_next(dl);
   }
  if(dl) ((DrawNode *) dl->data )->scrpos = 0;
  dl = g_list_next(dl);
 }
 checkSelected();
}

void buildIntialDrawList(GList *hosts)
{
 guint i = 0;
 DrawNode *drawnode;

 while(*(drawCategories+i)) {
  drawnode = g_new0(DrawNode, 1);
  drawnode->p = *(drawCategories+i);
  drawnode->type = CATEGORY;
  drawnode->extended = FALSE;
  drawnode->selected = i == 0 ? TRUE : FALSE;
  drawnode->scrpos = i == 0 ? 1 : 0;
  drawnode->elements = getHostCatCnt(hosts, i);
  if((i == (Category) C_UPDATES_PENDING || 
      i == (Category) C_NO_STATS ||
      i == (Category) C_SESSIONS ) && drawnode->elements > 0)
   drawnode->attrs = A_BOLD;
  else
   drawnode->attrs = A_NORMAL;
  if(drawnode->selected == TRUE)
   setEntryActiveStatus(drawnode, TRUE);
  drawlist = g_list_append(drawlist, drawnode);
  i++;
 }
}

void doUI (GList *hosts)
{
 initUI();

 /* Draw the host entries intial. */
 buildIntialDrawList(hosts);
 reorderScrpos(1);
 refreshDraw();
}

gboolean ctrlKeyUpDown(int ic)
{
 gboolean ret = FALSE;
 GList *dl;
 guint i = 0;

 dl = g_list_first(drawlist);
 while (dl && (((DrawNode *) dl->data)->selected != TRUE)) dl = g_list_next(dl);

 switch (ic) {
 case KEY_UP:
  if(!g_list_previous(dl)) return(ret);
  else {
   setEntryActiveStatus((DrawNode *) dl->data, FALSE);
   dl = g_list_previous(dl);
   setEntryActiveStatus((DrawNode *) dl->data, TRUE);
   if(((DrawNode *) dl->data)->scrpos == 0) {
    ((DrawNode *) dl->data)->scrpos = 1;
    reorderScrpos(1);
   }
   ret = TRUE;
  }
  break;
 case KEY_DOWN:
  if(!g_list_next(dl)) return(ret);
  else {
   setEntryActiveStatus((DrawNode *) dl->data, FALSE);
   dl = g_list_next(dl);
   setEntryActiveStatus((DrawNode *) dl->data, TRUE);
   if(((DrawNode *) dl->data)->scrpos >= LINES-2 || 
      ((DrawNode *) dl->data)->scrpos == 0) reorderScrpos(2);
   ret = TRUE;
  }
  break;
 case KEY_HOME:
  setEntryActiveStatus((DrawNode *) dl->data, FALSE);
  dl = g_list_first(drawlist);
  setEntryActiveStatus((DrawNode *) dl->data, TRUE);
  ((DrawNode *) dl->data)->scrpos = 1;
  reorderScrpos(1);
  ret = TRUE;
  break;
 case KEY_END :
  setEntryActiveStatus((DrawNode *) dl->data, FALSE);
  dl = g_list_last(drawlist);
  setEntryActiveStatus((DrawNode *) dl->data, TRUE);
  if(((DrawNode *) dl->data)->scrpos == 0 || 
     ((DrawNode *) dl->data)->scrpos > LINES-2) {
   i = 0;
   while (dl) {
    ((DrawNode *) dl->data)->scrpos = LINES-3-i;
    i++;
    i = i > LINES-3 ? LINES-3 : i;
    dl = g_list_previous(dl);
   }
  }
  ret = TRUE;
  break;
 default:
  break;
 }
 return (ret);
}

gboolean compDrawNodes(DrawNode* n1, DrawNode* n2)
{
 if (n1->type != n2->type) return(FALSE);
 switch(n1->type) {
 case CATEGORY:
 case GROUP:
  if(!g_strcasecmp(n1->p, n2->p))
   return(TRUE);
  break;
 case HOST:
  if (!g_strcasecmp(((HostNode *) (n1->p))->hostname, 
		    ((HostNode *)(n2->p))->hostname))
   return(TRUE);
  break;
 case UPDATE:
  if (!g_strcasecmp(((UpdNode *) n1->p)->package,
		    ((UpdNode *) n2->p)->package)) return(TRUE);
  break;
 case SESSION:
  if (((SessNode *) n1->p)->pid ==
      ((SessNode *) n2->p)->pid) return(TRUE);
  break;
 default:
  break;
 }
 return(FALSE);
}

void rebuildDrawList(GList *hosts)
{
 GList *dl_new = NULL;
 GList *dl_old = NULL;
 GList *old_drawlist = NULL; 
 DrawNode *n_old = NULL;
 DrawNode *n_new = NULL;
 guint i=0, j=0, k=0, savesel=0;
 gboolean posset = FALSE, selset = FALSE;

 rebuilddl = FALSE;

 old_drawlist = drawlist;
 drawlist = NULL;
 buildIntialDrawList(hosts);

 dl_old = g_list_nth(old_drawlist,i);
 dl_new = g_list_nth(drawlist,j);

 while(dl_old && dl_new) {
  n_old = (DrawNode *) dl_old->data;
  n_new = (DrawNode *) dl_new->data;

  if (!n_old || !n_new) break;

  setEntryActiveStatus(n_new, FALSE);
  if (j==0) n_new->scrpos=0;

  if(compDrawNodes(n_new,n_old)) {
   if(!posset && n_old->scrpos == 1) {
    n_new->scrpos=1;
    posset=TRUE;
   }
   if(!selset && n_old->selected) {
    savesel = j;
    selset = TRUE;
   }
   if(n_old->extended == TRUE ) {
    setEntryActiveStatus(n_new, TRUE);
    detectPos();
    ctrlKeyEnter(hosts);
    dl_new = g_list_nth(drawlist,j);
    n_new = (DrawNode *) dl_new->data;
    setEntryActiveStatus(n_new, FALSE);
   }
   i++;
   j++;
   dl_old = g_list_nth(old_drawlist,i);
   dl_new = g_list_nth(drawlist,j);
  }
  else {
   k=0;
   while (n_new->type >= n_old->type) {
    k++;
    dl_new = g_list_nth(drawlist,j+k);
    if (!dl_new) break;
    n_new = (DrawNode *) dl_new->data;
    if(n_new && compDrawNodes(n_old, n_new)) {
     if(!posset && n_old->scrpos==1) {
      n_new->scrpos=1;
      posset = TRUE;
     }
     if(!selset && n_old->selected) {
      savesel = j+k;
      selset = TRUE;
     }
     if(n_old->extended == TRUE ) {
      setEntryActiveStatus(n_new, TRUE);
      detectPos();
      ctrlKeyEnter(hosts);
      dl_new = g_list_nth(drawlist,j+k);
      n_new = (DrawNode *) dl_new->data;
      setEntryActiveStatus(n_new, FALSE);
     }
     j+= k + 1;
     break;
    }
   }
   if (!posset && n_old->scrpos == 1) {
    dl_new = g_list_nth(drawlist,j);
    if (dl_new) {
     n_new = (DrawNode *) dl_new->data;
     if (n_new) { n_new->scrpos = 1;
      posset = TRUE;
     }
    }
   }
   if (!selset && n_old->selected) {
    savesel = j;
    selset = TRUE;
   }
   i++;
   dl_old = g_list_nth(old_drawlist,i);
   dl_new = g_list_nth(drawlist,j);
  }
 }

 dl_new = g_list_nth(drawlist,savesel);
 if (dl_new) {
  n_new = (DrawNode *) dl_new->data;
  if (n_new) setEntryActiveStatus(n_new, TRUE);
 }

 reorderScrpos(1);

 if(drawlist) {
  g_list_foreach(old_drawlist, (GFunc) freeDrawNode, NULL);
  g_list_free(old_drawlist);
 }
 else drawlist = old_drawlist;
}

void extDrawListCategory(gint atpos, gchar *category, GList *hosts)
{
 guint i = 0;
 GList *ho;
 DrawNode *drawnode = NULL;

 while(*(drawCategories+i)) {
  if(category == *(drawCategories+i)) break;
  i++;
 }

 if(!(*(drawCategories+i))) return;

 ho = g_list_first(hosts);

 while(ho) {
  if(((HostNode *) ho->data)->category == i) {
   if((drawnode) && (!g_strcasecmp(drawnode->p, 
				   ((HostNode *) ho->data)->group))) {
    ho = g_list_next(ho);
    continue;
   }
   drawnode = g_new0(DrawNode, 1);
   drawnode->p = (((HostNode *) ho->data)->group);
   drawnode->type = GROUP;
   drawnode->extended = FALSE;
   drawnode->selected = FALSE;
   drawnode->scrpos = 0;
   drawnode->elements = getHostGrpCatCnt(hosts, 
					 ((HostNode *) ho->data)->group, i);
   drawnode->attrs = A_NORMAL;
   drawlist = g_list_insert(drawlist, drawnode, ++atpos);
  }
  ho = g_list_next(ho);
 }
 reorderScrpos(1);
}

void extDrawListGroup(gint atpos, gchar *group, GList *hosts)
{
 guint i = 0;
 GList *ho;
 DrawNode *drawnode = NULL;

 ho = g_list_first(hosts);

 while(ho) {
  if(!g_strcasecmp(((HostNode *) ho->data)->group, group) && 
     drawCategories[((HostNode *) ho->data)->category] == incategory) {
   drawnode = g_new0(DrawNode, 1);
   drawnode->p = ((HostNode *) ho->data);
   drawnode->type = HOST;
   drawnode->extended = FALSE;
   drawnode->selected = FALSE;
   drawnode->scrpos = 0;
   if (((HostNode *) ho->data)->category != C_SESSIONS)
     drawnode->elements = g_list_length(((HostNode *) ho->data)->updates);
   else
     drawnode->elements = g_list_length(((HostNode *) ho->data)->screens);
   drawnode->attrs = A_NORMAL;
   drawlist = g_list_insert(drawlist, drawnode, ++atpos);
  }
  ho = g_list_next(ho);
 }
 reorderScrpos(1);
}

void extDrawListHost(gint atpos, HostNode *n)
{
 guint i = 0;
 DrawNode *drawnode = NULL;

 if (n->category == C_SESSIONS) {
   GList *sess = g_list_first(n->screens);
   
   while(sess) {
     drawnode = g_new0(DrawNode, 1);
     drawnode->p = ((SessNode *)sess->data);
     drawnode->type = SESSION;
     drawlist = g_list_insert(drawlist, drawnode, ++atpos);
     sess = g_list_next(sess);
   }
   reorderScrpos(1);

   return;
 }
      

 GList *upd = g_list_first(n->updates);

 while(upd) {
  drawnode = g_new0(DrawNode, 1);
  drawnode->p = ((UpdNode *) upd->data);
  drawnode->type = UPDATE;
  drawnode->extended = FALSE;
  drawnode->selected = FALSE;
  drawnode->scrpos = 0;
  drawnode->elements = 0;
  drawnode->attrs = A_BOLD;
  drawlist = g_list_insert(drawlist, drawnode, ++atpos);
  upd = g_list_next(upd);
 }
 reorderScrpos(1);
}

void shrinkDrawListCategory(gint atpos)
{
 GList *dl;
 DrawNode *drawnode = NULL;

 dl = g_list_nth(drawlist, (guint) atpos+1);

 while(dl) {
  drawnode = ((DrawNode *) dl->data);
  if(drawnode->type != CATEGORY) {
   drawlist = g_list_remove(drawlist, drawnode);
   freeDrawNode(drawnode);
  }
  else break;
  dl = g_list_nth(drawlist, (guint) atpos+1);
 }
 reorderScrpos(1);
}

void shrinkDrawListGroup(gint atpos)
{
 GList *dl;
 DrawNode *drawnode = NULL;

 dl = g_list_nth(drawlist, (guint) atpos+1);

 while(dl) {
  drawnode = ((DrawNode *) dl->data);
  if(drawnode->type != GROUP && drawnode->type != CATEGORY) {
   drawlist = g_list_remove(drawlist, drawnode);
   freeDrawNode(drawnode);
  }
  else break;
  dl = g_list_nth(drawlist, (guint) atpos+1);
 }
 reorderScrpos(1);
}

void shrinkDrawListHost(gint atpos)
{
 GList *dl;
 DrawNode *drawnode = NULL;

 dl = g_list_nth(drawlist, (guint) atpos+1);

 while(dl) {
  drawnode = ((DrawNode *) dl->data);
  if(drawnode->type != GROUP && drawnode->type != CATEGORY && drawnode->type != HOST) {
   drawlist = g_list_remove(drawlist, drawnode);
   freeDrawNode(drawnode);
  }
  else break;
  dl = g_list_nth(drawlist, (guint) atpos+1);
 }
 reorderScrpos(1);
}

void extDrawList(gint atpos, gboolean extend, DrawNode *atn, GList *hosts)
{
 if(!atn) return;

 if(extend == TRUE) {
  switch(atn->type) {
  case CATEGORY:
   extDrawListCategory(atpos, (gchar *) atn->p, hosts);
   break;
  case GROUP:
   extDrawListGroup(atpos, (gchar *) atn->p, hosts);
   break;
  case HOST:
   extDrawListHost(atpos, (HostNode *) atn->p);
   break;
  default:
   break;
  }
 }
 else {
  switch(atn->type) {
  case CATEGORY:
   shrinkDrawListCategory(atpos);
   break;
  case GROUP:
   shrinkDrawListGroup(atpos);
   break;
  case HOST:
   shrinkDrawListHost(atpos);
   break;
  default:
   break;
  }
 }
}

gboolean ctrlKeyEnter(GList *hosts)
{
 gboolean ret = TRUE;
 gint extpos = 0;
 GList *dl;

 dl = g_list_first(drawlist);
 while (dl && (((DrawNode *) dl->data)->selected != TRUE)) {
  extpos++;
  dl = g_list_next(dl);
 }

 if (((DrawNode *) dl->data)->elements == 0) return (ret);

 ((DrawNode *) dl->data)->extended = ((DrawNode *) dl->data)->extended == TRUE ? FALSE : TRUE;

 extDrawList(extpos, ((DrawNode *) dl->data)->extended,(DrawNode *) dl->data, hosts);
 return (ret);
}

gboolean ctrlKeyLeft(GList *hosts)
{
 gboolean ret = TRUE;
 GList *dl, *dl_1;

 dl = g_list_first(drawlist);
 while (dl && (((DrawNode *) dl->data)->selected != TRUE)) dl = g_list_next(dl);

 if (((DrawNode *) dl->data)->extended) ret = ctrlKeyEnter(hosts);
 else if (((DrawNode *) dl->data)->type > 0){
  setEntryActiveStatus((DrawNode *) dl->data, FALSE);
  dl_1 = dl;
  while (dl && (((DrawNode *) dl->data)->type == ((DrawNode *) dl_1->data)->type)) dl = g_list_previous(dl);
  if (dl) {
   setEntryActiveStatus((DrawNode *) dl->data, TRUE);
   if(((DrawNode *) dl->data)->scrpos == 0) {
    ((DrawNode *) dl->data)->scrpos = 1;
    reorderScrpos(1);
   }
  }
  else setEntryActiveStatus((DrawNode *) dl_1->data, TRUE);
 }
 else {
  setEntryActiveStatus((DrawNode *) dl->data, FALSE);
  dl = g_list_first(drawlist);
  if (dl) {
   ((DrawNode *) dl->data)->scrpos = 1;
   setEntryActiveStatus((DrawNode *) dl->data, TRUE);
   reorderScrpos(1);
  }
 }
 return (ret);
}

gboolean ctrlKeyRight(GList *hosts)
{
 gboolean ret = TRUE;
 GList *dl, *dl_1;

 dl = g_list_first(drawlist);
 while (dl && (((DrawNode *) dl->data)->selected != TRUE)) dl = g_list_next(dl);

 if (!((DrawNode *) dl->data)->extended) ret = ctrlKeyEnter(hosts);
 return (ret);
}

gboolean ctrlKeyPgDown()
{
 gboolean ret = TRUE;
 GList *dl;

 dl = g_list_first(drawlist);
 while (dl && (((DrawNode *) dl->data)->selected != TRUE)) dl = g_list_next(dl);

 setEntryActiveStatus((DrawNode *) dl->data, FALSE);
 reorderScrpos(LINES-2);

 dl = g_list_first(drawlist);
 while (dl && (((DrawNode *) dl->data)->scrpos != 1)) dl = g_list_next(dl);

 if(dl) setEntryActiveStatus((DrawNode *) dl->data, TRUE);
 else {
  dl = g_list_last(drawlist);
  if(dl) {
   setEntryActiveStatus((DrawNode *) dl->data, TRUE);
   ((DrawNode *) dl->data)->scrpos =1;
  }
 }

 return (ret);
}

gboolean ctrlKeyPgUp()
{
 gboolean   ret = TRUE;
 gint       i = 0;
 GList      *dl, *dl_1;

 dl = g_list_first(drawlist);
 while (dl && (((DrawNode *) dl->data)->selected != TRUE)) dl = g_list_next(dl);

 setEntryActiveStatus((DrawNode *) dl->data, FALSE);

 dl = g_list_first(drawlist);
 while (dl && (((DrawNode *) dl->data)->scrpos != 1)) dl = g_list_next(dl);

 dl_1 = dl;
 dl_1 = g_list_previous(dl_1);

 while (dl && (++i < LINES-2)) dl = g_list_previous(dl);

 if (dl && dl_1) {
  ((DrawNode *) dl->data)->scrpos =1;
  setEntryActiveStatus((DrawNode *) dl_1->data, TRUE);
 }
 else {
  dl = g_list_first(drawlist);
  if(dl) {
   ((DrawNode *) dl->data)->scrpos =1;
   setEntryActiveStatus((DrawNode *) dl->data, TRUE);
  }
 }
 reorderScrpos(1);
 return (ret);
}

gboolean ctrlUI (GList *hosts)
{
 int ic = 0;
 gboolean ret = TRUE;
 gboolean refscr = FALSE;
 DrawNode *n;
 static   gchar in[64];
 gchar    *pkg = NULL;

 if(rebuilddl == TRUE) {
  rebuildDrawList(hosts);
  refscr = TRUE;
 }

 ic = tolower(getch());

 /* To slow down the idle process. */
 if(ic == ERR)
  g_usleep(10000);

 switch(ic) {
 case KEY_HOME:
 case KEY_END:
 case KEY_UP:
 case KEY_DOWN:
  refscr = ctrlKeyUpDown(ic);
  break;
 case KEY_PPAGE:
  refscr = ctrlKeyPgUp();
  break;
 case KEY_NPAGE:
  refscr = ctrlKeyPgDown();
  break;
 case KEY_LEFT:
  refscr = ctrlKeyLeft(hosts);
  break;
 case KEY_RIGHT:
  refscr = ctrlKeyRight(hosts);
  break;
 case '+':
 case ' ':
 case 13:
  refscr = ctrlKeyEnter(hosts);
  break;
 case 'g':
  n = getSelectedDrawNode();
  if(!n) break;
  if(n->type == HOST) {
   if(n->extended == TRUE) n->extended = FALSE;
   ((HostNode *) n->p)->category = C_REFRESH_REQUIRED;
   rebuildDrawList(hosts);
   refscr = TRUE;
  }
  else
   if(n->type == GROUP) {
    if(n->extended == TRUE) n->extended = FALSE;
    if(drawCategories[getCategoryNumber(incategory)]) {
     setHostsCategory(hosts, getCategoryNumber(incategory), ingroup, 
		      (Category) C_REFRESH_REQUIRED);
     rebuildDrawList(hosts);
     refscr = TRUE;
    }
   }
   else
    if(n->type == CATEGORY) {
     if(n->extended == TRUE) n->extended = FALSE;
     if(drawCategories[getCategoryNumber(incategory)]) {
      setHostsCategory(hosts, getCategoryNumber(incategory), NULL, (Category) C_REFRESH_REQUIRED);
      rebuildDrawList(hosts);
      refscr = TRUE;
     }
    }
  break;
 case 'c':
  n = getSelectedDrawNode();
  if(!n) break;
  if(n->type == HOST) {
   if(n->extended == TRUE) n->extended = FALSE;
   cleanUI();
   ssh_connect(((HostNode *) n->p)->hostname, ((HostNode *) n->p)->ssh_user, ((HostNode *) n->p)->ssh_port);
   ((HostNode *) n->p)->category = C_REFRESH_REQUIRED;
   freeUpdates(((HostNode *) n->p)->updates);
   ((HostNode *) n->p)->updates = NULL;
   rebuildDrawList(hosts);
   initUI();
   refscr = TRUE;
  }
  break;
 case 'u':
  n = getSelectedDrawNode();
  if(!n) break;
  if(n->type == HOST) {
   if(n->extended == TRUE) n->extended = FALSE;
   cleanUI();
   ssh_cmd_upgrade(((HostNode *) n->p)->hostname, ((HostNode *) n->p)->ssh_user, ((HostNode *) n->p)->ssh_port);
   ((HostNode *) n->p)->category = C_REFRESH_REQUIRED;
   freeUpdates(((HostNode *) n->p)->updates);
   ((HostNode *) n->p)->updates = NULL;
   rebuildDrawList(hosts);
   initUI();
   refscr = TRUE;
  }
  break;
 case 'i':
  n = getSelectedDrawNode();
  if(!inhost) break;
  
  if(!n || (n->type != UPDATE)) {
   queryString("Install package: ", in, sizeof(in));
    if (strlen(in)==0)
     break;
    pkg = in;
  }
  else {
    pkg = ((UpdNode *) n->p)->package;
  }
  cleanUI();
  ssh_cmd_install(inhost->hostname, inhost->ssh_user, inhost->ssh_port, pkg);
  inhost->category = C_REFRESH_REQUIRED;
  freeUpdates(inhost->updates);
  inhost->updates = NULL;
  rebuildDrawList(hosts);
  initUI();
  refscr = TRUE;
  break;
 case 'q':
  ret = FALSE;
  g_main_loop_quit (loop);
  break;
 default:
  break;
 }
 if(refscr == TRUE) {
  getOldestMtime(hosts);
  refreshDraw();
 }
 return (ret);
}
