//========================================================================
//
// FileSpec.h
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2008 Carlos Garcia Campos <carlosgc@gnome.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef FILE_SPEC_H
#define FILE_SPEC_H

#include "goo/gtypes.h"
#include "Object.h"

GBool getFileSpecName (Object *fileSpec, Object *fileName);
GBool getFileSpecNameForPlatform (Object *fileSpec, Object *fileName);

#endif /* FILE_SPEC_H */
