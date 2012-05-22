/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Andre Ellguth <ellguth@ibh.de>
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2012 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
 *
 * License:
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this package; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "apt-dater.h"
#include "colors.h"
#include "ui.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

int uicolors[UI_COLOR_MAX];

#ifdef HAVE_COLOR

static struct mapping_t Colors[] =
 {
  { "black",          COLOR_BLACK },
  { "blue",           COLOR_BLUE },
  { "cyan",           COLOR_CYAN },
  { "green",          COLOR_GREEN },
  { "magenta",        COLOR_MAGENTA },
  { "red",            COLOR_RED },
  { "white",          COLOR_WHITE },
  { "yellow",         COLOR_YELLOW },
#ifdef HAVE_USE_DEFAULT_COLORS
  { "default",  COLOR_DEFAULT },
#endif
  { NULL, 0 },
 };


static struct mapping_t Components[] =
 {
  { "default", UI_COLOR_DEFAULT },
  { "menu", UI_COLOR_MENU },
  { "status", UI_COLOR_STATUS },
  { "selector", UI_COLOR_SELECTOR },
  { "hoststatus", UI_COLOR_HOSTSTATUS },
  { "query", UI_COLOR_QUERY },
  { "input", UI_COLOR_INPUT },
  { NULL, 0 },
 };


gboolean setColorForComponent(const gchar *component, const gchar *fg, 
			      const gchar *bg)
{
 int i, pos = 0;
 short setfg, setbg, setcomp;
 gboolean isbright;

 setfg = setbg = setcomp = -1;

 if((isbright = g_str_has_prefix (fg, PREFIX_COLOR_BRIGHT)) == TRUE)
  pos = strlen(PREFIX_COLOR_BRIGHT);

 for(i = 0; Components[i].name; i++) {
  if(!g_ascii_strcasecmp(Components[i].name, component)) {
   setcomp = i;
   break;
  }
 }

 for(i = 0; Colors[i].name; i++) {
  if(!g_ascii_strcasecmp(Colors[i].name, &fg[pos])) {
   setfg = i;
  }
  if(!g_ascii_strcasecmp(Colors[i].name, bg)) {
   setbg = i;
  }
 }

 if(setfg == -1 || setbg == -1 || setcomp == -1)
  return(FALSE);

 init_pair(setcomp+1, Colors[setfg].value, Colors[setbg].value);

 uicolors[setcomp] = COLOR_PAIR(setcomp+1) | ((isbright == TRUE) ? A_BOLD : 0);

 return(TRUE);
}


gboolean setColors(const gchar **colors)
{
 gint i, j;
 gchar **colordef;
 gboolean r = TRUE;

 if(has_colors() == FALSE) return(FALSE);

 i = 0;
 while(*(colors+i)) {
   if((colordef = g_strsplit(colors[i++], " ", 3))) {
   j = 0;
   while(*(colordef+j++));
   if(j == 4)
    r&= setColorForComponent(colordef[0], colordef[1], colordef[2]);
   
   g_strfreev(colordef);
  }
 }

 return(r);
}
#endif /* HAVE_COLOR */


void ui_start_color ()
{
 short fg, bg;

 memset (uicolors, A_NORMAL, sizeof (gint) * UI_COLOR_MAX);

 /* Some defaults */
 uicolors[UI_COLOR_DEFAULT] = A_NORMAL;
 uicolors[UI_COLOR_MENU] = A_REVERSE;
 uicolors[UI_COLOR_STATUS] = A_REVERSE;
 uicolors[UI_COLOR_SELECTOR] = A_REVERSE;
 uicolors[UI_COLOR_QUERY] = A_BOLD;
 uicolors[UI_COLOR_INPUT] = A_NORMAL;
 uicolors[UI_COLOR_HOSTSTATUS] = A_BOLD;

#ifdef HAVE_COLOR
 start_color();
 if(cfg->colors)
  if(setColors((const gchar **) cfg->colors) == FALSE)
   g_warning(_("Wrong color definition!"));

#ifdef HAVE_USE_DEFAULT_COLORS
 if(pair_content(UI_COLOR_DEFAULT+1, &fg, &bg) != ERR) {
  if(!fg && !bg) fg = bg = COLOR_DEFAULT;
  assume_default_colors(fg, bg);
 }
 else
  use_default_colors();
#endif /* HAVE_USE_DEFAULT_COLORS */
#endif /* HAVE_COLOR */
}
