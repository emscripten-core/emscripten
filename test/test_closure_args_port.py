import os

def get(ports, settings, shared):
  return []


def clear(ports, settings, shared):
  pass


def linker_setup(ports, settings):
  externs_file = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                             'test_closure_externs.js')
  settings.CLOSURE_ARGS += [ f'--externs={externs_file}']
