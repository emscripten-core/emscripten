import json
import random

# Determinism for testing.
random.seed(2)

'''
Structural fuzz generator. 

Like the AFL and Binaryen etc. fuzzers, we start with random bytes as the input.
However, we start with a tree structure of random bytes, something like

[
  1,
  [
    42,
    50
    17
  ],
  [
    [
    ],
    [
      2,
      12
    ]
  ]
]

The grammar here is simply

Node = Number or Array
Array = Node^K, K >= 0
Number = [0..1)

Starting from structured random data has the benefit of making it easy to reduce
on the random input. Consider if the structure of the random input gets mapped
to something else with structure, like a source program, then pruning the input
can lead to similar pruned source programs. (In comparison, unstructured random
data allows for truncation easily, but changing bytes earlier can lead to
dramatic differences in the output.)

To get this benefit, the translator of the random structured data must convert
it to the output in a structured manner. That is, one node should be converted
to a corresponding node, and without looking at other nodes as much as possible,
so that if they are altered, that one will not be.

A downside to this approach is that a very large random input may lead to a very
small output, for example, if a huge nested tree of data is consumed in a place
that just wants a bool.
'''
class StructuredRandomData:
    # The overall maximum number of nodes we want.
    MAX_NODES = 2000

    # The minimum and maximum size of an array.
    MIN_ARRAY_SIZE = 2
    MAX_ARRAY_SIZE = 10

    # How much shorter arrays are the deeper we go.
    ARRAY_DEPTH_SHORTENING = 0.5

    # How likely we are to create an array instead of a number (at the top
    # level).
    ARRAY_PROBABILITY = 0.5

    # A reduction factor on the probability to have an array, per depth level.
    ARRAY_DEPTH_UNLIKELIHOOD = 0.66

    # How low the array probability can go due to ARRAY_DEPTH_UNLIKELIHOOD.
    MIN_ARRAY_PROBABILITY = 0.1

    # The maximum depth.
    MAX_DEPTH = 20

    def __init__(self):
        self.emitted_nodes = 0
        # The root is always an array, so that we can append enough items to get
        # to the size we want.
        self.root = self.make_array(depth=0)
        while self.emitted_nodes < self.MAX_NODES:
            self.root.append(self.make(depth=1))

    def make_array(self, depth):
        max_size = self.MAX_ARRAY_SIZE - int(depth * self.ARRAY_DEPTH_SHORTENING)
        max_size = max(max_size, self.MIN_ARRAY_SIZE + 1)
        size = random.randint(self.MIN_ARRAY_SIZE, max_size)
        size = min(size, self.MAX_NODES - self.emitted_nodes)
        self.emitted_nodes += 1
        return [self.make(depth + 1) for i in range(0, size)]

    def make_num(self):
        self.emitted_nodes += 1
        return random.random()

    def make(self, depth):
        if self.emitted_nodes < self.MAX_NODES:
          array_probability = self.ARRAY_PROBABILITY * (self.ARRAY_DEPTH_UNLIKELIHOOD ** depth)
          array_probability = max(array_probability, self.MIN_ARRAY_PROBABILITY)
          if random.random() < array_probability:
            return self.make_array(depth)
        return self.make_num()

# To see an example, run this line:
# print(json.dumps(StructuredRandomData().root, indent='  '))


def numify(node):
    if type(node) == list:
        if len(node) == 0:
            return 0
        return numify(node[0])
    return node


def arrayify(node):
    if type(node) != list:
        return [node]
    return node

def indent(code):
    return '\n'.join(['  ' + line for line in code.splitlines() if line])

'''
A cursor over an array, allowing gradual consumption of it. If we run out, we
return simple values.
'''
class Cursor:
    def __init__(self, array):
        self.array = arrayify(array)
        self.pos = 0

    def get(self):
        if self.pos >= len(self.array):
            return 0
        self.pos += 1
        return self.array[self.pos - 1]

    def get_num(self):
        return numify(self.get())

    def get_array(self):
        return arrayify(self.get())

    def remaining(self):
        return max(0, len(self.array) - self.pos)

    def has_more(self):
        return self.remaining() > 0


'''
Given a list of options (weight, func), and a value in [0, 1) to help pick from
them, pick one, and call it with the parameter.
'''
def pick(options, value, param):
    # Scale the value by the total weight.
    assert 0 <= value < 1, value
    total = 0
    for weight, func in options:
        total += weight
    value *= total

    for weight, func in options:
        if value < weight:
            return func(param)
        value -= weight

    raise Exception('inconceivable')


'''
Translates random structured data into a random C++ program that uses C++
exceptions.
'''
class CppTranslator:
    PREAMBLE = '''\
#include <stdio.h> // avoid iostream C++ code, just test libc++abi, not libc++
#include <stdint.h>

extern void checkRecursion();
extern bool getBoolean();
'''

    SUPPORT = '''\
#include <stdio.h>
#include <stdlib.h>

static int fuel = 100;

void checkRecursion() {
  if (fuel == 0) {
    puts("out of fuel");
    abort();
  }
  fuel--;
}

// TODO random data
static bool boolean = true;

bool getBoolean() {
  boolean = !boolean;
  return boolean;
}
'''

    def __init__(self, input):
        print(json.dumps(input.root, indent='  '))
        self.toplevel = Cursor(input.root)
        self.logging_index = 0

        # The output is a list of strings which will be concatenated when
        # writing.
        self.output = [self.PREAMBLE]
        self.make_structs()
        self.make_functions()

    '''
    Outputs the main file and the support file on the side. Support code is not
    in the main file so that the optimizer cannot see it all.
    '''
    def write(self, main, support):
        with open(main, 'w') as f:
            f.write('\n'.join(self.output))
        with open(support, 'w') as f:
            f.write(self.SUPPORT)

    def make_structs(self):
        array = arrayify(self.toplevel.get())
        # Global mapping of struct name to its array of fields.
        self.structs = {}
        structs = []
        for node in array:
            name = f'Struct{len(structs)}'
            sig = self.get_types(node)
            self.structs[name] = sig
            fields = '\n'.join([f'  {t} f{i};' for i, t in enumerate(sig)])
            structs.append('''\
struct %(name)s {
%(fields)s
};
''' % locals())
        self.output.append('\n'.join(structs))

    def make_functions(self):
        funcs = []
        main = '''\
int main() {
'''
        while self.toplevel.has_more():
            name = f'func_{len(funcs)}'
            body = indent(self.make_function_body(self.toplevel.get()))
            funcs.append('''\
void %(name)s() {
%(body)s
}
''' % locals())
            main += '''\
  // %(name)s
  puts("calling %(name)s");
  try {
    %(name)s();
  } catch(...) {
    puts("main caught from %(name)s");
  }
''' % locals()

        main +='''\
  return 0;
}
'''
        funcs.append(main)
        self.output.append('\n'.join(funcs))

    def make_function_body(self, node):
        statements = [self.make_statement(n) for n in arrayify(node)]
        return '\n'.join(statements)

    def make_statement(self, node):
        cursor = Cursor(node)
        return pick([
          (1, self.make_nothing),
          (10, self.make_logging),
          (10, self.make_throw),
          (10, self.make_catch),
          (10, self.make_if),
        ], cursor.get_num(), cursor)

    def make_nothing(self, cursor):
        return ''

    def make_logging(self, cursor):
        if cursor.has_more():
            return f'puts("log({cursor.get_num()})");'
        self.logging_index += 1
        return f'puts("log({self.logging_index})");'

    def make_throw(self, cursor):
        return f'throw {cursor.get_num()};'

    def make_catch(self, cursor):
        body = indent(self.make_statement(cursor.get_array()))
        catch = indent(self.make_statement(cursor.get_array()))

        return '''\
try {
%(body)s
} catch(...) {
%(catch)s
}
''' % locals()

    def make_if(self, cursor):
        if_arm = indent(self.make_statement(cursor.get_array()))

        else_ = ''
        if cursor.get_num() >= 0.5:
            else_arm = indent(self.make_statement(cursor.get_array()))
            else_ = '''\
 else {
%(else_arm)s
}''' % locals()

        return '''\
if (getBoolean()) {
%(if_arm)s
}%(else_)s
''' % locals()

    def get_types(self, node):
        return [self.get_type(x) for x in arrayify(node)]

    def get_type(self, node):
        if numify(node) < 0.5:
            return 'uint32_t'
        return 'double'

CppTranslator(StructuredRandomData()).write(main='a.cpp', support='b.cpp')

