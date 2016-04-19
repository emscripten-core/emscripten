#!/usr/bin/env python2
# -*- Mode: python -*-

import logging, sys, json

def run():
    args = sys.argv[1:]
    assert len(args) == 2

    with open(args[0]) as cd_f:
        cd_data = json.load(cd_f)

    try:
        with open(args[1]) as symbol_f:
            symbol_list = {x[0]:x[2] for x in [z.strip().partition(":") for z in symbol_f.readlines() if len(z) > 0]}
    except IOError as e:
        # If there's no symbol file, use an empty one
        symbol_list = {}

    cd_data['cyberdwarf']['function_name_map'] = symbol_list

    with open(args[0],"w") as cd_f:
        json.dump(cd_data, cd_f, separators=(',',':'))


if __name__ == '__main__':
  run()
