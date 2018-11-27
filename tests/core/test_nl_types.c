/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <nl_types.h>
#include <stdio.h>

int main(int argc, char ** argv) {
 nl_catd c = catopen("none", 0);
 printf("Hello, %s.\n", catgets(c, 0, 0, "world"));
 return catclose(c);
}
