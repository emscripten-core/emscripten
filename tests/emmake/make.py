import os

def which(program):
  def is_exe(fpath):
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

  fpath, fname = os.path.split(program)
  if fpath:
    if is_exe(program):
      return program
  else:
    for path in os.getenv("PATH", "").split(os.pathsep):
      exe_file = os.path.join(path, program)
      if is_exe(exe_file):
        return exe_file
  raise Exception('that is very bad')

def test(what):
  print(what)
  print(which(os.getenv(what, '')))

def check_ar():
  print("Testing...")
  test("CC")
  test("CXX")
  test("AR")
  test("LD")
  test("NM")
  test("LDSHARED")
  test("RANLIB")
  print("Done.")

check_ar()

