
import os, sys

import shared

sys.path.append(shared.path_from_root('third_party'))
sys.path.append(shared.path_from_root('third_party', 'ply'))

import WebIDL

print dir(WebIDL)

