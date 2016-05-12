# -*- encoding: utf8 -*-

'''
This tool extracts information about structs and defines from the C headers.
You can pass either the raw header files or JSON files to this script.

The JSON input format is as follows:
[
  {
    'file': 'some/header.h',
    'structs': {
      'struct_name': [
        'field1',
        'field2',
        'field3',
        {
          'field4': [
            'nested1',
            'nested2',
            {
              'nested3': [
                'deep_nested1',
                ...
              ]
            }
            ...
          ]
        },
        'field5'
      ],
      'other_struct': [
        'field1',
        'field2',
        ...
      ]
    },
    'defines': [
      'DEFINE_1',
      'DEFINE_2',
      ['f', 'FLOAT_DEFINE'],
      'DEFINE_3',
      ...
    ]
  },
  {
    'file': 'some/other/header.h',
    ...
  }
]

Please note that the 'f' for 'FLOAT_DEFINE' is just the format passed to printf(), you can put anything printf() understands.
If you call this script with the flag "-f" and pass a header file, it will create an automated boilerplate for you.

The JSON output format is based on the return value of Runtime.generateStructInfo().
{
  'structs': {
    'struct_name': {
      '__size__': <the struct's size>,
      'field1': <field1's offset>,
      'field2': <field2's offset>,
      'field3': <field3's offset>,
      'field4': {
        '__size__': <field4's size>,
        'nested1': <nested1's offset>,
        ...
      },
      ...
    }
  },
  'defines': {
    'DEFINE_1': <DEFINE_1's value>,
    ...
  }
}

'''

import sys, os, re, json, argparse, tempfile, subprocess
import shared

DEBUG = os.environ.get('EMCC_DEBUG')
if DEBUG == "0":
  DEBUG = None

QUIET = (__name__ != '__main__')

def show(msg):
  global QUIET, DEBUG
  if DEBUG or not QUIET:
    sys.stderr.write('gen_struct_info: ' + msg + '\n')

# Try to load pycparser.
try:
  import pycparser
except ImportError:
  # The import failed
  
  def parse_header(path, cpp_opts):
    # Tell the user how to get pycparser, if he or she tries to parse a C file.
    sys.stderr.write('ERR: I need pycparser to process C files. \n')
    sys.stderr.write('   Use "pip install pycparser" to install or download it from "https://github.com/eliben/pycparser".\n')
    sys.exit(1)
else:
  # We successfully imported pycparser, the script will be completely functional.
  
  class DelayedRef(object):
    def __init__(self, dest):
      self.dest = dest
    
    def __str__(self):
      return self.dest
  
  # For a list of node types and their fields, look here: https://github.com/eliben/pycparser/blob/master/pycparser/_c_ast.cfg
  class FieldVisitor(pycparser.c_ast.NodeVisitor):
    def __init__(self):
      self._name = None
      self.structs = {}
      self.named_structs = {}
    
    def visit_Struct(self, node):
      if node.decls == None:
        self.named_structs[self._name] = DelayedRef(node.name)
        return
        
      
      fields = []
      for decl in node.decls:
        if decl.name == None:
          # Well, this field doesn't have a name.
          continue
        
        if decl.type != None and isinstance(decl.type, pycparser.c_ast.PtrDecl):
          # This field is a pointer, there's no point in looking for nested structs.
          fields.append(decl.name)
        else:
          # Look for nested structs.
          subwalk = FieldVisitor()
          subwalk.visit(decl)
          
          if subwalk.named_structs:
            # Store the nested fields.
            fields.append(subwalk.named_structs)
          else:
            # Just store the field name.
            fields.append(decl.name)
      
      if node.name != None:
        self.structs[node.name] = fields
      
      self.named_structs[self._name] = fields
    
    def visit_Union(self, node):
      self.visit_Struct(node)
    
    def visit_TypeDecl(self, node):
      # Remember the name of this typedef, so we can access it later in visit_Struct().
      old_name = self._name
      self._name = node.declname
      self.generic_visit(node)
      self._name = old_name
  
  # The first parameter is a structure, the second is a path (a list containing all the keys, needed to reach the destination).
  # The last parameter is an item to look for. This function will try to follow the path into the given object and then look there for this key.
  # As long as the nested object doesn't have the given key, it will descent into the next higher object till it finds the given key.
  # 
  # Example:
  # 
  # res = look_through({
  #   'la1': {
  #     'lb1': {
  #       'lc1': 99,
  #       'lc2': { 'ld1': 11 }
  #       'lc2': 200
  #     },
  #     'nice': 100
  #   },
  #   'nice': 300
  # }, ['la1', 'lb1', 'lc2'], 'nice')
  # 
  # print(res)    # Prints 100 .
  # 
  # In this case the function looked inside obj['la1']['lb1']['lc2']['nice'], then obj['la1']['lb1']['nice'] and found the value
  # in obj['la1']['nice']. As soon as it finds a value it returns it and stops looking.
  def look_through(obj, path, name):
    cur_level = obj
    path = path[:]
    for i, p in enumerate(path):
      cur_level = cur_level[p]
      path[i] = cur_level
    
    path = [ obj ] + path
    
    while len(path) > 0:
      if name in path[-1]:
        return path[-1][name]
      else:
        path.pop()
    
    return None
  
  # Use the above function to resolve all DelayedRef() inside a list or dict recursively.
  def resolve_delayed(item, root=None, path=[]):
    if root == None:
      root = item
    
    if isinstance(item, DelayedRef):
      if item.dest in path:
        show('WARN: Circular reference found! Field "' + path[-1] + '" references "' + item.dest + '"! (Path = ' + '/'.join([str(part) for part in path]) + ')')
        return { '__ref__': item.dest }
      else:
        return look_through(root, path[:-1], item.dest)
    elif isinstance(item, dict):
      for name, val in item.items():
        item[name] = resolve_delayed(val, root, path + [ name ])
    elif isinstance(item, list):
      for i, val in enumerate(item):
        item[i] = resolve_delayed(val, root, path + [ i ])
    
    return item
  
  def parse_header(path, cpp_opts):
    show('Parsing header "' + path + '"...')
    
    # Use clang -E as the preprocessor for pycparser.
    ast = pycparser.parse_file(path, True, cpp_path=shared.CLANG_CC, cpp_args=['-E'] + cpp_opts)
    
    # Walk the parsed AST and filter out all the declared structs and their fields.
    walker = FieldVisitor()
    walker.visit(ast)
    
    walker.structs = resolve_delayed(walker.structs)
    with open(path, 'r') as stream:
      defines = re.findall(r'(?:^|\n)\s*#define\s+([A-Z|_|0-9]+)\s.*', stream.read())
    
    return {
      'file': path,
      'defines': defines,
      'structs': walker.structs
    }

# The following three functions generate C code. The output of the compiled code will be
# parsed later on and then put back together into a dict structure by parse_c_output().
# 
# Example:
#   c_descent('test1', code)
#   c_set('item', 'i%i', '111', code)
#   c_set('item2', 'i%i', '9', code)
#   c_set('item3', 's%s', '"Hello"', code)
#   c_ascent(code)
#   c_set('outer', 'f%f', '0.999', code)
#   
# Will result in:
#   {
#     'test1': {
#       'item': 111,
#       'item2': 9,
#       'item3': 'Hello',
#     },
#     'outer': 0.999
#   }
def c_set(name, type_, value, code):
  code.append('printf("K' + name + '\\n");')
  code.append('printf("V' + type_ + '\\n", ' + value + ');')

def c_descent(name, code):
  code.append('printf("D' + name + '\\n");')

def c_ascent(code):
  code.append('printf("A\\n");')

def parse_c_output(lines):
  result = {}
  cur_level = result
  parent = []
  key = None
  
  for line in lines:
    arg = line[1:].strip()
    if line[0] == 'K':
      # This is a key
      key = arg
    elif line[0] == 'V':
      # A value
      if arg[0] == 'i':
        arg = int(arg[1:])
      elif arg[0] == 'f':
        arg = float(arg[1:])
      elif arg[0] == 's':
        arg = arg[1:]
      
      cur_level[key] = arg
    elif line[0] == 'D':
      # Remember the current level as the last parent.
      parent.append(cur_level)
      
      # We descend one level.
      cur_level[arg] = {}
      cur_level = cur_level[arg]
    elif line[0] == 'A':
      # We return to the parent dict. (One level up.)
      cur_level = parent.pop()
  
  return result

def gen_inspect_code(path, struct, code):
  if path[0][-1] == '#':
    path[0] = path[0][:-1]
    prefix = ''
  else:
    prefix = 'struct '
  
  c_descent(path[-1], code)
  
  if len(path) == 1:
    c_set('__size__', 'i%zu', 'sizeof (' + prefix + path[0] + ')', code)
  else:
    c_set('__size__', 'i%zu', 'sizeof ((' + prefix + path[0] + ' *)0)->' + '.'.join(path[1:]), code)
    #c_set('__offset__', 'i%zu', 'offsetof(' + prefix + path[0] + ', ' + '.'.join(path[1:]) + ')', code)
  
  for field in struct:
    if isinstance(field, dict):
      # We have to recurse to inspect the nested dict.
      fname = field.keys()[0]
      gen_inspect_code(path + [fname], field[fname], code)
    else:
      c_set(field, 'i%zu', 'offsetof(' + prefix + path[0] + ', ' + '.'.join(path[1:] + [field]) + ')', code)
  
  c_ascent(code)

def inspect_code(headers, cpp_opts, structs, defines):
  show('Generating C code...')
  
  code = ['#include <stdio.h>', '#include <stddef.h>']
  # Include all the needed headers.
  for path in headers:
    code.append('#include "' + path + '"')
  
  code.append('int main() {')
  c_descent('structs', code)
  for name, struct in structs.items():
    gen_inspect_code([name], struct, code)
  
  c_ascent(code)
  c_descent('defines', code)
  for name, type_ in defines.items():
    # Add the necessary python type, if missing.
    if '%' not in type_:
      if type_[-1] in ('d', 'i', 'u'):
        # integer
        type_ = 'i%' + type_
      elif type_[-1] in ('f', 'F', 'e', 'E', 'g', 'G'):
        # float
        type_ = 'f%' + type_
      elif type_[-1] in ('x', 'X', 'a', 'A', 'c', 's'):
        # hexadecimal or string
        type_ = 's%' + type_
    
    c_set(name, type_, name, code)
  
  code.append('return 0;')
  code.append('}')
  
  # Write the source code to a temporary file.
  src_file = tempfile.mkstemp('.c')
  js_file = tempfile.mkstemp('.js')
  
  os.write(src_file[0], '\n'.join(code))
  
  # Close all unneeded FDs.
  os.close(src_file[0])
  os.close(js_file[0])

  # Remove dangerous env modifications
  safe_env = os.environ.copy()
  for opt in ['EMCC_FORCE_STDLIBS', 'EMCC_ONLY_FORCED_STDLIBS']:
    if opt in safe_env:
      del safe_env[opt]

  # Use binaryen, if necessary
  binaryen = os.environ.get('EMCC_WASM_BACKEND_BINARYEN')
  if binaryen:
    cpp_opts += ['-s', 'BINARYEN=1']

  info = []

  try:
    try:
      # Compile the program.
      show('Compiling generated code...')
      subprocess.check_call([shared.PYTHON, shared.EMCC] + cpp_opts + ['-o', js_file[1], src_file[1], '-s', 'BOOTSTRAPPING_STRUCT_INFO=1', '-s', 'WARN_ON_UNDEFINED_SYMBOLS=0', '-Oz', '--js-opts', '0', '--memory-init-file', '0'], env=safe_env) # -Oz optimizes enough to avoid warnings on code size/num locals
    except:
      sys.stderr.write('FAIL: Compilation failed!\n')
      sys.exit(1)

    # Run the compiled program.
    show('Calling generated program...')
    try:
      info = shared.run_js(js_file[1]).splitlines()
    except subprocess.CalledProcessError:
      sys.stderr.write('FAIL: Running the generated program failed!\n')
      sys.exit(1)

  finally:
    # Remove all temporary files.
    os.unlink(src_file[1])
    
    if os.path.exists(js_file[1]):
      os.unlink(js_file[1])
  
  # Parse the output of the program into a dict.
  return parse_c_output(info)

def parse_json(path, header_files, structs, defines):
  with open(path, 'r') as stream:
    # Remove comments before loading the JSON.
    data = json.loads(re.sub(r'//.*\n', '', stream.read()))
  
  if not isinstance(data, list):
    data = [ data ]
  
  for item in data:
    header_files.append(item['file'])
    for name, data in item['structs'].items():
      if name in structs:
        show('WARN: Description of struct "' + name + '" in file "' + item['file'] + '" replaces an existing description!')
      
      structs[name] = data
    
    for part in item['defines']:
      if not isinstance(part, list):
        # If no type is specified, assume integer.
        part = ['i', part]
      
      if part[1] in defines:
        show('WARN: Description of define "' + part[1] + '" in file "' + item['file'] + '" replaces an existing description!')
      
      defines[part[1]] = part[0]

def output_json(obj, compressed=True, stream=None):
  if stream == None:
    stream = sys.stdout
  elif isinstance(stream, str):
    stream = open(stream, 'w')
  
  if compressed:
    json.dump(obj, stream, separators=(',', ':'))
  else:
    json.dump(obj, stream, indent=4, sort_keys=True)
  
  stream.close()

def filter_opts(opts):
  # Only apply compiler options regarding syntax, includes and defines.
  # We have to compile for the current system, we aren't compiling to bitcode after all.
  out = []
  for flag in opts:
    if flag[:2] in ('-f', '-I', '-i', '-D', '-U'):
      out.append(flag)
  
  return out

def main(args):
  global QUIET
  
  parser = argparse.ArgumentParser(description='Generate JSON infos for structs.')
  parser.add_argument('headers', nargs='+', help='A header (.h) file or a JSON file with a list of structs and their fields')
  parser.add_argument('-q', dest='quiet', action='store_true', default=False, help='Don\'t output anything besides error messages.')
  parser.add_argument('-f', dest='list_fields', action='store_true', default=False, help='Output a list of structs and fields for the given headers.')
  parser.add_argument('-p', dest='pretty_print', action='store_true', default=False, help='Pretty print the outputted JSON.')
  parser.add_argument('-o', dest='output', metavar='path', default=None, help='Path to the JSON file that will be written. If omitted, the generated data will be printed to stdout.')
  parser.add_argument('-I', dest='includes', metavar='dir', action='append', default=[], help='Add directory to include search path')
  parser.add_argument('-D', dest='defines', metavar='define', action='append', default=[], help='Pass a define to the preprocessor')
  parser.add_argument('-U', dest='undefines', metavar='undefine', action='append', default=[], help='Pass an undefine to the preprocessor')
  args = parser.parse_args(args)
  
  QUIET = args.quiet
  
  # Avoid parsing problems due to gcc specifc syntax.
  cpp_opts = ['-D_GNU_SOURCE']
  
  # Add the user options to the list as well.
  for path in args.includes:
    cpp_opts.append('-I' + path)
  
  for arg in args.defines:
    cpp_opts.append('-D' + arg)
  
  for arg in args.undefines:
    cpp_opts.append('-U' + arg)
  
  if args.list_fields:
    # Just parse the given headers and output the result.
    data = []
    for path in args.headers:
      if path[-5:] == '.json':
        show('WARN: Skipping "' + path + '" because it\'s already a JSON file!')
      else:
        data.append(parse_header(path, cpp_opts))
    
    output_json(data, not args.pretty_print, args.output)
    sys.exit(0)
  
  # Look for structs in all passed headers.
  header_files = []
  structs = {}
  defines = {}
  
  for header in args.headers:
    if header[-5:] == '.json':
      # This is a JSON file, parse it.
      parse_json(header, header_files, structs, defines)
    else:
      # If the passed file isn't a JSON file, assume it's a header.
      header_files.append(header)
      data = parse_header(header, cpp_opts)
      structs.update(data['structs'])
      defines.extend(data['defines'])
  
  # Inspect all collected structs.
  struct_info = inspect_code(header_files, cpp_opts, structs, defines)
  output_json(struct_info, not args.pretty_print, args.output)

if __name__ == '__main__':
  main(sys.argv[1:])
