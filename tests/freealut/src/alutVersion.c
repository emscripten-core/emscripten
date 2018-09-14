/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "alutInternal.h"

ALint alutGetMajorVersion(void)
{
  return ALUT_API_MAJOR_VERSION;
}

ALint alutGetMinorVersion(void)
{
  return ALUT_API_MINOR_VERSION;
}
