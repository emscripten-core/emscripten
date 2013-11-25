//========================================================================
//
// gfile.h
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
// Copyright (C) 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2009, 2011, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2013 Adam Reichold <adamreichold@myopera.com>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef GFILE_H
#define GFILE_H

#include "poppler-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
extern "C" {
#if defined(_WIN32)
#  include <sys/stat.h>
#  ifdef FPTEX
#    include <win32lib.h>
#  else
#    include <windows.h>
#  endif
#elif defined(ACORN)
#elif defined(MACOS)
#  include <ctime.h>
#else
#  include <unistd.h>
#  include <sys/types.h>
#  ifdef VMS
#    include "vms_dirent.h"
#  elif HAVE_DIRENT_H
#    include <dirent.h>
#    define NAMLEN(d) strlen((d)->d_name)
#  else
#    define dirent direct
#    define NAMLEN(d) (d)->d_namlen
#    if HAVE_SYS_NDIR_H
#      include <sys/ndir.h>
#    endif
#    if HAVE_SYS_DIR_H
#      include <sys/dir.h>
#    endif
#    if HAVE_NDIR_H
#      include <ndir.h>
#    endif
#  endif
#endif
}
#include "gtypes.h"

class GooString;

//------------------------------------------------------------------------

// Get current directory.
extern GooString *getCurrentDir();

// Append a file name to a path string.  <path> may be an empty
// string, denoting the current directory).  Returns <path>.
extern GooString *appendToPath(GooString *path, const char *fileName);

// Grab the path from the front of the file name.  If there is no
// directory component in <fileName>, returns an empty string.
extern GooString *grabPath(char *fileName);

// Is this an absolute path or file name?
extern GBool isAbsolutePath(char *path);

// Get the modification time for <fileName>.  Returns 0 if there is an
// error.
extern time_t getModTime(char *fileName);

// Create a temporary file and open it for writing.  If <ext> is not
// NULL, it will be used as the file name extension.  Returns both the
// name and the file pointer.  For security reasons, all writing
// should be done to the returned file pointer; the file may be
// reopened later for reading, but not for writing.  The <mode> string
// should be "w" or "wb".  Returns true on success.
extern GBool openTempFile(GooString **name, FILE **f, const char *mode);

#ifdef WIN32
// Convert a file name from Latin-1 to UTF-8.
extern GooString *fileNameToUTF8(char *path);

// Convert a file name from UCS-2 to UTF-8.
extern GooString *fileNameToUTF8(wchar_t *path);
#endif

// Open a file.  On Windows, this converts the path from UTF-8 to
// UCS-2 and calls _wfopen (if available).  On other OSes, this simply
// calls fopen.
extern FILE *openFile(const char *path, const char *mode);

// Just like fgets, but handles Unix, Mac, and/or DOS end-of-line
// conventions.
extern char *getLine(char *buf, int size, FILE *f);

// Like fseek/ftell but uses platform specific variants that support large files
extern int Gfseek(FILE *f, Goffset offset, int whence);
extern Goffset Gftell(FILE *f);

// Largest offset supported by Gfseek/Gftell
extern Goffset GoffsetMax();

//------------------------------------------------------------------------
// GooFile
//------------------------------------------------------------------------

class GooFile
{
public:
  int read(char *buf, int n, Goffset offset) const;
  Goffset size() const;
  
  static GooFile *open(const GooString *fileName);
  
#ifdef _WIN32
  static GooFile *open(const wchar_t *fileName);
  
  ~GooFile() { CloseHandle(handle); }
  
private:
  GooFile(HANDLE handleA): handle(handleA) {}
  HANDLE handle;
#else
  ~GooFile() { close(fd); }
    
private:
  GooFile(int fdA) : fd(fdA) {}
  int fd;
#endif // _WIN32
};

//------------------------------------------------------------------------
// GDir and GDirEntry
//------------------------------------------------------------------------

class GDirEntry {
public:

  GDirEntry(char *dirPath, char *nameA, GBool doStat);
  ~GDirEntry();
  GooString *getName() { return name; }
  GooString *getFullPath() { return fullPath; }
  GBool isDir() { return dir; }

private:
  GDirEntry(const GDirEntry &other);
  GDirEntry& operator=(const GDirEntry &other);

  GooString *name;		// dir/file name
  GooString *fullPath;
  GBool dir;			// is it a directory?
};

class GDir {
public:

  GDir(char *name, GBool doStatA = gTrue);
  ~GDir();
  GDirEntry *getNextEntry();
  void rewind();

private:
  GDir(const GDir &other);
  GDir& operator=(const GDir &other);

  GooString *path;		// directory path
  GBool doStat;			// call stat() for each entry?
#if defined(_WIN32)
  WIN32_FIND_DATA ffd;
  HANDLE hnd;
#elif defined(ACORN)
#elif defined(MACOS)
#else
  DIR *dir;			// the DIR structure from opendir()
#ifdef VMS
  GBool needParent;		// need to return an entry for [-]
#endif
#endif
};

#endif
