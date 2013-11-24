//========================================================================
//
// GooMutex.h
//
// Portable mutex macros.
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2013 Adam Reichold <adamreichold@myopera.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef GMUTEX_H
#define GMUTEX_H

// Usage:
//
// GooMutex m;
// gInitMutex(&m);
// ...
// gLockMutex(&m);
//   ... critical section ...
// gUnlockMutex(&m);
// ...
// gDestroyMutex(&m);

#ifdef _WIN32

#include <windows.h>

typedef CRITICAL_SECTION GooMutex;

#define gInitMutex(m) InitializeCriticalSection(m)
#define gDestroyMutex(m) DeleteCriticalSection(m)
#define gLockMutex(m) EnterCriticalSection(m)
#define gUnlockMutex(m) LeaveCriticalSection(m)

#else // assume pthreads

#include <pthread.h>

typedef pthread_mutex_t GooMutex;

inline void gInitMutex(GooMutex *m) {
  pthread_mutexattr_t mutexattr;
  pthread_mutexattr_init(&mutexattr);
  pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(m, &mutexattr);
  pthread_mutexattr_destroy(&mutexattr);
}
#define gDestroyMutex(m) pthread_mutex_destroy(m)
#define gLockMutex(m) pthread_mutex_lock(m)
#define gUnlockMutex(m) pthread_mutex_unlock(m)

#endif

class MutexLocker {
public:
  MutexLocker(GooMutex *mutexA) : mutex(mutexA) { gLockMutex(mutex); }
  ~MutexLocker() { gUnlockMutex(mutex); }

private:
  GooMutex *mutex;
};

#endif
