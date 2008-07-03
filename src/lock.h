#ifndef _LOCK_H
#define _LOCK_H

int setLockForHost(HostNode *);
int unsetLockForHost(HostNode *);
void cleanupLocks();

#endif /* _LOCK_H */
