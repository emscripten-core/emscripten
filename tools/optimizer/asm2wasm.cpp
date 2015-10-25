
#include "simple_ast.h"
#include "wasm.h"

using namespace cashew;
using namespace wasm;

class Asm2WasmModule : public Module {
public:
  void process(Ref ast);
};

void Asm2WasmModule::process(Ref ast) {
  // Separate asm modules look like
  //
  //    Module["asm"] = (function(global, env, buffer) {
  //
  // , we can remove the part until the function.
  assert(ast[0] == ASSIGN && ast[2][0] == FUNCTION);
  Ref asmFunction = ast[2];
}

