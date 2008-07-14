
#ifndef _KEYFILES_H
#define _KEYFILES_H

GList *loadHosts (char *filename);
CfgFile *loadConfig (char *filename);
void freeConfig (CfgFile *cfg);
int chkForInitialConfig(const gchar *, const gchar *);



static char apt_dater_conf[] = "# Config file of apt-dater in the form of the"
 " glib GKeyFile required\n\n[Paths]\n# Default: $XDG_CONFIG_HOME/apt-dater/h"
 "osts.conf\n#HostsFile=path-to/hosts.conf\n\n# Default: $XDG_DATA_HOME/apt-d"
 "ater\n#StatsDir=path-to/stats\n\n[SSH]\n# SSH binary\nCmd=/usr/bin/ssh\nOpt"
 "ionalCmdFlags=\n\n# SFTP binary\nSFTPCmd=/usr/bin/mc /#sh:%u@%h:C/\n\nDefau"
 "ltUser=apt-dater\nDefaultPort=22\n\n#[Screen]\n## Default: $XDG_CONFIG_HOME"
 "/apt-dater/screenrc\n#RCFile=path-to/screenrc\n#\n## Default: %m # %u@%h:%p"
 "\n#Title=%m # %u@%h:%p\n#\n\n# These values requires apt-dater-host to be i"
 "nstalled on the target host.\n# You could call apt/aptitude directly (see a"
 "pt-dater-host source),\n# but this is not recommended.\n[Commands]\nCmdRefr"
 "esh=apt-dater-host refresh\nCmdUpgrade=apt-dater-host upgrade\nCmdInstall=a"
 "pt-dater-host install %s\n\n[Appearance]\n# Colors      = (COMPONENT FG BG "
 "\';\')*\n# COMPONENT ::= \'default\' | \'menu\' | \'status\' | \'selector\'"
 " | \'hoststatus\' |\n#               \'query\' | \'input\'\n# FG        ::="
 " COLOR\n# BG        ::= COLOR\n# COLOR     ::= \'black\' | \'blue\' | \'cya"
 "n\' | \'green\' | \'magenta\' | \'red\' |\n#               \'white\' | \'ye"
 "llow\'\nColors=menu brightgreen blue;status brightgreen blue;selector black"
 " red;\n";

static char hosts_conf[] = "# Syntax:\n#\n#  [Customer Name]\n#  Hosts=([Opti"
 "onalUser@]host.domain[:OptionalPort];)*\n#\n\n[Localdomain]\nHosts=localhos"
 "t;\n\n[IBH]\nHosts=example1.ibh.net;example2.ibh.net;test@example3.ibh.net:"
 "62222;\n";

static char screenrc[] = "# -------------------------------------------------"
 "-----------------------------\n# SCREEN SETTINGS\n# -----------------------"
 "-------------------------------------------------------\n\nstartup_message "
 "off\n\n#defflow on # will force screen to process ^S/^Q\ndeflogin on\nautod"
 "etach on\n\n# turn visual bell on\nvbell on\n\n# define a bigger scrollback"
 ", default is 100 lines\ndefscrollback 2048\n\n# ---------------------------"
 "---------------------------------------------------\n# SCREEN KEYBINDINGS\n"
 "# -------------------------------------------------------------------------"
 "-----\n\n# Remove some stupid / dangerous key bindings\nbind ^k\n#bind L\nb"
 "ind ^\\\n# Make them better\nbind \\\\ quit\nbind K kill\nbind I login on\n"
 "bind O login off\nbind } history\n\n# Sessions should stay until destroyed "
 "by pressing space\nzombie \'x\'\n\n# --------------------------------------"
 "----------------------------------------\n# TERMINAL SETTINGS\n# ----------"
 "--------------------------------------------------------------------\n\n# T"
 "he vt100 description does not mention \"dl\". *sigh*\ntermcapinfo vt100 dl="
 "5\\E[M\n\n# Set the hardstatus prop on gui terms to set the titlebar/icon t"
 "itle\ntermcapinfo xterm*|rxvt*|kterm*|Eterm* hs:ts=\\E]0;:fs=\\007:ds=\\E]0"
 ";\\007:OP\n\n# set these terminals up to be \'optimal\' instead of vt100\n#"
 "termcapinfo xterm*|linux*|rxvt*|Eterm* OP\n\n# Change the xterm initializat"
 "ion string from is2=\\E[!p\\E[?3;4l\\E[4l\\E>\n# (This fixes the \"Aborted "
 "because of window size change\" konsole symptoms found\n#  in bug #134198)"
 "\ntermcapinfo xterm \'is=\\E[r\\E[m\\E[2J\\E[H\\E[?7h\\E[?1;4;6l\'\n\n# To "
 "get screen to add lines to xterm\'s scrollback buffer, uncomment the\n# fol"
 "lowing termcapinfo line which tells xterm to use the normal screen buffer\n"
 "# (which has scrollback), not the alternate screen buffer.\n#\ntermcapinfo "
 "xterm|xterms|xs|rxvt ti@:te@\n\n# Add caption line with clock, window title"
 " and window flags.\ncaption always \"%{b bG}%c%=%t%=%f\"\n\n# Catch zmodem "
 "file transfers\nzmodem catch\n";


#endif /* _KEYFILES_H */
