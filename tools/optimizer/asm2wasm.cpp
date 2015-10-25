
#include "simple_ast.h"
#include "wasm.h"

using namespace cashew;
using namespace wasm;

class Asm2WasmModule : public Module {
  unsigned nextGlobal; // next place to put a global
  unsigned maxGlobal; // highest address we can put a global
  std::map<IString, unsigned> globalAddresses;

  void allocateGlobal(IString name) {
    assert(globalAddresses.find(name) == globalAddresses.end());
    globalAddresses[name] = nextGlobal;
    nextGlobal += 8;
    assert(nextGlobal < maxGlobal);
  }

  GeneralType getFunctionType(IString module, IString base) {
    // XXX need an oracle here
  }

public:
  Asm2WasmModule() : nextGlobal(8), maxGlobal(100) {}
  void processAsm(Ref ast);
};

void Asm2WasmModule::processAsm(Ref ast) {
  // Separate asm modules look like
  //
  //    Module["asm"] = (function(global, env, buffer) {
  //
  // , we can remove the part until the function.
  assert(ast[0] == ASSIGN && ast[2][0] == FUNCTION);
  Ref asmFunction = ast[2];
  Ref body = asmFunction[2];
  assert(body[0][0] == STRING && body[0][1]->getString() == IString("use asm"));

  auto addImport = [&](IString name, Ref import, BasicType type) {
    assert(import[0] == DOT);
    Ref module = import[1];
    if (module[0] == DOT) {
      // we can have (global.Math).floor; skip the 'Math'
      module = module[1];
    }
    assert(module[0] == STRING);
    auto import = allocator.alloc<Import>();
    import->name = name;
    import->module = module[1]->getString();
    import->base = import[2]->getString();
    if (type != BasicType::none) {
      import->type.basic = type;
    } else {
      import->type = getFunctionType(import->module, import->base);
    }
    imports.push_back(import);
  };

  for (var i = 1; i < body->size(); i++) {
    Ref curr = body[i];
    if (curr[0] == VAR) {
      // import, global, or table
      for (var j = 0; j < curr[1].size(); j++) {
        Ref pair = curr[1][j];
        IString name = pair[0]->getString();
        Ref value = pair[1];
        if (value->isNumber()) {
          // global int
          assert(value->getNumber() == 0);
          allocateGlobal(name);
        } else if (value[0] == BINARY) {
          // int import
          assert(value[1] == OR && value[3][0] == NUM && value[3][1]->getNumber() == 0);
          Ref import = value[2]; // env.what
          addImport(name, import, BasicType::i32);
        } else if (value[0] == UNARY_PREFIX) {
          // double import or global
          assert(value[1] == PLUS);
          Ref import = value[2];
          if (import[0] == NUM) {
            // global
            assert(import[1]->getNumber() == 0);
            allocateGlobal(name);
          } else {
            // import
            addImport(name, import, BasicType::f64);
          }
        } else if (value[0] == DOT) {
          // function import
          addImport(name, value, BasicType::none);
        } else if (value[0] == NEW) {
          // ignore imports of typed arrays, but note the names of the arrays
          // XXX
        } else {
          abort();
        }
      }
    } else if (curr[0] == FUNCTION) {
      // function
    } else if (curr[0] == RETURN) {
      // exports
    }
  }
}

