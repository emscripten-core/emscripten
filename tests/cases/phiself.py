# Copyright 2012 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

assert '$phi' not in generated, 'we should optimize out self-phis'  # noqa
