#!/usr/bin/env python3

import os
from lit.main import main

# A simple wrapper around `lit` test running.   Loosely based on
# llvm's llvm-lit script

script_dir = os.path.dirname(os.path.abspath(__file__))

builtin_parameters = {
    'config_map': {
        os.path.normcase(os.path.join(script_dir, 'lit/lit.cfg.py')):
        os.path.normcase(os.path.join(script_dir, 'lit/lit.site.cfg.py'))
    }
}

if __name__ == '__main__':
    main(builtin_parameters)
