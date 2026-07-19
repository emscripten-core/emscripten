# Configuration file for emscripten test suite.
#
# Alternative JS engines to use during testing:
#
# NODE_JS_TEST = 'node' # executable
# SPIDERMONKEY_ENGINE = ['js'] # executable
# V8_ENGINE = 'd8' # executable
#
# All JS engines to use when running the automatic tests. Not all the engines in
# this list must exist (if they don't, they will be skipped in the test runner).
#
# JS_ENGINES = [NODE_JS_TEST] # add V8_ENGINE or SPIDERMONKEY_ENGINE if you have them installed too.
#
# WASMER = 'wasmer'
# WASMTIME = 'wasmtime'
#
# Wasm engines to use in STANDALONE_WASM tests.
#
# WASM_ENGINES = [] # add WASMER or WASMTIME if you have them installed
