import os

def get(ports, settings, shared): # noqa: ARG001
  return []


def clear(ports, settings, shared): # noqa: ARG001
  pass


def linker_setup(ports, settings): # noqa: ARG001
  externs_file = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                             'test_closure_externs.js')
  settings.CLOSURE_ARGS += [ f'--externs={externs_file}']
