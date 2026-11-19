#ifndef LOCK_H
#define LOCK_H

hidden void __lock(_Atomic int *);
hidden void __unlock(_Atomic int *);
#define LOCK(x) __lock(x)
#define UNLOCK(x) __unlock(x)

#endif
