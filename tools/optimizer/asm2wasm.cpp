
#include "simple_ast.h"
#include "wasm.h"

using namespace cashew;
using namespace wasm;

class Asm2WasmModule : public wasm::Module {
  wasm::Arena allocator;

  // globals

  unsigned nextGlobal; // next place to put a global
  unsigned maxGlobal; // highest address we can put a global
  std::map<IString, unsigned> globalAddresses;

  void allocateGlobal(IString name) {
    assert(globalAddresses.find(name) == globalAddresses.end());
    globalAddresses[name] = nextGlobal;
    nextGlobal += 8;
    assert(nextGlobal < maxGlobal);
  }

  // function types. we fill in this information as we see
  // uses, in the first pass

  std::map<IString, GeneralType> importedFunctionTypes;

public:
  Asm2WasmModule() : nextGlobal(8), maxGlobal(100) {}
  void processAsm(Ref ast);

private:
  BasicType wasmTypeFromCoercion(Ref ast) {
    if (ast[0] == BINARY) return BasicType::i32;
    if (ast[0] == UNARY_PREFIX) return BasicType::f64;
    abort();
  }

  Function* processFunction(Ref ast);
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
  assert(body[0][0] == STRING && body[0][1]->getIString() == IString("use asm"));

  auto addImport = [&](IString name, Ref imported, BasicType type) {
    assert(imported[0] == DOT);
    Ref module = imported[1];
    if (module[0] == DOT) {
      // we can have (global.Math).floor; skip the 'Math'
      module = module[1];
    }
    assert(module[0] == STRING);
    Import import;
    import.name = name;
    import.module = module[1]->getIString();
    import.base = imported[2]->getIString();
    if (type != BasicType::none) {
      import.type.basic = type;
    } else {
      import.type = importedFunctionTypes[name];
    }
    imports.push_back(import);
  };

  // first pass - do almost everything, but function imports

  for (unsigned i = 1; i < body->size(); i++) {
    Ref curr = body[i];
    if (curr[0] == VAR) {
      // import, global, or table
      for (unsigned j = 0; j < curr[1]->size(); j++) {
        Ref pair = curr[1][j];
        IString name = pair[0]->getIString();
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
          // we have to do this later, since we don't know the type yet.
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
      functions.push_back(processFunction(curr));
    } else if (curr[0] == RETURN) {
      // exports
      Ref object = curr[1];
      for (auto pair : *object->obj) {
        IString key = pair.first;
        Ref value = pair.second;
        assert(value[0] == STRING);
        Export export_;
        export_.name = key;
        export_.value = value[1]->getIString();
        exports.push_back(export_);
      }
    }
  }

  // second pass - function imports

  for (unsigned i = 1; i < body->size(); i++) {
    Ref curr = body[i];
    if (curr[0] == VAR) {
      for (unsigned j = 0; j < curr[1]->size(); j++) {
        Ref pair = curr[1][j];
        IString name = pair[0]->getIString();
        Ref value = pair[1];
        if (value[0] == DOT) {
          // function import
          // we can do this now, after having seen the type based on the use
          addImport(name, value, BasicType::none);
        }
      }
    }
  }
}

Function* Asm2WasmModule::processFunction(Ref ast) {
  auto function = allocator.alloc<Function>();
  function->name = ast[1]->getIString();
  Ref params = ast[2];
  Ref body = ast[3];
  for (unsigned i = 0; i < params->size(); i++) {
    Ref curr = body[i];
    assert(curr[0] == ASSIGN && curr[2][0] == NAME);
    function->params.emplace_back(curr[2][1]->getIString(), wasmTypeFromCoercion(curr[3]));
  }
  unsigned start = params->size();
  while (start < body->size() && body[start][0] == VAR) {
    Ref curr = body[start];
    for (unsigned j = 0; j < curr[1]->size(); j++) {
      Ref pair = curr[1][j];
      function->locals.emplace_back(pair[0]->getIString(), wasmTypeFromCoercion(pair[1]));
    }
    start++;
  }
  function->returnType = BasicType::none; // updated if we see a return
  function->body = processStatements(body, start);
  return function;
}

