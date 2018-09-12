/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// This is just a trivial test function, the key bit of interest is that it returns a 64 bit long.
long long test_return64() {
    long long x = ((long long)1234 << 32) + 5678;
    return x;
}
