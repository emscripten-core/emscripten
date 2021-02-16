import json
import os
import random
import subprocess
import sys
import time

'''
Structural fuzz generator.

Like the AFL and Binaryen etc. fuzzers, we start with random bytes as the input.
However, we start with a tree structure of random bytes, something like

[
  1,
  [
    0.42,
    0.501,
    0.17
  ],
  [
    [
    ],
    [
      0.2,
      0.12
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
    NUM_TOPLEVEL = 25

    # The range of widths.
    MIN_WIDTH = 1
    MAX_WIDTH = 4

    # The range of depths.
    MIN_DEPTH = 2
    MAX_DEPTH = 9

    # The chance to just emit a number instead of a list.
    NUM_PROB = 0.25

    def __init__(self):
        self.root = [self.make_toplevel() for x in range(self.NUM_TOPLEVEL)]

    def make_toplevel(self):
        depth_left = random.randint(self.MIN_DEPTH, self.MAX_DEPTH)
        return self.make_array(0, depth_left)

    def make_array(self, depth, depth_left):
        width = random.randint(self.MIN_WIDTH, self.MAX_WIDTH)
        # When there is almost no depth left, emit fewer things.
        width = min(width, depth_left + 1)
        return [self.make(depth + 1, depth_left - 1) for i in range(width)]

    def make_num(self):
        return random.random()

    def make(self, depth, depth_left):
        if depth_left == 0 or random.random() < self.NUM_PROB:
            return self.make_num()
        return self.make_array(depth, depth_left)


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


class Cursor:
    '''
    A cursor over an array, allowing gradual consumption of it. If we run out, we
    return simple values.
    '''
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

    def get_int(self):
        return int(1000 * self.get_num())


def pick(options, value, param):
    '''
    Given a list of options (weight, func), and a value in [0, 1) to help pick from
    them, pick one, and call it with the parameter.
    '''
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


class CppTranslator:
    '''
    Translates random structured data into a random C++ program that uses C++
    exceptions.
    '''

    PREAMBLE = '''\
#include <stdio.h> // avoid iostream C++ code, just test libc++abi, not libc++
#include <stdint.h>

extern void refuel();
extern void checkRecursion();
extern bool getBoolean();
'''

    SUPPORT = '''\
#include <stdio.h>
#include <stdlib.h>

const int INIIAL_FUEL = 100;

static int fuel = INIIAL_FUEL;

void refuel() {
  fuel = INIIAL_FUEL;
}

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
  // If we are done, exit all loops etc.
  if (fuel == 0) {
    return false;
  }
  fuel--;
  boolean = !boolean;
  return boolean;
}
'''

    def __init__(self, data):
        self.toplevel = Cursor(data)
        self.logging_index = 0
        self.try_nesting = 0
        self.loop_nesting = 0

        # The output is a list of strings which will be concatenated when
        # writing.
        self.output = [self.PREAMBLE]
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
  refuel();
  try {
    %(name)s();
  } catch(...) {
    puts("main caught from %(name)s");
  }
''' % locals()

        main += '''\
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
        options = [
          (1,  self.make_nothing),
          (10, self.make_logging),
          (10, self.make_try),
          (10, self.make_if),
          (5,  self.make_loop),
        ]
        if self.try_nesting:
            options.append((10, self.make_throw))
        else:
            # Only rarely emit throws outside of a try.
            options.append((2, self.make_throw))
        if self.loop_nesting:
            options.append((10, self.make_branch))
        return pick(options, cursor.get_num(), cursor)

    def make_nothing(self, cursor):
        return ''

    def make_logging(self, cursor):
        if cursor.has_more():
            return f'puts("log(-{cursor.get_int()})");'
        self.logging_index += 1
        return f'puts("log({self.logging_index})");'

    def make_throw(self, cursor):
        return f'throw {cursor.get_int()};'

    def make_try(self, cursor):
        self.try_nesting += 1
        body = indent(self.make_statement(cursor.get_array()))
        self.try_nesting -= 1
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

    def make_loop(self, cursor):
        self.loop_nesting += 1
        body = indent(self.make_statement(cursor.get_array()))
        self.loop_nesting -= 1

        return '''\
while (getBoolean()) {
%(body)s
}
''' % locals()

    def make_branch(self, cursor):
        assert self.loop_nesting
        if cursor.get_num() < 0.5:
            return 'break;'
        else:
            return 'continue;'

    def get_types(self, node):
        return [self.get_type(x) for x in arrayify(node)]

    def get_type(self, node):
        if numify(node) < 0.5:
            return 'uint32_t'
        return 'double'


# Main harness


def check_testcase(data, silent=True):
    # Generate C++
    CppTranslator(data).write(main='a.cpp', support='b.cpp')

    # Compile with emcc, looking for a compilation error.
    # TODO: also compile b.cpp, and remove -c so that we test linking.
    result = subprocess.run(['./em++', 'a.cpp', '-sWASM_BIGINT', '-c',
                             '-fwasm-exceptions'],
                            stderr=subprocess.PIPE, text=True)

    if not silent:
        print(result.stderr)

    if result.returncode == 0:
        return True

    # Optionally look for something more specific:
    #if 'Delegate destination should be in scope' not in result.stderr:
    #    return True

    return False


def reduce(data):
    '''
    Given a failing testcase, reduce the input data to create a reduced C++
    testcase.
    '''

    assert not check_testcase(data)

    # The input is structured. The simplest thing is to reduce on it in text
    # form, so that we do not need to have references to nested things etc.
    text = json.dumps(data)
    assert not check_testcase(json.loads(text))
    print(f'[reducing, starting from size {len(text)}]')

    def iteration(text):
        # Find ',' delimiters, and reduce using it it, starting from the start
        # and going to the end (doing this on the text lets us handle all
        # nested structure in a single loop)
        print(f'[reduction iteration begins]')

        def find_delimiter_at_same_scope(text, delimiters, i):
            '''
            Given a reference to the first comma here:
                [1,2,3]
                  ^
            We return a reference to the next one, or to a ] if there is none.
            This handles scoping, that is,
                [1,[5,6],3]
                  ^
            That will return the comma before the '3'.
            '''
            nesting = 0
            while True:
                curr = text[i]
                if curr in delimiters and nesting == 0:
                    return i
                elif curr == '[':
                    nesting += 1
                elif curr == ']':
                    nesting -= 1
                i += 1

        # Reduce starting from commas.
        i = 0
        while True:
            i = text.find(',', i)
            if i < 0:
                break
            # Look for the ], which might allow us to reduce all the tail of the
            # current array. Often the tails are ignored, so this is a big
            # speedup potentially.
            j = find_delimiter_at_same_scope(text, ']', i + 1)

            # We now have something like
            #   ,...,
            #   i   j
            # or
            #   ,...]
            #   i   j
            # And we can try a reduction by removing up to j.
            new_text = text[:i] + text[j:]
            if not check_testcase(json.loads(new_text)):
                # This is a successful reduction!
                text = new_text
                print(f'[reduced (large) to {len(text)}]')
                # Note that i can stay where it is.
                continue

            # The reduction failed. Try a smaller reduction, not all the way
            # to the end of the tail.
            j = find_delimiter_at_same_scope(text, ',]', i + 1)
            if text[j] == ',':
                new_text = text[:i] + text[j:]
                if not check_testcase(json.loads(new_text)):
                    text = new_text
                    print(f'[reduced (small) to {len(text)}]')
                    continue
            i += 1

        # Reduce starting from open braces. This handles removing the very first
        # element.
        i = 0
        while True:
            i = text.find('[', i)
            if i < 0:
                break
            j = find_delimiter_at_same_scope(text, ',]', i + 1)
            if text[j] == ',':
                # [..,  =>  [
                new_text = text[:i + 1] + text[j + 1:]
            else:
                # [..]  =>  []
                new_text = text[:i + 1] + text[j:]
            if not check_testcase(json.loads(new_text)):
                text = new_text
                print(f'[reduced (open) to {len(text)}]')
            i += 1

        # Reduce a singleton parent to a child,
        #  [[x]]  =>  [x]
        i = 0
        while True:
            i = text.find('[', i)
            if i < 0:
                break
            if text[i + 1] != '[':
                i += 1
                continue
            j = find_delimiter_at_same_scope(text, ']', i + 2)
            if text[j + 1] != ']':
                i += 1
                continue
            # We now have
            # [[..]]
            # i   j
            new_text = text[:i] + text[i + 1:j + 1] + text[j + 2:]
            if not check_testcase(json.loads(new_text)):
                text = new_text
                print(f'[reduced (singleton) to {len(text)}]')
                continue
            i += 1

        return text

    # Main loop: do iterations while we are still reducing.
    while True:
        reduced = iteration(text)
        if reduced == text:
            break
        text = reduced

    # Run and verify the final reduction.
    assert not check_testcase(json.loads(text))
    print('[reduction complete, reduced testcase written out]')


def main():
    total = 0
    seed = time.time() * os.getpid()
    random.seed(seed)

    while 1:
        seed = random.randint(0, 1 << 64)
        random.seed(seed)
        print(f'[iteration {total} (seed = {seed})]')
        total += 1

        # Generate a testcase.
        data = StructuredRandomData().root

        # Test it.
        if not check_testcase(data, silent=False):
            print('[testcase failed]')
            reduce(data)
            sys.exit(1)


main()
