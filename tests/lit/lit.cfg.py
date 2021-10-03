import lit.formats
import re

config.name = "Emscripten lit tests"
config.test_format = lit.formats.ShTest()

config.suffixes = ['.cpp', '.c']

config.test_source_root = config.src_root + '/tests/lit'
config.test_exec_root = config.src_root + '/tests/out'

# Replace all Binaryen tools with their absolute paths
#bin_dir = os.path.join(config.binaryen_build_root, 'bin')
#for tool_file in os.listdir(bin_dir):
    #tool_path = config.binaryen_build_root + '/bin/' + tool_file
    #tool = tool_file[:-4] if tool_file.endswith('.exe') else tool_file
for tool in ('emcc', 'em++', 'emar'):
    config.substitutions.append((re.escape(tool), config.src_root + '/' + tool))

# Also make the `not` and `foreach` commands available
for tool in ('not', 'foreach'):
    tool_file = config.src_root + '/tests/lit/' + tool + '.py'
    python = sys.executable.replace('\\', '/')
    config.substitutions.append((tool, python + ' ' + tool_file))

print(config.substitutions)
