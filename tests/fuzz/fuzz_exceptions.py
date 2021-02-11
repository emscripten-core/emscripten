import json
import random

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

Node = int or Array
Array = Node^K, K >= 0

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
'''
class StructuredRandomData:
    # The overall maximum number of nodes we want.
    MAX_NODES = 200

    # The minimum and maximum size of an array.
    MIN_ARRAY_SIZE = 3
    MAX_ARRAY_SIZE = 10

    # How much shorter arrays are the deeper we go.
    ARRAY_DEPTH_SHORTENING = 0.95

    # How likely we are to create an array instead of an int.
    ARRAY_PROBABILITY = 0.3

    # How much each extra level of depth makes it less likely to have an array.
    ARRAY_DEPTH_UNLIKELIHOOD = 0.95

    # The maximum depth.
    MAX_DEPTH = 7

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

    def make_int(self):
        self.emitted_nodes += 1
        return random.randrange(0, 256)

    def make(self, depth):
        if self.emitted_nodes < self.MAX_NODES and \
           random.random() < \
           self.ARRAY_PROBABILITY * (self.ARRAY_DEPTH_UNLIKELIHOOD ** depth):
            return self.make_array(depth)
        return self.make_int()

# To see an example, run this line:
# print(json.dumps(StructuredRandomData().root, indent='  '))


def intify(node):
    if type(node) == list:
        return len(list)
    return node


def arrayify(node):
    if type(node) != list:
        return [node]
    return node


'''
A cursor over an array, allowing gradual consumption of it. If we run out, we
return simple values.
'''
class Cursor:
    def __init__(self, array):
        self.array = array
        self.pos = 0

    def get(self):
        if self.pos >= len(self.array):
            return 0
        self.pos += 1
        return self.array[self.pos - 1]


'''
Translates random structured data into a random C++ program that uses C++
exceptions.
'''
class CppTranslator:
    PREAMBLE = '''\
#include <stdio.h> // avoid iostream C++ code, just test libc++abi, not libc++
'''

    def __init__(self, input):
        self.toplevel = Cursor(input)

        # The output is a list of strings which will be concatenated at the end.
        self.output = [self.PREAMBLE]

        self.output.append(self.make_structs())

    def make_structs(self):
        array = arrayify(self.toplevel.get())
        self.structs = []
        ret = []
        for node in array:
            name = f'Struct{len(ret)}'










