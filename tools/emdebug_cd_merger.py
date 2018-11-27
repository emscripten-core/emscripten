#!/usr/bin/env python2
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import sys
import json


def run():
    args = sys.argv[1:]
    assert len(args) == 2

    with open(args[0]) as cd_f:
        cd_data = json.load(cd_f)

    try:
        with open(args[1]) as symbol_f:
            symbol_list = {x[0]: x[2] for x in [z.strip().partition(":") for z in symbol_f.readlines() if len(z)]}
    except IOError:
        # If there's no symbol file, use an empty one
        symbol_list = {}

    cd_data['cyberdwarf']['function_name_map'] = symbol_list

    with open(args[0], "w") as cd_f:
        json.dump(cd_data, cd_f, separators=(',', ':'))


if __name__ == '__main__':
  run()
