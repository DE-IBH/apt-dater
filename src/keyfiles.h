
#ifndef _KEYFILES_H
#define _KEYFILES_H

GList *loadHosts (char *filename);
CfgFile *loadConfig (char *filename);
void freeConfig (CfgFile *cfg);
int chkForInitialConfig(const gchar *, const gchar *);

#endif /* _KEYFILES_H */
