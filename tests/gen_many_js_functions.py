import sys

NUM_FUNCS_TO_GENERATE = 1000

def func_name(i):
	return 'thisIsAFunctionWithVeryLongFunctionNameThatWouldBeGreatToBeMinifiedWhenImportingToAsmJsOrWasmSideCodeToCallOtherwiseCodeSizesWillBeLargeAndNetworkTransfersBecomeVerySlowThatUsersWillGoAwayAndVisitSomeOtherSiteInsteadAndThenWebAssemblyDeveloperIsSadOrEvenWorseNobodyNoticesButInternetPipesWillGetMoreCongestedWhichContributesToGlobalWarmingAndThenEveryoneElseWillBeSadAsWellEspeciallyThePolarBearsAndPenguinsJustThinkAboutThePenguins' + str(i+1)

def generate_js_library_with_lots_of_functions(out_file):
	f = open(out_file, 'w')

	f.write('var FunctionsLibrary = {\n')

	for i in range(NUM_FUNCS_TO_GENERATE):
		f.write('  ' + func_name(i) + ': function() { return ' + str(i+1) + '; },\n')

	f.write('}\n');
	f.write('mergeInto(LibraryManager.library, FunctionsLibrary);\n');
	f.close()

def generate_c_program_that_calls_js_library_with_lots_of_functions(out_file):
	f = open(out_file, 'w')

	f.write('#include <stdio.h>\n\n')

	for i in range(NUM_FUNCS_TO_GENERATE):
		f.write('int ' + func_name(i) + '(void);\n')

	f.write('\nint main() {\n')
	f.write('  int sum = 0;\n')

	for i in range(NUM_FUNCS_TO_GENERATE):
		f.write('  sum += ' + func_name(i) + '();\n')

	f.write('\n  printf("Sum of numbers from 1 to ' + str(NUM_FUNCS_TO_GENERATE) + ': %d (expected ' + str((NUM_FUNCS_TO_GENERATE * (NUM_FUNCS_TO_GENERATE+1))/2) + ')\\n", sum);\n');
	f.write('}\n');
	f.close()

if __name__ == '__main__':
	generate_js_library_with_lots_of_functions(sys.argv[1])
	generate_c_program_that_calls_js_library_with_lots_of_functions(sys.argv[2])
