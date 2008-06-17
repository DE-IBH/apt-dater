
#ifndef _KEYFILES_H
#define _KEYFILES_H

GList *loadHosts (char *filename);
CfgFile *loadConfig (char *filename);
void freeConfig (CfgFile *cfg);

#endif /* _KEYFILES_H */
