#!/usr/bin/env python3

# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import sys

NUM_FUNCS_TO_GENERATE = 1000


def func_name(i):
  return 'thisIsAFunctionWithVeryLongFunctionNameThatWouldBeGreatToBeMinifiedWhenImportingToAsmJsOrWasmSideCodeToCallOtherwiseCodeSizesWillBeLargeAndNetworkTransfersBecomeVerySlowThatUsersWillGoAwayAndVisitSomeOtherSiteInsteadAndThenWebAssemblyDeveloperIsSadOrEvenWorseNobodyNoticesButInternetPipesWillGetMoreCongestedWhichContributesToGlobalWarmingAndThenEveryoneElseWillBeSadAsWellEspeciallyThePolarBearsAndPenguinsJustThinkAboutThePenguins' + str(i + 1)


def generate_js_library_with_lots_of_functions(out_file):
  with open(out_file, 'w') as f:
    f.write('var FunctionsLibrary = {\n')

    for i in range(NUM_FUNCS_TO_GENERATE):
      f.write('  ' + func_name(i) + ': function() { return ' + str(i + 1) + '; },\n')

    f.write('}\n')
    f.write('addToLibrary(FunctionsLibrary);\n')


def generate_c_program_that_calls_js_library_with_lots_of_functions(out_file):
  with open(out_file, 'w') as f:
    f.write('#include <stdio.h>\n\n')

    for i in range(NUM_FUNCS_TO_GENERATE):
      f.write('int ' + func_name(i) + '(void);\n')

    f.write('\nint main() {\n')
    f.write('  int sum = 0;\n')

    for i in range(NUM_FUNCS_TO_GENERATE):
      f.write('  sum += ' + func_name(i) + '();\n')

    f.write('\n  printf("Sum of numbers from 1 to ' + str(NUM_FUNCS_TO_GENERATE) + ': %d (expected ' + str(int((NUM_FUNCS_TO_GENERATE * (NUM_FUNCS_TO_GENERATE + 1)) / 2)) + ')\\n", sum);\n')
    f.write('}\n')


if __name__ == '__main__':
  generate_js_library_with_lots_of_functions(sys.argv[1])
  generate_c_program_that_calls_js_library_with_lots_of_functions(sys.argv[2])
