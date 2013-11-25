//========================================================================
//
// gfile.cc
//
// Miscellaneous file and directory name manipulation.
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2008 Adam Batkin <adam@batkin.net>
// Copyright (C) 2008, 2010, 2012, 2013 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2009, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2013 Adam Reichold <adamreichold@myopera.com>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Peter Breitenlohner <peb@mppmu.mpg.de>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef _WIN32
#  include <time.h>
#else
#  if defined(MACOS)
#    include <sys/stat.h>
#  elif !defined(ACORN)
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#  endif
#  include <time.h>
#  include <limits.h>
#  include <string.h>
#  if !defined(VMS) && !defined(ACORN) && !defined(MACOS)
#    include <pwd.h>
#  endif
#  if defined(VMS) && (__DECCXX_VER < 50200000)
#    include <unixlib.h>
#  endif
#endif // _WIN32
#include <stdio.h>
#include <limits>
#include "GooString.h"
#include "gfile.h"

// Some systems don't define this, so just make it something reasonably
// large.
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

//------------------------------------------------------------------------

GooString *getCurrentDir() {
  char buf[PATH_MAX+1];

#if defined(__EMX__)
  if (_getcwd2(buf, sizeof(buf)))
#elif defined(_WIN32)
  if (GetCurrentDirectory(sizeof(buf), buf))
#elif defined(ACORN)
  if (strcpy(buf, "@"))
#elif defined(MACOS)
  if (strcpy(buf, ":"))
#else
  if (getcwd(buf, sizeof(buf)))
#endif
    return new GooString(buf);
  return new GooString();
}

GooString *appendToPath(GooString *path, const char *fileName) {
#if defined(VMS)
  //---------- VMS ----------
  //~ this should handle everything necessary for file
  //~ requesters, but it's certainly not complete
  char *p0, *p1, *p2;
  char *q1;

  p0 = path->getCString();
  p1 = p0 + path->getLength() - 1;
  if (!strcmp(fileName, "-")) {
    if (*p1 == ']') {
      for (p2 = p1; p2 > p0 && *p2 != '.' && *p2 != '['; --p2) ;
      if (*p2 == '[')
	++p2;
      path->del(p2 - p0, p1 - p2);
    } else if (*p1 == ':') {
      path->append("[-]");
    } else {
      path->clear();
      path->append("[-]");
    }
  } else if ((q1 = strrchr(fileName, '.')) && !strncmp(q1, ".DIR;", 5)) {
    if (*p1 == ']') {
      path->insert(p1 - p0, '.');
      path->insert(p1 - p0 + 1, fileName, q1 - fileName);
    } else if (*p1 == ':') {
      path->append('[');
      path->append(']');
      path->append(fileName, q1 - fileName);
    } else {
      path->clear();
      path->append(fileName, q1 - fileName);
    }
  } else {
    if (*p1 != ']' && *p1 != ':')
      path->clear();
    path->append(fileName);
  }
  return path;

#elif defined(_WIN32)
  //---------- Win32 ----------
  GooString *tmp;
  char buf[256];
  char *fp;

  tmp = new GooString(path);
  tmp->append('/');
  tmp->append(fileName);
  GetFullPathName(tmp->getCString(), sizeof(buf), buf, &fp);
  delete tmp;
  path->clear();
  path->append(buf);
  return path;

#elif defined(ACORN)
  //---------- RISCOS ----------
  char *p;
  int i;

  path->append(".");
  i = path->getLength();
  path->append(fileName);
  for (p = path->getCString() + i; *p; ++p) {
    if (*p == '/') {
      *p = '.';
    } else if (*p == '.') {
      *p = '/';
    }
  }
  return path;

#elif defined(MACOS)
  //---------- MacOS ----------
  char *p;
  int i;

  path->append(":");
  i = path->getLength();
  path->append(fileName);
  for (p = path->getCString() + i; *p; ++p) {
    if (*p == '/') {
      *p = ':';
    } else if (*p == '.') {
      *p = ':';
    }
  }
  return path;

#elif defined(__EMX__)
  //---------- OS/2+EMX ----------
  int i;

  // appending "." does nothing
  if (!strcmp(fileName, "."))
    return path;

  // appending ".." goes up one directory
  if (!strcmp(fileName, "..")) {
    for (i = path->getLength() - 2; i >= 0; --i) {
      if (path->getChar(i) == '/' || path->getChar(i) == '\\' ||
	  path->getChar(i) == ':')
	break;
    }
    if (i <= 0) {
      if (path->getChar(0) == '/' || path->getChar(0) == '\\') {
	path->del(1, path->getLength() - 1);
      } else if (path->getLength() >= 2 && path->getChar(1) == ':') {
	path->del(2, path->getLength() - 2);
      } else {
	path->clear();
	path->append("..");
      }
    } else {
      if (path->getChar(i-1) == ':')
	++i;
      path->del(i, path->getLength() - i);
    }
    return path;
  }

  // otherwise, append "/" and new path component
  if (path->getLength() > 0 &&
      path->getChar(path->getLength() - 1) != '/' &&
      path->getChar(path->getLength() - 1) != '\\')
    path->append('/');
  path->append(fileName);
  return path;

#else
  //---------- Unix ----------
  int i;

  // appending "." does nothing
  if (!strcmp(fileName, "."))
    return path;

  // appending ".." goes up one directory
  if (!strcmp(fileName, "..")) {
    for (i = path->getLength() - 2; i >= 0; --i) {
      if (path->getChar(i) == '/')
	break;
    }
    if (i <= 0) {
      if (path->getChar(0) == '/') {
	path->del(1, path->getLength() - 1);
      } else {
	path->clear();
	path->append("..");
      }
    } else {
      path->del(i, path->getLength() - i);
    }
    return path;
  }

  // otherwise, append "/" and new path component
  if (path->getLength() > 0 &&
      path->getChar(path->getLength() - 1) != '/')
    path->append('/');
  path->append(fileName);
  return path;
#endif
}

GooString *grabPath(char *fileName) {
#ifdef VMS
  //---------- VMS ----------
  char *p;

  if ((p = strrchr(fileName, ']')))
    return new GooString(fileName, p + 1 - fileName);
  if ((p = strrchr(fileName, ':')))
    return new GooString(fileName, p + 1 - fileName);
  return new GooString();

#elif defined(__EMX__) || defined(_WIN32)
  //---------- OS/2+EMX and Win32 ----------
  char *p;

  if ((p = strrchr(fileName, '/')))
    return new GooString(fileName, p - fileName);
  if ((p = strrchr(fileName, '\\')))
    return new GooString(fileName, p - fileName);
  if ((p = strrchr(fileName, ':')))
    return new GooString(fileName, p + 1 - fileName);
  return new GooString();

#elif defined(ACORN)
  //---------- RISCOS ----------
  char *p;

  if ((p = strrchr(fileName, '.')))
    return new GooString(fileName, p - fileName);
  return new GooString();

#elif defined(MACOS)
  //---------- MacOS ----------
  char *p;

  if ((p = strrchr(fileName, ':')))
    return new GooString(fileName, p - fileName);
  return new GooString();

#else
  //---------- Unix ----------
  char *p;

  if ((p = strrchr(fileName, '/')))
    return new GooString(fileName, p - fileName);
  return new GooString();
#endif
}

GBool isAbsolutePath(char *path) {
#ifdef VMS
  //---------- VMS ----------
  return strchr(path, ':') ||
	 (path[0] == '[' && path[1] != '.' && path[1] != '-');

#elif defined(__EMX__) || defined(_WIN32)
  //---------- OS/2+EMX and Win32 ----------
  return path[0] == '/' || path[0] == '\\' || path[1] == ':';

#elif defined(ACORN)
  //---------- RISCOS ----------
  return path[0] == '$';

#elif defined(MACOS)
  //---------- MacOS ----------
  return path[0] != ':';

#else
  //---------- Unix ----------
  return path[0] == '/';
#endif
}

time_t getModTime(char *fileName) {
#ifdef _WIN32
  //~ should implement this, but it's (currently) only used in xpdf
  return 0;
#else
  struct stat statBuf;

  if (stat(fileName, &statBuf)) {
    return 0;
  }
  return statBuf.st_mtime;
#endif
}

GBool openTempFile(GooString **name, FILE **f, const char *mode) {
#if defined(_WIN32)
  //---------- Win32 ----------
  char *tempDir;
  GooString *s, *s2;
  FILE *f2;
  int t, i;

  // this has the standard race condition problem, but I haven't found
  // a better way to generate temp file names with extensions on
  // Windows
  if ((tempDir = getenv("TEMP"))) {
    s = new GooString(tempDir);
    s->append('\\');
  } else {
    s = new GooString();
  }
  s->appendf("x_{0:d}_{1:d}_",
	     (int)GetCurrentProcessId(), (int)GetCurrentThreadId());
  t = (int)time(NULL);
  for (i = 0; i < 1000; ++i) {
    s2 = s->copy()->appendf("{0:d}", t + i);
    if (!(f2 = fopen(s2->getCString(), "r"))) {
      if (!(f2 = fopen(s2->getCString(), mode))) {
	delete s2;
	delete s;
        return gFalse;
      }
      *name = s2;
      *f = f2;
      delete s;
      return gTrue;
    }
    fclose(f2);
    delete s2;
  }
  delete s;
  return gFalse;
#elif defined(VMS) || defined(__EMX__) || defined(ACORN) || defined(MACOS)
  //---------- non-Unix ----------
  char *s;

  // There is a security hole here: an attacker can create a symlink
  // with this file name after the tmpnam call and before the fopen
  // call.  I will happily accept fixes to this function for non-Unix
  // OSs.
  if (!(s = tmpnam(NULL))) {
    return gFalse;
  }
  *name = new GooString(s);
  if (!(*f = fopen((*name)->getCString(), mode))) {
    delete (*name);
    *name = NULL;
    return gFalse;
  }
  return gTrue;
#else
  //---------- Unix ----------
  char *s;
  int fd;

#if HAVE_MKSTEMP
  if ((s = getenv("TMPDIR"))) {
    *name = new GooString(s);
  } else {
    *name = new GooString("/tmp");
  }
  (*name)->append("/XXXXXX");
  fd = mkstemp((*name)->getCString());
#else // HAVE_MKSTEMP
  if (!(s = tmpnam(NULL))) {
    return gFalse;
  }
  *name = new GooString(s);
  fd = open((*name)->getCString(), O_WRONLY | O_CREAT | O_EXCL, 0600);
#endif // HAVE_MKSTEMP
  if (fd < 0 || !(*f = fdopen(fd, mode))) {
    delete *name;
    *name = NULL;
    return gFalse;
  }
  return gTrue;
#endif
}

#ifdef WIN32
GooString *fileNameToUTF8(char *path) {
  GooString *s;
  char *p;

  s = new GooString();
  for (p = path; *p; ++p) {
    if (*p & 0x80) {
      s->append((char)(0xc0 | ((*p >> 6) & 0x03)));
      s->append((char)(0x80 | (*p & 0x3f)));
    } else {
      s->append(*p);
    }
  }
  return s;
}

GooString *fileNameToUTF8(wchar_t *path) {
  GooString *s;
  wchar_t *p;

  s = new GooString();
  for (p = path; *p; ++p) {
    if (*p < 0x80) {
      s->append((char)*p);
    } else if (*p < 0x800) {
      s->append((char)(0xc0 | ((*p >> 6) & 0x1f)));
      s->append((char)(0x80 | (*p & 0x3f)));
    } else {
      s->append((char)(0xe0 | ((*p >> 12) & 0x0f)));
      s->append((char)(0x80 | ((*p >> 6) & 0x3f)));
      s->append((char)(0x80 | (*p & 0x3f)));
    }
  }
  return s;
}
#endif

FILE *openFile(const char *path, const char *mode) {
#ifdef WIN32
  OSVERSIONINFO version;
  wchar_t wPath[_MAX_PATH + 1];
  char nPath[_MAX_PATH + 1];
  wchar_t wMode[8];
  const char *p;
  size_t i;

  // NB: _wfopen is only available in NT
  version.dwOSVersionInfoSize = sizeof(version);
  GetVersionEx(&version);
  if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
    for (p = path, i = 0; *p && i < _MAX_PATH; ++i) {
      if ((p[0] & 0xe0) == 0xc0 &&
	  p[1] && (p[1] & 0xc0) == 0x80) {
	wPath[i] = (wchar_t)(((p[0] & 0x1f) << 6) |
			     (p[1] & 0x3f));
	p += 2;
      } else if ((p[0] & 0xf0) == 0xe0 &&
		 p[1] && (p[1] & 0xc0) == 0x80 &&
		 p[2] && (p[2] & 0xc0) == 0x80) {
	wPath[i] = (wchar_t)(((p[0] & 0x0f) << 12) |
			     ((p[1] & 0x3f) << 6) |
			     (p[2] & 0x3f));
	p += 3;
      } else {
	wPath[i] = (wchar_t)(p[0] & 0xff);
	p += 1;
      }
    }
    wPath[i] = (wchar_t)0;
    for (i = 0; mode[i] && i < sizeof(mode) - 1; ++i) {
      wMode[i] = (wchar_t)(mode[i] & 0xff);
    }
    wMode[i] = (wchar_t)0;
    return _wfopen(wPath, wMode);
  } else {
    for (p = path, i = 0; *p && i < _MAX_PATH; ++i) {
      if ((p[0] & 0xe0) == 0xc0 &&
	  p[1] && (p[1] & 0xc0) == 0x80) {
	nPath[i] = (char)(((p[0] & 0x1f) << 6) |
			  (p[1] & 0x3f));
	p += 2;
      } else if ((p[0] & 0xf0) == 0xe0 &&
		 p[1] && (p[1] & 0xc0) == 0x80 &&
		 p[2] && (p[2] & 0xc0) == 0x80) {
	nPath[i] = (char)(((p[1] & 0x3f) << 6) |
			  (p[2] & 0x3f));
	p += 3;
      } else {
	nPath[i] = p[0];
	p += 1;
      }
    }
    nPath[i] = '\0';
    return fopen(nPath, mode);
  }
#else
  return fopen(path, mode);
#endif
}

char *getLine(char *buf, int size, FILE *f) {
  int c, i;

  i = 0;
  while (i < size - 1) {
    if ((c = fgetc(f)) == EOF) {
      break;
    }
    buf[i++] = (char)c;
    if (c == '\x0a') {
      break;
    }
    if (c == '\x0d') {
      c = fgetc(f);
      if (c == '\x0a' && i < size - 1) {
	buf[i++] = (char)c;
      } else if (c != EOF) {
	ungetc(c, f);
      }
      break;
    }
  }
  buf[i] = '\0';
  if (i == 0) {
    return NULL;
  }
  return buf;
}

int Gfseek(FILE *f, Goffset offset, int whence) {
#if HAVE_FSEEKO
  return fseeko(f, offset, whence);
#elif HAVE_FSEEK64
  return fseek64(f, offset, whence);
#elif defined(__MINGW32__)
  return fseeko64(f, offset, whence);
#elif _WIN32
  return _fseeki64(f, offset, whence);
#else
  return fseek(f, offset, whence);
#endif
}

Goffset Gftell(FILE *f) {
#if HAVE_FSEEKO
  return ftello(f);
#elif HAVE_FSEEK64
  return ftell64(f);
#elif defined(__MINGW32__)
  return ftello64(f);
#elif _WIN32
  return _ftelli64(f);
#else
  return ftell(f);
#endif
}

Goffset GoffsetMax() {
#if HAVE_FSEEKO
  return (std::numeric_limits<off_t>::max)();
#elif HAVE_FSEEK64 || defined(__MINGW32__)
  return (std::numeric_limits<off64_t>::max)();
#elif _WIN32
  return (std::numeric_limits<__int64>::max)();
#else
  return (std::numeric_limits<long>::max)();
#endif
}

//------------------------------------------------------------------------
// GooFile
//------------------------------------------------------------------------

#ifdef _WIN32

int GooFile::read(char *buf, int n, Goffset offset) const {
  DWORD m;
  
  LARGE_INTEGER largeInteger = {0};
  largeInteger.QuadPart = offset;
  
  OVERLAPPED overlapped = {0};
  overlapped.Offset = largeInteger.LowPart;
  overlapped.OffsetHigh = largeInteger.HighPart;

  return FALSE == ReadFile(handle, buf, n, &m, &overlapped) ? -1 : m;
}

Goffset GooFile::size() const {
  LARGE_INTEGER size = {(DWORD)-1,-1};
  
  GetFileSizeEx(handle, &size);

  return size.QuadPart;
}

GooFile* GooFile::open(const GooString *fileName) {
  HANDLE handle = CreateFile(fileName->getCString(),
                              GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, NULL);
  
  return handle == INVALID_HANDLE_VALUE ? NULL : new GooFile(handle);
}

GooFile* GooFile::open(const wchar_t *fileName) {
  HANDLE handle = CreateFileW(fileName,
                              GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, NULL);
  
  return handle == INVALID_HANDLE_VALUE ? NULL : new GooFile(handle);
}

#else

int GooFile::read(char *buf, int n, Goffset offset) const {
#ifdef HAVE_PREAD64
  return pread64(fd, buf, n, offset);
#else
  return pread(fd, buf, n, offset);
#endif
}

Goffset GooFile::size() const {
#ifdef HAVE_LSEEK64
  return lseek64(fd, 0, SEEK_END);
#else
  return lseek(fd, 0, SEEK_END);
#endif
}

GooFile* GooFile::open(const GooString *fileName) {
#ifdef VMS
  int fd = ::open(fileName->getCString(), Q_RDONLY, "ctx=stm");
#else
  int fd = ::open(fileName->getCString(), O_RDONLY);
#endif
  
  return fd < 0 ? NULL : new GooFile(fd);
}

#endif // _WIN32

//------------------------------------------------------------------------
// GDir and GDirEntry
//------------------------------------------------------------------------

GDirEntry::GDirEntry(char *dirPath, char *nameA, GBool doStat) {
#ifdef VMS
  char *p;
#elif defined(_WIN32)
  DWORD fa;
#elif defined(ACORN)
#else
  struct stat st;
#endif

  name = new GooString(nameA);
  dir = gFalse;
  fullPath = new GooString(dirPath);
  appendToPath(fullPath, nameA);
  if (doStat) {
#ifdef VMS
    if (!strcmp(nameA, "-") ||
	((p = strrchr(nameA, '.')) && !strncmp(p, ".DIR;", 5)))
      dir = gTrue;
#elif defined(ACORN)
#else
#ifdef _WIN32
    fa = GetFileAttributes(fullPath->getCString());
    dir = (fa != 0xFFFFFFFF && (fa & FILE_ATTRIBUTE_DIRECTORY));
#else
    if (stat(fullPath->getCString(), &st) == 0)
      dir = S_ISDIR(st.st_mode);
#endif
#endif
  }
}

GDirEntry::~GDirEntry() {
  delete fullPath;
  delete name;
}

GDir::GDir(char *name, GBool doStatA) {
  path = new GooString(name);
  doStat = doStatA;
#if defined(_WIN32)
  GooString *tmp;

  tmp = path->copy();
  tmp->append("/*.*");
  hnd = FindFirstFile(tmp->getCString(), &ffd);
  delete tmp;
#elif defined(ACORN)
#elif defined(MACOS)
#else
  dir = opendir(name);
#ifdef VMS
  needParent = strchr(name, '[') != NULL;
#endif
#endif
}

GDir::~GDir() {
  delete path;
#if defined(_WIN32)
  if (hnd != INVALID_HANDLE_VALUE) {
    FindClose(hnd);
    hnd = INVALID_HANDLE_VALUE;
  }
#elif defined(ACORN)
#elif defined(MACOS)
#else
  if (dir)
    closedir(dir);
#endif
}

GDirEntry *GDir::getNextEntry() {
  GDirEntry *e;

#if defined(_WIN32)
  if (hnd != INVALID_HANDLE_VALUE) {
    e = new GDirEntry(path->getCString(), ffd.cFileName, doStat);
    if (!FindNextFile(hnd, &ffd)) {
      FindClose(hnd);
      hnd = INVALID_HANDLE_VALUE;
    }
  } else {
    e = NULL;
  }
#elif defined(ACORN)
#elif defined(MACOS)
#elif defined(VMS)
  struct dirent *ent;
  e = NULL;
  if (dir) {
    if (needParent) {
      e = new GDirEntry(path->getCString(), "-", doStat);
      needParent = gFalse;
      return e;
    }
    ent = readdir(dir);
    if (ent) {
      e = new GDirEntry(path->getCString(), ent->d_name, doStat);
    }
  }
#else
  struct dirent *ent;
  e = NULL;
  if (dir) {
    do {
      ent = readdir(dir);
    }
    while (ent && (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")));
    if (ent) {
      e = new GDirEntry(path->getCString(), ent->d_name, doStat);
    }
  }
#endif

  return e;
}

void GDir::rewind() {
#ifdef _WIN32
  GooString *tmp;

  if (hnd != INVALID_HANDLE_VALUE)
    FindClose(hnd);
  tmp = path->copy();
  tmp->append("/*.*");
  hnd = FindFirstFile(tmp->getCString(), &ffd);
  delete tmp;
#elif defined(ACORN)
#elif defined(MACOS)
#else
  if (dir)
    rewinddir(dir);
#ifdef VMS
  needParent = strchr(path->getCString(), '[') != NULL;
#endif
#endif
}
