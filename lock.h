#ifndef _LOCK_H
#define _LOCK_H

int setLockForHost(const gchar *, FILE *);
int unsetLockForHost(const gchar *, FILE *);

gchar *getLockFile(const gchar *);

#endif /* _LOCK_H */
