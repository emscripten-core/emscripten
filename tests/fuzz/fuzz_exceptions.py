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
'''

import random

class Generator:
    # The overall maximum number of nodes we want.
    MAX_NODES = 20

    # The maximum size of an array.
    MAX_ARRAY = 7

    # How much shorter arrays are the deeper we go.
    ARRAY_DEPTH_SHORTENING = 0.9

    # How likely we are to create an array instead of an int.
    ARRAY_PROBABILITY = 0.3

    # How much each extra level of depth makes it less likely to have an array.
    ARRAY_DEPTH_UNLIKELIHOOD = 0.9

    # The maximum depth.
    MAX_DEPTH = 5

    def __init__(self):
        self.emitted_nodes = 0
        self.root = self.make_array(depth=0)
        while self.emitted_nodes < self.MAX_NODES:
            self.root.append(self.make_array(depth=1))

    def make_array(self, depth):
        size = random.randrange(0, self.MAX_ARRAY - int(depth * self.ARRAY_DEPTH_SHORTENING))
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

print(Generator().root)

















