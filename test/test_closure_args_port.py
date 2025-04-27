import os

def get(unused_ports, unused_settings, unused_shared):
  return []


def clear(unused_ports, unused_settings, unused_shared):
  pass


def linker_setup(unused_ports, settings):
  externs_file = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                             'test_closure_externs.js')
  settings.CLOSURE_ARGS += [ f'--externs={externs_file}']
