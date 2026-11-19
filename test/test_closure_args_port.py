import os


def get(_ports, _settings, _shared):
  return []


def clear(_ports, _settings, _shared):
  pass


def linker_setup(_ports, settings):
  externs_file = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                             'test_closure_externs.js')
  settings.CLOSURE_ARGS += [f'--externs={externs_file}']
