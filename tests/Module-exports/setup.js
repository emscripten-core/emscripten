/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * This file provides some setup for the emscripten runtime. In particular it
 * prevents the runtime exiting. This is necessary as otherwise things like
 * printf don't seem to work from methods called by JavaScript.
 */
Module = {
    'noExitRuntime' : true
};
