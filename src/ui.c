/* $Id$
 *
 */

#include <curses.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <signal.h>
#include "apt-dater.h"
#include "ui.h"
#include "colors.h"
#include "screen.h"
#include "exec.h"
#include "stats.h"
#include "keyfiles.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef FEAT_TCLFILTER
#include <tcl.h>
#endif

static GList *drawlist = NULL;
gchar *drawCategories[] = {
    "Updates pending",
    "Up to date",
    "Status file missing",
    "Refresh required",
    "In refresh",
    "Sessions",
#ifdef FEAT_TCLFILTER
    "Filtered",
#endif
    "Unknown",
    NULL};
static gchar *incategory = NULL;
static gchar *ingroup = NULL;
static HostNode *inhost = NULL;
static gint bottomDrawLine;
static WINDOW *win_dump = NULL;
static gboolean dump_screen = FALSE;
gchar  maintainer[48];
#ifdef FEAT_TCLFILTER
gchar  filterexp[0x1ff];
#endif
gint   sc_mask = 0;
static GCompletion* hstCompl = NULL;
static Tcl_Interp *tcl_interp;

#ifdef FEAT_TCLFILTER
typedef enum {
    TCLM_STRING,
    TCLM_INT,
    TCLM_IGNORE,
} ETCLMAPING;

typedef enum {
    TCLMK_CATEGORY,
    TCLMK_GROUP,
    TCLMK_HOSTNAME,
    TCLMK_KERNEL,
    TCLMK_LSBCNAME,
    TCLMK_LSBDISTRI,
    TCLMK_LSBREL,

    TCLMK_EXTRAS,
    TCLMK_FLAGS,
    TCLMK_HOLDS,
    TCLMK_INSTALLED,
    TCLMK_UPDATES,
} ETCLMAPKEYS;

struct TCLMapping {
    gint code;
    gchar *name;
    ETCLMAPING type;
};

const static struct TCLMapping tclmap[] = {
    {TCLMK_CATEGORY , "cat"       , TCLM_INT},
    {TCLMK_GROUP    , "group"     , TCLM_STRING},
    {TCLMK_HOSTNAME , "hostname"  , TCLM_STRING},
    {TCLMK_KERNEL   , "kernel"    , TCLM_STRING},
    {TCLMK_LSBCNAME , "lsb_cname" , TCLM_STRING},
    {TCLMK_LSBDISTRI, "lsb_distri", TCLM_STRING},
    {TCLMK_LSBREL   , "lsb_rel"   , TCLM_STRING},

    {TCLMK_EXTRAS   , "extras"    , TCLM_IGNORE},
    {TCLMK_FLAGS    , "flags"     , TCLM_IGNORE},
    {TCLMK_HOLDS    , "holds"     , TCLM_IGNORE},
    {TCLMK_INSTALLED, "installed" , TCLM_IGNORE},
    {TCLMK_UPDATES  , "updates"   , TCLM_IGNORE},

    {0              ,         NULL,           0},
};
#endif

struct ShortCut {
 EShortCuts sc;
 int keycode;
 gchar *key;
 gchar *descr;
 gboolean visible;
 EVisKeyMask id;
};

static struct ShortCut shortCuts[] = {
 {SC_KEY_LEFT, KEY_LEFT, "<Left>", "shrink node" , FALSE, 0},
 {SC_KEY_RIGHT, KEY_RIGHT, "<Right>", "expand node" , FALSE, 0},
 {SC_KEY_SPACE, ' ', "<Space>", "shrink/expand node" , FALSE, 0},
 {SC_KEY_RETURN, 13, "<Return>", "shrink/expand node" , FALSE, 0},
 {SC_KEY_UP, KEY_UP, "<Up>", "move up" , FALSE, 0},
 {SC_KEY_DOWN, KEY_DOWN, "<Down>", "move down" , FALSE, 0},
 {SC_KEY_HOME, KEY_HOME, "<Home>", "move to the top" , FALSE, 0},
 {SC_KEY_END, KEY_END, "<End>", "move to the end" , FALSE, 0},
 {SC_KEY_PPAGE, KEY_PPAGE, "<PageUp>", "previous page" , FALSE, 0},
 {SC_KEY_NPAGE, KEY_NPAGE, "<PageDown>", "next page" , FALSE, 0},
 {SC_KEY_PLUS, '+', "+", "shrink/expand node" , FALSE, 0},
 {SC_KEY_QUIT, 'q', "q" , "quit" , TRUE , 0},
 {SC_KEY_HELP, '?', "?" , "help" , TRUE , 0},
 {SC_KEY_FIND, '/', "/" , "search host" , TRUE , 0},
#ifdef FEAT_TCLFILTER
 {SC_KEY_FILTER, 'f', "f" , "filter hosts" , FALSE, 0},
#endif
 {SC_KEY_ATTACH, 'a', "a" , "attach session" , FALSE, VK_ATTACH},
 {SC_KEY_KILLSESS, 'K', "K" , "kill session" , FALSE, VK_KILL},
 {SC_KEY_CONNECT, 'c', "c" , "connect host" , FALSE, VK_CONNECT},
 {SC_KEY_FILETRANS, 't', "f" , "file transfer" , FALSE, 0},
 {SC_KEY_TOGGLEDUMPS, 'd', "d" , "toggle dumps" , FALSE, VK_DUMP},
 {SC_KEY_REFRESH, 'g', "g" , "refresh host" , FALSE, VK_REFRESH},
 {SC_KEY_INSTALL, 'i', "i" , "install pkg" , FALSE, VK_INSTALL},
 {SC_KEY_UPGRADE, 'u', "u" , "upgrade host(s)" , FALSE, VK_UPGRADE},
 {SC_KEY_NEXTSESS, 'n', "n" , "next detached session" , FALSE, 0},
 {SC_KEY_CYCLESESS, 'N', "N" , "cycle detached sessions" , FALSE, 0},
 {SC_MAX, 0, NULL, NULL, FALSE, 0},
};

struct HostFlag {
  gint flag;
  gchar *code;
  gchar *descr;
};

static const struct HostFlag hostFlags[] = {
  {HOST_STATUS_PKGKEPTBACK    ,  "h", "some packages are kept back"},
  {HOST_STATUS_PKGEXTRA       ,  "x", "extra packages are installed"},
  {HOST_STATUS_KERNELNOTMATCH ,  "R", "running kernel is not the latest (reboot required)"},
  {HOST_STATUS_KERNELSELFBUILD,  "k", "a selfbuild kernel is running"},
  {0                          , NULL, NULL},
};

static void setMenuEntries(gint mask) {
  mask |= sc_mask;
  gint i = -1;
  while(shortCuts[++i].key) {
    if(shortCuts[i].id == 0)
      continue;

    shortCuts[i].visible = mask & shortCuts[i].id;
  }
}

static gchar *compHost(gpointer p) {
  return ((HostNode *)p)->hostname;
}

void freeDrawNode (DrawNode *n)
{
 g_free(n);
}

DrawNode *getSelectedDrawNode()
{
 GList *dl;

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
#ifdef FEAT_TCLFILTER
  if(category == C_FILTERED) {
    if((((HostNode *) ho->data)->filtered) && (!g_strcasecmp(((HostNode *) ho->data)->group, group))) cnt++;
  }
  else
#endif
   if((((HostNode *) ho->data)->category == category) && (!g_strcasecmp(((HostNode *) ho->data)->group, group))) cnt++;

  ho = g_list_next(ho);
 }
 return(cnt);
}

void disableInput() {
 noecho();
 curs_set(0);
 leaveok(stdscr, TRUE);
 /* To slow down the idle process. */
 timeout(200);
}

void enableInput() {
 echo();
 curs_set(1);
 leaveok(stdscr, FALSE);
 timeout(-1);
}


void initUI ()
{
 initscr();
 ui_start_color();
 cbreak();
 nonl();
 disableInput();
 intrflush(stdscr, FALSE);
 keypad(stdscr, TRUE);
 clear();
 refresh();

 bottomDrawLine = LINES-2;
}


void cleanUI ()
{
 clear();
 endwin();
}

void drawMenu (gint mask)
{
 setMenuEntries(mask);

 attron(uicolors[UI_COLOR_MENU]);
 move(0,0);
 hline(' ',COLS);

 gint i=-1;
 gint p=COLS;

 while(shortCuts[++i].key) {
   if(!shortCuts[i].visible)
     continue;

   addnstr(shortCuts[i].key, MAX(0, p));
   p -= strlen(shortCuts[i].key);

   addnstr(":", MAX(0, p));
   p--;

   addnstr(shortCuts[i].descr, MAX(0, p));
   p -= strlen(shortCuts[i].descr);

   addnstr("  ", MAX(0, p));
   p-=2;
 }

 attroff(uicolors[UI_COLOR_MENU]);
}

void drawStatus (char *str)
{
 char strmtime[30];
 struct tm *tm_mtime;
 int mtime_pos;

 tm_mtime = localtime(&oldest_st_mtime);
 strftime(strmtime, sizeof(strmtime), " Oldest: %D %H:%M", tm_mtime);

 attron(uicolors[UI_COLOR_STATUS]);
 move(bottomDrawLine, 0);
 hline( ' ', COLS);
 if(str) {
  addnstr(str, COLS);
 }

 mtime_pos = COLS - strlen(strmtime) - 1;

 move(bottomDrawLine, mtime_pos);
 addnstr(strmtime, COLS-mtime_pos);

 attroff(uicolors[UI_COLOR_STATUS]);
}

void queryString(const gchar *query, gchar *in, const gint size)
{
 gint i;

 enableInput();

 attron(uicolors[UI_COLOR_QUERY]);
 mvaddstr(LINES - 1, 0, query);
 attroff(uicolors[UI_COLOR_QUERY]);

 move(LINES - 1, strlen(query));

 attron(uicolors[UI_COLOR_INPUT]);

 for(i = strlen(in)-1; i>=0; i--)
  ungetch(in[i]);

 getnstr(in, size);
 disableInput();

 attroff(uicolors[UI_COLOR_INPUT]);

 move(LINES - 1, 0);
 hline(' ', COLS);
}

gboolean queryConfirm(const gchar *query, const gboolean enter_is_yes)
{
 int c;

 enableInput();

 attron(uicolors[UI_COLOR_QUERY]);
 mvaddstr(LINES - 1, 0, query);
 attroff(uicolors[UI_COLOR_QUERY]);

 move(LINES - 1, strlen(query));

 attron(uicolors[UI_COLOR_INPUT]);
 c = getch();
 attroff(uicolors[UI_COLOR_INPUT]);

 disableInput();

 move(LINES - 1, 0);
 hline(' ', COLS);

 return ((c == 'y') || (c == 'Y') || 
	 (enter_is_yes == TRUE && (c == 0xd || c == KEY_ENTER)));
}

void drawCategoryEntry (DrawNode *n)
{
 char statusln[BUF_MAX_LEN];

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

  drawMenu(VK_REFRESH);
  drawStatus(statusln);
 }
}

void drawGroupEntry (DrawNode *n)
{
 char statusln[BUF_MAX_LEN];

 attron(n->attrs);
 mvhline(n->scrpos, 0, ' ', COLS); 
 mvaddstr(n->scrpos, 2, " [");
 addch(n->elements > 0 && n->extended == FALSE ? '+' : '-');
 addstr("] ");
 addnstr((char *) n->p, COLS);
 attroff(n->attrs);

 if(n->selected == TRUE) {
  sprintf(statusln, "%d %s is in status \"%s\"", n->elements, n->elements > 1 || n->elements == 0 ? "Hosts" : "Host", incategory);

  drawMenu(VK_REFRESH);
  drawStatus(statusln);
 }
}

void drawHostEntry (DrawNode *n)
{
 char statusln[BUF_MAX_LEN];
 gint mask = 0;
 char *hostentry;

 attron(n->attrs);
 mvhline(n->scrpos, 0, ' ', COLS);

 if (((HostNode *) n->p)->status & HOST_STATUS_LOCKED) {
  attron(uicolors[UI_COLOR_HOSTSTATUS]);
  mvaddstr(n->scrpos, 1, "L");
  attroff(uicolors[UI_COLOR_HOSTSTATUS]);
 }
 else {
  move(n->scrpos, 1);
  attron(uicolors[UI_COLOR_HOSTSTATUS]);

  gint i = 0;
  while(hostFlags[i].flag) {
    if (((HostNode *) n->p)->status & hostFlags[i].flag)
      addch(hostFlags[i].code[0]);
    else
      addch(' ');
      
    i++;
  }

  attroff(uicolors[UI_COLOR_HOSTSTATUS]);
 }
 mvaddstr(n->scrpos, 4, " [");

 if(n->elements > 0)
  addch(n->elements > 0 && n->extended == FALSE ? '+' : '-');
 else
  addch(' ');
 addstr("] ");

 if(((HostNode *) n->p)->lsb_distributor) {
  hostentry = g_strdup_printf("%s (%s %s %s; %s)", 
			      ((HostNode *) n->p)->hostname,
			      ((HostNode *) n->p)->lsb_distributor, 
			      ((HostNode *) n->p)->lsb_release,
			      ((HostNode *) n->p)->lsb_codename,
			      ((HostNode *) n->p)->kernelrel);
  
  addnstr((char *) hostentry, COLS);
  g_free(hostentry);
 } else {
  addnstr((char *) ((HostNode *) n->p)->hostname, COLS);
 }

 attroff(n->attrs);


 if(n->selected == TRUE) {
  switch(((HostNode *) n->p)->category) {
  case C_UPDATES_PENDING:
   mask = VK_CONNECT | VK_UPGRADE | VK_REFRESH | VK_INSTALL;
   sprintf(statusln, "%d %s required", ((HostNode *) n->p)->nupdates, ((HostNode *) n->p)->nupdates > 1 || ((HostNode *) n->p)->nupdates == 0 ? "Updates" : "Update");
   break;
  case C_UP_TO_DATE:
   mask = VK_CONNECT | VK_REFRESH | VK_INSTALL;
   sprintf(statusln, "No update required");
   break;
  case C_NO_STATS:
   mask = VK_CONNECT | VK_REFRESH | VK_INSTALL;
   sprintf(statusln, "Statusfile is missing");
   break;
  case C_REFRESH_REQUIRED:
   mask = VK_CONNECT | VK_REFRESH | VK_INSTALL;
   sprintf(statusln, "Refresh required");
   break;
  case C_REFRESH:
   sprintf(statusln, "In refresh");
   break;
  case C_SESSIONS:
   mask = VK_ATTACH;
   sprintf(statusln, "%d session%s running", g_list_length(((HostNode *) n->p)->screens),
	   (g_list_length(((HostNode *) n->p)->screens)==1?"":"s"));
   break;
  default:
   mask = VK_CONNECT | VK_REFRESH | VK_INSTALL;
   sprintf(statusln, "Status is unknown");
   break;
  }

  if (((HostNode *) n->p)->status & HOST_STATUS_LOCKED) 
   strcat(statusln," - host locked by another process");
  drawMenu(mask);

  drawStatus(statusln);
 }
}

void drawPackageEntry (DrawNode *n)
{
 char statusln[BUF_MAX_LEN];

 attron(n->attrs);
 mvhline(n->scrpos, 0, ' ', COLS);

 if(((PkgNode *) n->p)->flag & HOST_STATUS_PKGUPDATE)
   mvaddstr(n->scrpos, 7, "u:");
 else if(((PkgNode *) n->p)->flag & HOST_STATUS_PKGKEPTBACK)
   mvaddstr(n->scrpos, 7, "h:");
 else if(((PkgNode *) n->p)->flag & HOST_STATUS_PKGEXTRA)
   mvaddstr(n->scrpos, 7, "x:");

 mvaddnstr(n->scrpos, 10, (char *) ((PkgNode *) n->p)->package, COLS-10);
 attroff(n->attrs);
 if(n->selected == TRUE) {
  if(((PkgNode *) n->p)->data)
    sprintf(statusln, "%s -> %s", ((PkgNode *) n->p)->version, ((PkgNode *) n->p)->data);
  else
    sprintf(statusln, "%s", ((PkgNode *) n->p)->version);
  drawMenu(VK_INSTALL);
  drawStatus(statusln);
 }
}

void drawSessionEntry (DrawNode *n)
{
 char h[BUF_MAX_LEN];
 struct tm *tm_mtime;

 snprintf(h, sizeof(h), "%5d: ", ((SessNode *) n->p)->pid);

 tm_mtime = localtime(&((SessNode *) n->p)->st.st_mtime);
 strftime(&h[strlen(h)], sizeof(h)-strlen(h), "%D %H:%M ", tm_mtime);

 snprintf(&h[strlen(h)], sizeof(h)-strlen(h), "(%s)",
	  (screen_is_attached((SessNode *) n->p) ? "Attached" : "Detached"));


 attron(n->attrs);
 mvhline(n->scrpos, 0, ' ', COLS);
 mvaddnstr(n->scrpos, 7, h, COLS);
 attroff(n->attrs);
 if(n->selected == TRUE) {
  if (screen_is_attached((SessNode *) n->p))
    drawMenu(VK_ATTACH | VK_DUMP);
  else
    drawMenu(VK_ATTACH | VK_DUMP | VK_KILL);

  if (dump_screen) {
    gchar *dump = screen_get_dump((SessNode *) n->p);

    if(dump) {
      if (win_dump) {
	wmove(win_dump, 0, 0);
	waddstr(win_dump, dump);
	wrefresh(win_dump);
      }

      drawStatus("Running session:");

      g_free(dump);
    }
    else
      drawStatus("Could not read session dump.");
  }
  else
    drawStatus("");
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
 if(n->scrpos == 0 || n->scrpos >= bottomDrawLine) return;

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
 case PKG:
  drawPackageEntry(n);
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

 if(dump_screen) {
   if(getSelectedDrawNode()->type == SESSION) {
     if (!win_dump) {
       bottomDrawLine = LINES/2;

       win_dump = subwin(stdscr, bottomDrawLine-1, COLS, LINES-bottomDrawLine, 0);
       scrollok(win_dump, TRUE);
       syncok(win_dump, TRUE);
     }
   }
   else {
     bottomDrawLine = LINES - 2;

     if(win_dump) {
       delwin(win_dump);
       win_dump = NULL;
     }
   }
 }
 else {
   if(win_dump) {
     delwin(win_dump);
     win_dump = NULL;
   }

   bottomDrawLine = LINES - 2;
 }

 g_list_foreach(drawlist, (GFunc) drawEntry, NULL);
 refresh();
}

void refreshUI()
{
 refresh();
}

void setEntryActiveStatus(DrawNode *n, gboolean active)
{
 if(!n) return;

 if(active == TRUE) {
  n->selected = TRUE;
  n->attrs|=uicolors[UI_COLOR_SELECTOR];
 }
 else {
  n->selected = FALSE;
  if(n->attrs & uicolors[UI_COLOR_SELECTOR])
   n->attrs^=uicolors[UI_COLOR_SELECTOR];
 }
}


guint getHostCatCnt(GList *hosts, Category category)
{
 guint cnt = 0;
 GList *ho;

 ho = g_list_first(hosts);
 while(ho) {
#ifdef FEAT_TCLFILTER
  if(category == C_FILTERED) {
    if(((HostNode *) ho->data)->filtered) cnt++;
  }
  else
#endif
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
  while (dl && (((DrawNode *) dl->data)->scrpos < bottomDrawLine)) 
   dl = g_list_next(dl);
  if (!dl) dl = g_list_last(drawlist);
  setEntryActiveStatus((DrawNode *) dl->data, TRUE);
  return;
 }
 if (dn -> scrpos == 0 || dn -> scrpos > bottomDrawLine) {
  dl = g_list_first(drawlist);

  while(dl && (dl->data != dn))
   dl = g_list_next(dl);

  gint p = bottomDrawLine-1;
  while(dl && p) {
   ((DrawNode *)dl->data)->scrpos = p--;
   dl = g_list_previous(dl);
  }

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
   while (++count <= bottomDrawLine) {
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
#ifdef FEAT_TCLFILTER
 /* Prepare TCL interpreter */
 if(cfg->filterexp)
  strcpy(filterexp, cfg->filterexp);
 else
  filterexp[0] = 0;

 tcl_interp = Tcl_CreateInterp();
 if(cfg->filterfile)
   Tcl_EvalFile(tcl_interp, cfg->filterfile);
#endif

 initUI();

 /* Create completion list. */
 hstCompl = g_completion_new(compHost);
 g_completion_add_items(hstCompl, hosts);

 /* Draw the host entries intial. */
 buildIntialDrawList(hosts);
 reorderScrpos(1);
 refreshDraw();

 const gchar *m = getenv("MAINTAINER");
 if (m)
   strncpy(maintainer, m, sizeof(maintainer));
 else {
   struct passwd *pw = getpwuid(getuid());
   if (pw && pw->pw_gecos)
     strncpy(maintainer, pw->pw_gecos, sizeof(maintainer));
   else
     maintainer[0] = 0;
 }

 if ((cfg->query_maintainer == 1) ||
     ((cfg->query_maintainer > 1) && (m == NULL))) {
   WINDOW *w = newwin(5, 52, LINES/2-3, (COLS-52)/2);
   box(w,0,0);

   enableInput();
   wattron(w, uicolors[UI_COLOR_QUERY]);
   mvwaddstr(w, 1, 2, "Maintainer name:");
   wattroff(w, uicolors[UI_COLOR_QUERY]);
   wmove(w, 3, 2);
   
   wattron(w, uicolors[UI_COLOR_INPUT]);

   int i;
   for(i = strlen(maintainer)-1; i>=0; i--)
     ungetch(maintainer[i]);
   
   wgetnstr(w, maintainer, sizeof(maintainer));
   disableInput();

   wattroff(w, uicolors[UI_COLOR_INPUT]);

   delwin(w);
   refreshDraw();
 }
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
   if(((DrawNode *) dl->data)->scrpos >= bottomDrawLine || 
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
     ((DrawNode *) dl->data)->scrpos > bottomDrawLine) {
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
 case PKG:
  if (!g_strcasecmp(((PkgNode *) n1->p)->package,
		    ((PkgNode *) n2->p)->package)) return(TRUE);
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
  if(
#ifdef FEAT_TCLFILTER
     (i == C_FILTERED) ||
#endif
     (((HostNode *) ho->data)->category == i)
    ) {
   if((drawnode) && (!g_strcasecmp(drawnode->p, 
				   ((HostNode *) ho->data)->group))) {
    ho = g_list_next(ho);
    continue;
   }
   gint elements = getHostGrpCatCnt(hosts, ((HostNode *) ho->data)->group, i);
   if(elements) {
     drawnode = g_new0(DrawNode, 1);
     drawnode->p = (((HostNode *) ho->data)->group);
     drawnode->type = GROUP;
     drawnode->extended = FALSE;
     drawnode->selected = FALSE;
     drawnode->scrpos = 0;
     drawnode->elements = elements;
     drawnode->attrs = A_NORMAL;
     drawlist = g_list_insert(drawlist, drawnode, ++atpos);
   }
  }
  ho = g_list_next(ho);
 }
 reorderScrpos(1);
}

void extDrawListGroup(gint atpos, gchar *group, GList *hosts)
{
 GList *ho;
 DrawNode *drawnode = NULL;

 ho = g_list_first(hosts);

 while(ho) {
  if(!g_strcasecmp(((HostNode *) ho->data)->group, group) && 
     (
#ifdef FEAT_TCLFILTER
      (incategory == drawCategories[C_FILTERED] && ((HostNode *) ho->data)->filtered) ||
#endif
      (drawCategories[((HostNode *) ho->data)->category] == incategory)
     )) {
   drawnode = g_new0(DrawNode, 1);
   drawnode->p = ((HostNode *) ho->data);
   drawnode->type = HOST;
   drawnode->extended = FALSE;
   drawnode->selected = FALSE;
   drawnode->scrpos = 0;
   if (((HostNode *) ho->data)->category != C_SESSIONS)
     drawnode->elements = ((HostNode *) ho->data)->nupdates + ((HostNode *) ho->data)->nholds + ((HostNode *) ho->data)->nextras;
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
      

 GList *upd = g_list_first(n->packages);
 while(upd) {
  if((((PkgNode *) upd->data)->flag & HOST_STATUS_PKGUPDATE) ||
     (((PkgNode *) upd->data)->flag & HOST_STATUS_PKGKEPTBACK) ||
     (((PkgNode *) upd->data)->flag & HOST_STATUS_PKGEXTRA)) {
    drawnode = g_new0(DrawNode, 1);
    drawnode->type = PKG;
    drawnode->p = ((PkgNode *) upd->data);
    drawnode->attrs = (((PkgNode *) upd->data)->flag & HOST_STATUS_PKGUPDATE) ? A_BOLD : A_NORMAL;
    drawlist = g_list_insert(drawlist, drawnode, ++atpos);
  }
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
 GList *dl;

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
 reorderScrpos(bottomDrawLine);

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

 while (dl && (++i < bottomDrawLine)) dl = g_list_previous(dl);

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


void injectKey(int ch)
{
 ungetch(ch);
}


void searchEntry(GList *hosts) {
 gint c;
 gchar s[0xff];
 gint pos = 0;
 const gchar *query = "Search: ";
 const int offset = strlen(query)-1;
 GList *matches = NULL;
 GList *selmatch = NULL;

 enableInput();
 noecho();

 attron(uicolors[UI_COLOR_QUERY]);
 mvaddstr(LINES - 1, 0, query);
 attroff(uicolors[UI_COLOR_QUERY]);

 while((c = getch())) {
   /* handle backspace */
   if(c == KEY_BACKSPACE) {
     if (strlen(s)>0)
       s[--pos] = 0;
     else
       beep();
   }
   /* search again */
   else if((c == '/') || (c == '\t')) {
     if(selmatch && matches) {
       selmatch = g_list_next(selmatch);
       if(!selmatch)
	 selmatch = g_list_first(matches);
     }

     if(!selmatch)
       beep();
   }
   /* abort on control key */
   else if(iscntrl(c)) {
     if((c==KEY_UP) ||
	(c==KEY_DOWN))
       ungetch(c);
     break;
   }
   /* accept char */
   else if(strlen(s)<sizeof(s)) {
     s[pos++] = c;
     s[pos] = 0;
   }

   /* print new search string */
   attron(uicolors[UI_COLOR_INPUT]);
   mvaddstr(LINES-1, offset+1, s);
   attroff(uicolors[UI_COLOR_INPUT]);

   /* find completion matches */
   if(strlen(s))
     matches = g_completion_complete(hstCompl, s, NULL);
   else
     matches = NULL;

   /* check if selected match is still valid... or get new one */
   if (matches) {
     if(!selmatch ||
	!g_list_find(matches, selmatch->data))
       selmatch = g_list_first(matches);

     if(selmatch) {
       attron(uicolors[UI_COLOR_INPUT]);
       attron(A_REVERSE);
       mvaddstr(LINES-1, offset+pos+1, &((HostNode *)selmatch->data)->hostname[pos]);
       attroff(A_REVERSE);
       attroff(uicolors[UI_COLOR_INPUT]);

       hline(' ', COLS-offset-strlen(&((HostNode *)selmatch->data)->hostname[pos])-1);
     }
     else
       hline(' ', COLS-offset-pos-1);


     /* we have a match which is selected */
     if(selmatch) {
       GList *dl;
       int i;
       
       /* UGLY: expand everything (so the matched host is on the drawlist) */
       for(i=2; i>0; i--) {
	 dl = g_list_first(drawlist);
	 while(dl) {
	   DrawNode *dn = (DrawNode *) dl->data;
	   
	   dn->extended = TRUE;
	   
	   dl = g_list_next(dl);
	 }
	 
	 rebuildDrawList(hosts);
       }
       
       /* clear selection */
       DrawNode *n = getSelectedDrawNode();
       if (n)
	 setEntryActiveStatus(n, FALSE);
       
       /* traverse drawlist bottom up... and only expand
	* the path to the selmatch */
       dl = g_list_last(drawlist);
       gint up = 0; /* 0: no expand; 2: exp. category 4: exp. group */
       while(dl) {
	 DrawNode *dn = (DrawNode *) dl->data;
	 
	 if((dn->type == HOST) && (selmatch->data == dn->p)) {
	   up = 4;
	   dn->selected = TRUE;
	 }
	 else if(dn->type == GROUP) {
	   if(up == 4) {
	     up = 2;
	     dn->extended = TRUE;
	   }
	   else
	     dn->extended = FALSE;
	 }
	 else if((dn->type == CATEGORY) &&
		 (up == 2)) {
	   up = 0;
	   dn->extended = TRUE;
	 }
	 else
	   dn->extended = FALSE;
	 
	 dl = g_list_previous(dl);
       }
       
       rebuildDrawList(hosts);
       g_list_foreach(drawlist, (GFunc) drawEntry, NULL);
     }
   }
   else
     hline(' ', COLS-offset-pos-1);

   /* print matches in status bar */
   attron(uicolors[UI_COLOR_STATUS]);
   move(bottomDrawLine, 0);
   hline(' ', COLS);

   if(matches) {
     gint p = COLS;

     addnstr("Hosts:", MAX(0, p));
     p-=6;

     GList *m = g_list_first(matches);
     while(m) {
       addnstr(" ", MAX(0, p));
       p--;

       addnstr(((HostNode *)m->data)->hostname, MAX(0, p));
       p -= strlen(((HostNode *)m->data)->hostname);

       m = g_list_next(m);
     }
   }
   else {
     addnstr("Hosts: -", COLS);
     selmatch = NULL;
   }
   attroff(uicolors[UI_COLOR_STATUS]);


   /* move to cursor position */
   move(LINES-1, offset+pos+1);


   refresh();
 }

 attroff(uicolors[UI_COLOR_INPUT]);

 disableInput();

 move(LINES - 1, 0);
 hline(' ', COLS);

 drawStatus ("");
}

#ifdef FEAT_TCLFILTER
void applyFilter(GList *hosts) {
 if(!strlen(filterexp)) return;

 gint i;
 gboolean filtered;
 GList *l = hosts;

 while(l) {
    HostNode *n = (HostNode *)l->data;
 
    for(i=0; tclmap[i].name; i++) {
      switch(tclmap[i].type) {
        case TCLM_STRING:
	  switch(tclmap[i].code) {
	    case TCLMK_GROUP:
        	Tcl_SetVar(tcl_interp, tclmap[i].name, n->group, 0);
		break;
	    case TCLMK_HOSTNAME:
        	Tcl_SetVar(tcl_interp, tclmap[i].name, n->hostname, 0);
		break;
	    case TCLMK_KERNEL:
        	Tcl_SetVar(tcl_interp, tclmap[i].name, n->kernelrel, 0);
		break;
	    case TCLMK_LSBCNAME:
        	Tcl_SetVar(tcl_interp, tclmap[i].name, n->lsb_codename, 0);
		break;
	    case TCLMK_LSBDISTRI:
        	Tcl_SetVar(tcl_interp, tclmap[i].name, n->lsb_distributor, 0);
		break;
	    case TCLMK_LSBREL:
        	Tcl_SetVar(tcl_interp, tclmap[i].name, n->lsb_release, 0);
		break;
	    default:
		g_warning("Internal error: unhandled TCL TCLM_STRING maping!\n");
	  }
	  break;
	case TCLM_INT:
	  {
	    gchar *h = NULL;

	    switch(tclmap[i].code) {
		case TCLMK_CATEGORY:
		    h = g_strdup_printf("%d", n->category);
		    break;
		default:
		    g_warning("Internal error: unhandled TCL TCLM_INT maping!\n");
	    }

	    Tcl_SetVar(tcl_interp, tclmap[i].name, h, 0);
	    g_free(h);
	  }
	  break;
	case TCLM_IGNORE:
	  break;
	default:
	  g_warning("Internal error: unkown TCL maping type!\n");
      }
    }

    Tcl_UnsetVar(tcl_interp, "updates", 0);
    Tcl_UnsetVar(tcl_interp, "installed", 0);
    Tcl_UnsetVar(tcl_interp, "extras", 0);
    Tcl_UnsetVar(tcl_interp, "holds", 0);
    GList *p = n->packages;
    while(p) {
      Tcl_SetVar2(tcl_interp, "installed", ((PkgNode *)(p->data))->package, ((PkgNode *)(p->data))->version, 0);

      if( ((PkgNode *)p->data)->flag & HOST_STATUS_PKGUPDATE)
        Tcl_SetVar2(tcl_interp, "updates", ((PkgNode *)(p->data))->package, ((PkgNode *)(p->data))->data, 0);

      if( ((PkgNode *)p->data)->flag & HOST_STATUS_PKGKEPTBACK)
        Tcl_SetVar2(tcl_interp, "holds", ((PkgNode *)(p->data))->package, ((PkgNode *)(p->data))->data, 0);

      if( ((PkgNode *)p->data)->flag & HOST_STATUS_PKGEXTRA)
        Tcl_SetVar2(tcl_interp, "extras", ((PkgNode *)(p->data))->package, ((PkgNode *)(p->data))->version, 0);
	
      p = g_list_next(p);
    }
    
    Tcl_UnsetVar(tcl_interp, "flags", 0);
    for(i=0; hostFlags[i].code; i++) {
      if(n->status & hostFlags[i].flag)
        Tcl_SetVar2(tcl_interp, "flags", hostFlags[i].code, hostFlags[i].code, 0);
    }

    Tcl_ResetResult(tcl_interp);
    tcl_interp->errorLine = 0;
    Tcl_Eval(tcl_interp, filterexp);
    if(tcl_interp->errorLine)
     filtered = FALSE;
    else
     filtered = atoi(tcl_interp->result) > 0;
     
    if(filtered != n->filtered) {
      n->filtered = filtered;
      rebuilddl = TRUE;
    }

    l = g_list_next(l);
 }
}

static void filterHosts(GList *hosts)
{
 gint i;
 gint first;
 WINDOW *w = newwin(LINES-3, COLS, 2, 0);

 wattron(w, uicolors[UI_COLOR_QUERY]);
 mvwaddstr(w, 1, 0, "Scalars:");
 wattroff(w, uicolors[UI_COLOR_QUERY]);

 waddstr(w, "\n");

 first = 1;
 for(i=0; tclmap[i].name; i++) {
   if((tclmap[i].type != TCLM_STRING) &&
      (tclmap[i].type != TCLM_INT))
     continue;

   if(!first)
    waddstr(w, ", ");
   else
    first = 0;
    
   waddstr(w, tclmap[i].name);
 }
  
 waddstr(w, "\n\n");
 
 wattron(w, uicolors[UI_COLOR_QUERY]);
 waddstr(w, "Arrays:");
 wattroff(w, uicolors[UI_COLOR_QUERY]);
 
 waddstr(w, "\n");

 first = 1;
 for(i=0; tclmap[i].name; i++) {
   if(tclmap[i].type != TCLM_IGNORE)
     continue;

   if(!first)
    waddstr(w, ", ");
   else
    first = 0;
    
   waddstr(w, tclmap[i].name);
 }
 
 waddstr(w, "\n\n");

 wattron(w, uicolors[UI_COLOR_QUERY]);
 waddstr(w, "Examples:");
 wattroff(w, uicolors[UI_COLOR_QUERY]);
 
 waddstr(w, "\n");

 waddstr(w, "return [expr [string compare $lsb_distri \"Debian\"] == 0]\n");
 waddstr(w, "return [expr [string compare $lsb_distri \"Debian\"] == 0 && $lsb_rel < 4.0]\n");
 waddstr(w, "return [llength [array names installed \"bind*\"]]\n");

 waddstr(w, "\n");

 wattron(w, uicolors[UI_COLOR_QUERY]);
 waddstr(w, "Enter filter expression:");
 wattroff(w, uicolors[UI_COLOR_QUERY]);
 waddstr(w, "\n");
   
 enableInput();
 wattron(w, uicolors[UI_COLOR_INPUT]);

 for(i = strlen(filterexp)-1; i>=0; i--)
   ungetch(filterexp[i]);
   
 wgetnstr(w, filterexp, sizeof(filterexp));
 disableInput();

 wattroff(w, uicolors[UI_COLOR_INPUT]);

 delwin(w);
 refreshDraw();

 applyFilter(hosts);
}
#endif


gboolean ctrlUI (GList *hosts)
{
 gint       hostcnt;
 int        ic, i;
 gboolean   ret = TRUE;
 gboolean   retqry = FALSE;
 gboolean   refscr = FALSE;
 DrawNode   *n;
 static     gchar in[BUF_MAX_LEN];
 gchar      *qrystr = NULL;
 gchar      *pkg = NULL;
 EShortCuts sc = SC_MAX;

 if(rebuilddl == TRUE) {
  rebuildDrawList(hosts);
  refscr = TRUE;
 }

 if((ic = getch()) == -1)
  refresh();
 else 
  for(i = 0; shortCuts[i].keycode; i++) 
   if(shortCuts[i].keycode == tolower(ic)) sc = shortCuts[i].sc;

#ifdef KEY_RESIZE
 if(ic == KEY_RESIZE) refscr = TRUE;
#endif

 if(sc != SC_MAX) {
  switch(sc) {

  case SC_KEY_HOME:
  case SC_KEY_END:
  case SC_KEY_UP:
  case SC_KEY_DOWN:
   refscr = ctrlKeyUpDown(ic);
   break; /* case SC_KEY_DOWN */

  case SC_KEY_PPAGE:
   refscr = ctrlKeyPgUp();
   break; /* case SC_KEY_PPAGE */

  case SC_KEY_NPAGE:
   refscr = ctrlKeyPgDown();
   break; /* case SC_KEY_NPAGE */

  case SC_KEY_LEFT:
   refscr = ctrlKeyLeft(hosts);
   break; /* case SC_KEY_LEFT */

  case SC_KEY_RIGHT:
   refscr = ctrlKeyRight(hosts);
   break; /* case SC_KEY_RIGHT */

  case SC_KEY_PLUS:
  case SC_KEY_SPACE:
  case SC_KEY_RETURN:
   refscr = ctrlKeyEnter(hosts);
   break; /* case SC_KEY_RETURN */

  case SC_KEY_REFRESH:
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
       setHostsCategory(hosts, getCategoryNumber(incategory), NULL, 
			(Category) C_REFRESH_REQUIRED);
       rebuildDrawList(hosts);
       refscr = TRUE;
      }
     }
   break; /* case SC_KEY_REFRESH */

  case SC_KEY_CONNECT:
   n = getSelectedDrawNode();
   if(!n) break;
   if(n->type == HOST) {
    if(n->extended == TRUE) n->extended = FALSE;

    if (g_list_length(inhost->screens)) {
     if (!queryConfirm("There are running sessions on this host! Continue? [y/N]: ",
		       FALSE))
      break;
    }

    dump_screen = FALSE;
    cleanUI();
    ssh_connect((HostNode *) n->p, FALSE);
    ((HostNode *) n->p)->category = C_REFRESH_REQUIRED;
    rebuildDrawList(hosts);
    initUI();
    refscr = TRUE;
   }
   break; /* case SC_KEY_CONNECT */

  case SC_KEY_UPGRADE:
   n = getSelectedDrawNode();
   if(!n) break;
   switch(n->type) {
   case HOST:
    if(n->extended == TRUE) n->extended = FALSE;
    
    if (g_list_length(inhost->screens)) {
     if (!queryConfirm("There are running sessions on this host! Continue? [y/N]: ",
		       FALSE))
      break;
    }

    cleanUI();
    ssh_cmd_upgrade((HostNode *) n->p, FALSE);
    ((HostNode *) n->p)->category = C_REFRESH_REQUIRED;
    rebuildDrawList(hosts);
    initUI();
    refscr = TRUE;
    break;
   case CATEGORY:
   case GROUP:
   default:
    {
     if(((n->type == CATEGORY) && !queryConfirm("Run update for the whole category? [y/N]: ", FALSE)) ||
	((n->type == GROUP) && !queryConfirm("Run update for the whole group? [y/N]: ", FALSE)))
      break;

     GList *ho = g_list_first(hosts);
     gint cat = getCategoryNumber(incategory);

     while(ho) {
      HostNode *m = (HostNode *)ho->data;
      if(((n->type == GROUP) && (strcmp(m->group, ingroup) == 0)) ||
	 ((n->type == CATEGORY) && (m->category == cat)))
       ssh_cmd_upgrade(m, TRUE);

      ho = g_list_next(ho);
     }
    }
    break;
   }
   break; /* case SC_KEY_UPGRADE */

  case SC_KEY_INSTALL:
   n = getSelectedDrawNode();
   if(!n) break;
   switch(n->type) {
   case PKG:
    if(n->p) {
     pkg = ((PkgNode *) n->p)->package;
    
     qrystr = g_strdup_printf("Install package `%s' [y/N]: ", pkg);
     if(!qrystr) break;
     retqry = queryConfirm(qrystr, FALSE);
     g_free(qrystr);
     qrystr = NULL;
     
     if(retqry == TRUE) {
      cleanUI();
      ssh_cmd_install(inhost, pkg, FALSE);
      inhost->category = C_REFRESH_REQUIRED;
      rebuildDrawList(hosts);
      initUI();
      refscr = TRUE;
     }
    }
    break;
   case HOST:
    if(n->extended == TRUE) n->extended = FALSE;

    if (g_list_length(inhost->screens)) {
     if (!queryConfirm("There are running sessions on this host! Continue? [y/N]: ", 
		       FALSE))
      break;
    }
  
    queryString("Install package: ", in, sizeof(in));
    if (strlen(in)==0)
     break;
    pkg = in;

    cleanUI();
    ssh_cmd_install(inhost, pkg, FALSE);
    inhost->category = C_REFRESH_REQUIRED;
    rebuildDrawList(hosts);
    initUI();
    refscr = TRUE;
    break;
   case CATEGORY:
   case GROUP:
   default:
    {
     queryString("Install package: ", in, sizeof(in));
     if (strlen(in)==0)
      break;

     if(((n->type == CATEGORY) && !queryConfirm("Run install for the whole category? [y/N]: ", FALSE)) ||
	((n->type == GROUP) && !queryConfirm("Run install for the whole group? [y/N]: ", FALSE)))
      break;

     GList *ho = g_list_first(hosts);
     gint cat = getCategoryNumber(incategory);

     while(ho) {
      HostNode *m = (HostNode *)ho->data;
      if(((n->type == GROUP) && (strcmp(m->group, ingroup) == 0)) ||
	 ((n->type == CATEGORY) && (m->category == cat)))
       ssh_cmd_install(m, in, TRUE);

      ho = g_list_next(ho);
     }
    }
    break;
   }
   break; /* case SC_KEY_INSTALL */

  case SC_KEY_NEXTSESS:
   {
    GList *ho = g_list_first(hosts);

    while(ho) {
     HostNode *m = (HostNode *)ho->data;

     GList *sc = m->screens;

     while(sc) {
      qrystr = g_strdup_printf("Attach host %s session %d [Y/n]: ", 
			       m->hostname, ((SessNode *)sc->data)->pid);
      if(!qrystr) {
       g_warning("Memory allocation failed!");
       break;
      }

      retqry = queryConfirm(qrystr, TRUE);
      g_free(qrystr);
      qrystr = NULL;

      if(retqry == FALSE) {
       sc = g_list_next(sc);
       break;
      }

      if (!screen_is_attached((SessNode *)sc->data)) {
       cleanUI();
       screen_attach((SessNode *)sc->data, FALSE);
       initUI();

       if(ic != 'N') {
	ho = NULL;
	break;
       }

      }
      sc = g_list_next(sc);
 
     }

     if(ho)
      ho = g_list_next(ho);
    }
   }
   break; /* case SC_KEY_NEXTSESS */

  case SC_KEY_ATTACH:
   n = getSelectedDrawNode();
   if(!inhost) break;

   SessNode *s = NULL;
  
   if(!n || (n->type != SESSION)) {
    GList *l = g_list_first(inhost->screens);
    if (l)
     s = l->data;
   }
   else {
    s = (SessNode *) n->p;
   }

   if(!s)
    break;

   {
    gboolean may_share = FALSE;

    /* Session already attached! */
    if (screen_is_attached(s)) {
     if (!queryConfirm("Already attached - share session? [y/N]: ", FALSE))
      break;
      
     may_share = TRUE;
    }

    dump_screen = FALSE;
    cleanUI();
    screen_attach(s, may_share);
    initUI();
   }
   refscr = TRUE;
   break; /* case SC_KEY_ATTACH */

  case SC_KEY_KILLSESS:
   if(ic != 'K') break;
   n = getSelectedDrawNode();
   if(!inhost) break;
   if(n->type != SESSION) break;

   /* Session already attached! */
   if (screen_is_attached((SessNode *) n->p))
    break;
  
   if (!queryConfirm("Realy kill this session? [y/N]: ", FALSE))
    break;
  
   kill(((SessNode *) n->p)->pid, SIGTERM);
   break; /* case SC_KEY_KILLSESS */

  case SC_KEY_TOGGLEDUMPS:
   if (!cfg->dump_screen)
    break;
   dump_screen = !dump_screen;

   if(getSelectedDrawNode()->type != SESSION) {
    if(dump_screen)
     mvaddstr(LINES - 1, 0, "Session dumps enabled.");
    else
     mvaddstr(LINES - 1, 0, "Session dumps disabled.");
   }
   else
    refscr = TRUE;

   break; /* case SC_KEY_TOGGLEDUMPS */

  case SC_KEY_HELP:
   {
    WINDOW *wp = newpad(BUF_MAX_LEN+SC_MAX, COLS);
    gint l = 0;
    int  wic = 0, pminrow = 0, kcquit = 'q';
    
    keypad(wp, TRUE);

    wattron(wp, A_BOLD);
    mvwaddnstr(wp, l  ,  2, "FLAG"       , COLS - 2);
    mvwaddnstr(wp, l++, 16, "DESCRIPTION", COLS - 16);
    wattroff(wp, A_BOLD);

    gint i = -1;
    while(hostFlags[++i].flag) {
     mvwaddch  (wp, l,  2, hostFlags[i].code[0]);
     mvwaddnstr(wp, l, 16, hostFlags[i].descr, COLS - 16);
     l++;
    }
    l++;
     
    wattron(wp, A_BOLD);
    mvwaddnstr(wp, l  ,  2, "KEY"        , COLS - 2);
    mvwaddnstr(wp, l++, 16, "DESCRIPTION", COLS - 16);
    wattroff(wp, A_BOLD);

    i = -1;
    while(shortCuts[++i].key) {
     mvwaddnstr(wp, l,  2, shortCuts[i].key  , COLS - 2);
     mvwaddnstr(wp, l, 16, shortCuts[i].descr, COLS - 16);
       
     l++;
    }


    for(i = 0; shortCuts[i].key; i++)
     if(shortCuts[i].sc == SC_KEY_QUIT) kcquit = shortCuts[i].keycode;

    pminrow = 0;
    prefresh(wp, pminrow, 0, 1, 0, LINES-3, COLS);
    while((wic = tolower(wgetch(wp))) != kcquit) {
     if(wic == KEY_UP && pminrow)
      pminrow--;
     else if(wic == KEY_DOWN && pminrow < l-LINES+4)
      pminrow++;
     else if(wic == KEY_HOME)
      pminrow=0;
     else if(wic == KEY_END)
      pminrow=l-LINES+4;
#ifdef KEY_RESIZE     
     else if(wic == KEY_RESIZE) {
      refscr = TRUE;
      break;
     }
#endif
     prefresh(wp, pminrow, 0, 1, 0, LINES-3, COLS);
    }
   
    delwin(wp);
    refscr = TRUE;
   }
   break; /* case SC_KEY_HELP */

  case SC_KEY_FIND:
   searchEntry(hosts);
   break;

#ifdef FEAT_TCLFILTER
  case SC_KEY_FILTER:
   filterHosts(hosts);

   rebuildDrawList(hosts);
   refscr = TRUE;
   break;
#endif

  case SC_KEY_FILETRANS:
   n = getSelectedDrawNode();
   if(!n) break;
   if(n->type == HOST) {
    if(n->extended == TRUE) n->extended = FALSE;

    cleanUI();
    sftp_connect((HostNode *) n->p);
    initUI();
    refscr = TRUE;
   }
   break; /* case SC_KEY_FILETRANS */

  case SC_KEY_QUIT:

   hostcnt = 0;
   GList *ho = g_list_first(hosts);

   while(ho) {
    HostNode *m = (HostNode *)ho->data;
    if(m->category == C_REFRESH_REQUIRED || m->category == C_REFRESH)
     hostcnt++;

    ho = g_list_next(ho);
   }

   if(hostcnt > 0) {
    qrystr = g_strdup_printf("There are %d %s in status refresh state,\
 quit apt-dater? [y/N]: ", hostcnt, hostcnt > 1 ? "hosts" : "host");

    retqry = queryConfirm(qrystr, FALSE);
    g_free(qrystr);

    if (retqry == FALSE)
     break;
   }

   ret = FALSE;
   attrset(A_NORMAL);
   cleanUI();
   refreshUI();
   refscr = FALSE;
   g_main_loop_quit (loop);
   break; /* case SC_KEY_QUIT */

  default:
   break;
  } /* switch (sc) */
 }


 if(refscr == TRUE) {
  getOldestMtime(hosts);
  refreshDraw();
 }

 return (ret);
}
