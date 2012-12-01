#!/usr/bin/python2

'''
Tool that generates namespace boilerplate. Given

  _mangled_name1_ = MyClass::MyClass()
  _mangled_name2_ = MyClass::~MyClass()
  _mangled_name3_ = MyClass::DoSomething(int)

the tool will generate

  MyClass = {
    'MyClass':     _mangled_name1_,
    '~MyClass':  _mangled_name2_,
    'DoSomething': _mangled_name3_
  };

You can then do the following in JavaScript:

  MyClass.MyClass(ptr);
  MyClass['~MyClass'](ptr); // Need string here, due to |~|
  MyClass.DoSomething(ptr, 17);

Note that you need to send the |this| pointer yourself. TODO:
a more OO boilerplate on top of that.
'''

import os, sys, json

js = open(sys.argv[1], 'r').read()
data = open(sys.argv[2], 'r').readlines()

space = {}

counts = {}

for line in data:
  line = line.rstrip()

  realname, signature = line.split(' = ')
  signature = signature.replace(') const', ')')
  signature = signature[:-1].split('(')
  func, params = signature[0], '('.join(signature[1:])

  if ' ' in func:
    i = func.index(' ')
    ret = func[:i]
    if '<' not in ret and '[' not in ret and '(' not in ret:
      func = func[i+1:]

  #funcparts = ['Namespace'] + func.split('::')
  funcparts = func.split('::')

  currspace = space
  for part in funcparts[:-1]:
    currspace = currspace.setdefault(part, {})

  finalname = funcparts[-1]
  key = str(funcparts)
  if key in counts:
    i = counts[key]
    counts[key] += 1
    finalname += '_' + str(i)
  else:
    i = 0
    counts[key] = 1
  currspace[finalname] = realname
  currspace[finalname + '__params'] = params
  if len(funcparts) >= 2 and funcparts[-1] == funcparts[-2]:
    found = False
    for what in ['struct', 'class']:
      size = '$%s_%s' % (what, '__'.join(funcparts[:-1]))
      if len(funcparts) > 2:
        size = '_' + size + '_'
      size = size + '___SIZE'
      if size in js:
        found = True
        break
    if not found:
      #print '// Warning: cannot find %s' % ('[_]$[class|struct]' + '__'.join(funcparts[:-1]) + '[_]___SIZE')
      continue
    currspace['__new__' + ('' if i == 0 else str(i))] = 'function() { var ret = _malloc(%s); Module._.%s.%s.apply(null, [ret].concat(Array.prototype.slice.apply(arguments))); return ret; }' % (
      size, '.'.join(funcparts[:-1]), finalname
    )

def finalize(line):
  try:
    key, val = line.split(': ')
    assert val != '}' and '_params"' not in key
    return key + ': ' + val.replace('"', '')
  except:
    return line

print 'var ModuleNames = ' + '\n'.join(map(finalize, json.dumps(space, sort_keys=True, indent=2).split('\n')))

