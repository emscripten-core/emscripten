# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# This class can be used to produce a set of minified names to be used as JS
# variables
class MinifiedJsNameGenerator(object):
  reserved_names = ['do', 'if', 'in', 'for', 'new', 'try', 'var', 'env', 'let', 'case', 'else', 'enum', 'void', 'this', 'with']
  valid_first_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$"
  valid_later_chars = valid_first_chars + '0123456789'

  name_iterator = []

  overflow_warned = False

  def max_length(self, pos):
    return len(self.valid_first_chars) if pos == 0 else len(self.valid_later_chars)

  def produce_name(self):
    name = ''
    for i in range(len(self.name_iterator) - 1, 0, -1):
      name += self.valid_later_chars[self.name_iterator[i]]
    name += self.valid_first_chars[self.name_iterator[0]]
    return name

  def generate(self):
    i = 0
    while i < len(self.name_iterator):
      self.name_iterator[i] += 1
      if self.name_iterator[i] >= self.max_length(i):
        self.name_iterator[i] = 0
        i += 1
      else:
        name = self.produce_name()
        if name not in self.reserved_names:
          return name

    self.name_iterator += [0]
    if len(self.name_iterator) >= 5:
      if not self.overflow_warned:
        logging.warning('MinifiedJsNameGenerator has only been defined for symbols up to 4 characters! TODO: Add JavaScript reserved names of length 5 and more to this list')
        self.overflow_warned = True

    name = self.produce_name()
    if name not in self.reserved_names:
      return name
    else:
      return self.generate()
