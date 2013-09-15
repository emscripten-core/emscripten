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

import sys
import os
import json
import argparse
import tempfile
import subprocess
import shared

# Try to load pycparser.
try:
  import pycparser
except ImportError:
  # The import failed, warn the user.
  sys.stderr.write('WARN: pycparser isn\'t available. I won\'t be able to parse C files, only .json files.\n')
  
  def parse_header(path, cpp_opts):
    # Tell the user how to get pycparser, if he tries to parse a C file.
    sys.stderr.write('ERR: I need pycparser to process C files. \n')
    sys.stderr.write('   Use "pip install pycparser" or go to "https://github.com/eliben/pycparser" to install it.\n')
    sys.exit(1)
else:
  # We successfully imported pycparser, the script will be completely functional.
  
  class FieldVisitor(pycparser.c_ast.NodeVisitor):
    def __init__(self):
      self._name = None
      self.structs = {}
      self.named_structs = {}
    
    def visit_Struct(self, node):
      if node.decls == None:
        # Skip empty struct declarations.
        return
      
      fields = []
      for decl in node.decls:
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

  def parse_header(path, cpp_opts):
    sys.stderr.write('Parsing header "' + path + '"...\n')
    
    # Use clang -E as the preprocessor for pycparser.
    ast = pycparser.parse_file(path, True, cpp_path=shared.CLANG_CC, cpp_args=['-E'] + cpp_opts)
    
    # Walk the parsed AST and filter out all the declared structs and their fields.
    walker = FieldVisitor()
    walker.visit(ast)
    return walker.structs

# The following three functions generate C code. The output of the compiled code will be
# parsed later on and then put back together into a dict structure by parse_c_output().
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
      cur_level[key] = int(arg)
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
  c_descent(path[-1], code)
  if len(path) == 1:
    c_set('__size__', '%lu', 'sizeof (struct ' + path[0] + ')', code)
  else:
    c_set('__size__', '%lu', 'sizeof ((struct ' + path[0] + ' *)0)->' + '.'.join(path[1:]), code)
  
  for field in struct:
    if isinstance(field, dict):
      # We have to recurse to inspect the nested dict.
      fname = field.keys()[0]
      gen_inspect_code(path + [fname], field[fname], code)
    else:
      c_set(field, '%u', 'offsetof(struct ' + path[0] + ', ' + '.'.join(path[1:] + [field]) + ')', code)
  
  c_ascent(code)

def inspect_code(headers, cpp_opts, structs, defines):
  sys.stderr.write('Generating C code...\n')
  
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
  for name in defines:
    if isinstance(name, list):
      type_, name = name
    else:
      type_ = 'i'
    c_set(name, '%' + type_, name, code)
  
  code.append('return 0;')
  code.append('}')
  
  # Write the source code to a temporary file.
  src_file = tempfile.mkstemp('.c')
  bin_file = tempfile.mkstemp()
  
  os.write(src_file[0], '\n'.join(code))
  
  # Close all unneeded FDs.
  os.close(src_file[0])
  os.close(bin_file[0])
  
  info = []
  try:
    # Compile the program.
    sys.stderr.write('Compiling generated code...\n')
    subprocess.check_call([shared.CLANG_CC] + cpp_opts + ['-o', bin_file[1], src_file[1]])
    
    # Run the compiled program.
    sys.stderr.write('Calling generated program...\n')
    info = subprocess.check_output([bin_file[1]]).splitlines()
  except subprocess.CalledProcessError:
    if os.path.isfile(bin_file[1]):
      sys.stderr.write('FAIL: Running the generated program failed!\n')
    else:
      sys.stderr.write('FAIL: Compilation failed!\n')
    
    sys.exit(1)
  finally:
    # Remove all temporary files.
    os.unlink(src_file[1])
    
    if os.path.exists(bin_file[1]):
      os.unlink(bin_file[1])
  
  # Parse the output of the program into a dict.
  data = parse_c_output(info)
  
  # Convert all the define's values into the appropriate python types (based on the type passed to printf).
  for name in defines:
    if isinstance(name, list):
      type_, name = name
    else:
      type_ = 'i'
    
    if type_[-1] in ('d', 'i', 'u'):
      # Integer
      data['defines'][name] = int(data['defines'][name])
    elif type_[-1] in ('x', 'X', 'a', 'A'):
      # Hexadecimal
      data['defines'][name] = float.fromhex(data['defines'][name])
    elif type_[-1] in ('f', 'F', 'e', 'E', 'g', 'G'):
      # Float
      data['defines'][name] = float(data['defines'][name])
    # Leave everything else untouched.
  
  return data

def main():
  parser = argparse.ArgumentParser(description='Generate JSON infos for structs.')
  parser.add_argument('headers', nargs='+', help='A header (.h) file or a JSON file with a list of structs and their fields')
  parser.add_argument('-f', dest='list_fields', action='store_true', default=False, help='Output a list of structs and fields for the first header.')
  parser.add_argument('-p', dest='pretty_print', action='store_true', default=False, help='Pretty print the outputted JSON.')
  parser.add_argument('-o', dest='output', metavar='path', default=None, help='Path to the JSON file that will be written. If omitted, the generated data will be printed to stdout.')
  parser.add_argument('-I', dest='includes', metavar='dir', action='append', default=[], help='Add directory to include search path')
  parser.add_argument('-D', dest='defines', metavar='define', action='append', default=[], help='Pass a define to the preprocessor')
  parser.add_argument('-U', dest='undefines', metavar='undefine', action='append', default=[], help='Pass an undefine to the preprocessor')
  args = parser.parse_args()
  
  # Avoid parsing problems due to gcc specifc syntax.
  cpp_opts = ['-U__GNUC__']
  
  # Only apply compiler options regarding syntax, includes and defines.
  # We have to compile for the current system, we aren't compiling to bitcode after all.
  for flag in shared.COMPILER_OPTS:
    if flag[:2] in ('-f', '-I', '-i', '-D', '-U'):
      cpp_opts.append(flag)
  
  # Add the user options to the list as well.
  for path in args.includes:
    cpp_opts.append('-I' + path)
  
  for arg in args.defines:
    cpp_opts.append('-D' + arg)
  
  for arg in args.undefines:
    cpp_opts.append('-U' + arg)
  
  if args.list_fields:
    # Just parse the first header and output the result.
    structs = parse_header(args.headers[0], cpp_opts)
    data = {
      'file': args.headers[0],
      'structs': structs,
      'defines': []
    }
    
    if args.output == None:
      sys.stdout.write(json.dumps(data, indent=4 if args.pretty_print else None))
    else:
      with open(args.output, 'w') as stream:
        json.dump(data, stream, indent=4 if args.pretty_print else None)
    
    sys.exit(0)
  
  # Look for structs in all passed headers.
  header_files = []
  structs = {}
  defines = {}
  
  for header in args.headers:
    if header[-5:] == '.json':
      # This is a JSON file, simply load it.
      with open(header, 'r') as stream:
        data = json.load(stream)
      
      if not isinstance(data, list):
        data = [ data ]
      
      for item in data:
        header_files.append(item['file'])
        structs.update(item['structs'])
        defines.update(item['defines'])
    else:
      # If the passed file isn't a JSON file, assume it's a header.
      header_files.append(header)
      structs.update(parse_header(header, cpp_opts))
  
  # Inspect all collected structs.
  struct_info = inspect_code(header_files, cpp_opts, structs, defines)
  
  if args.output == None:
    sys.stdout.write(json.dumps(struct_info, indent=4 if args.pretty_print else None))
  else:
    with open(args.output, 'w') as stream:
      json.dump(struct_info, stream, indent=4 if args.pretty_print else None)

if __name__ == '__main__':
  main()