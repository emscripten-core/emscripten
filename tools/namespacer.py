#!/usr/bin/python

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

data = open(sys.argv[1], 'r').readlines()

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

  key = str(funcparts)
  if key in counts:
    i = counts[key]
    counts[key] += 1
    funcparts[-1] += '_' + str(i)
  else:
    counts[key] = 1
  currspace[funcparts[-1]] = realname
  currspace[funcparts[-1] + '__params'] = params
  if len(funcparts) >= 2 and funcparts[-1] == funcparts[-2]:
    currspace['__new__'] = 'function() { var ret = _malloc(_struct_%s___SIZE); Module._.%s.%s.apply(null, [ret].concat(Array.prototype.slice.apply(arguments))); return ret; }' % (
      funcparts[-1], funcparts[-1], funcparts[-1]
    )

def finalize(line):
  try:
    key, val = line.split(': ')
    assert val != '}' and '_params"' not in key
    return key + ': ' + val.replace('"', '')
  except:
    return line

print '\n'.join(map(finalize, json.dumps(space, sort_keys=True, indent=2).split('\n')))

