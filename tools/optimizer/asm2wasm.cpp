
#include "simple_ast.h"
#include "wasm.h"

static void abort_on(std::string why, Ref element) {
  std::cerr << why << ' ';
  element->stringify(std::cerr);
  std::cerr << "\n";
  abort();
}

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
  Asm2WasmModule() : nextGlobal(8), maxGlobal(1000) {}
  void processAsm(Ref ast);

private:
  BasicType wasmTypeFromCoercion(Ref ast) {
    if (ast[0] == BINARY) return BasicType::i32;
    if (ast[0] == UNARY_PREFIX) return BasicType::f64;
    abort();
  }

  bool isUnsignedCoercion(Ref ast) {
    if (ast[0] == BINARY && ast[1] == TRSHIFT) return true;
    return false;
  }

  BinaryOp wasmBinaryOp(IString op, Ref left, Ref right) {
    if (op == PLUS) return BinaryOp::Add;
    if (op == MINUS) return BinaryOp::Sub;
    if (op == MUL) return BinaryOp::Mul;
    if (op == AND) return BinaryOp::And;
    if (op == OR) return BinaryOp::Or;
    if (op == XOR) return BinaryOp::Xor;
    if (op == LSHIFT) return BinaryOp::Shl;
    if (op == RSHIFT) return BinaryOp::ShrS;
    if (op == TRSHIFT) return BinaryOp::ShrU;
    if (op == DIV) {
      return isUnsignedCoercion(left) ? BinaryOp::DivU : BinaryOp::DivS;
    }
  }

  Function* processFunction(Ref ast);
};

void Asm2WasmModule::processAsm(Ref ast) {
  assert(ast[0] == TOPLEVEL);
  Ref asmFunction = ast[1][0];
  assert(asmFunction[0] == DEFUN);
  Ref body = asmFunction[3];
  assert(body[0][0] == STAT && body[0][1][0] == STRING && body[0][1][1]->getIString() == IString("use asm"));

  auto addImport = [&](IString name, Ref imported, BasicType type) {
    assert(imported[0] == DOT);
    Ref module = imported[1];
    if (module[0] == DOT) {
      // we can have (global.Math).floor; skip the 'Math'
      module = module[1];
    }
    assert(module[0] == NAME);
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
        if (value[0] == NUM) {
          // global int
          assert(value[1]->getInteger() == 0);
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
          abort_on("invalid var element", pair);
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
  // processors
  std::function<Expression* (Ref)> process = [&](Ref ast) -> Expression* {
    IString what = ast[0]->getIString();
    if (what == ASSIGN) {
      if (ast[2][0] == NAME) {
        auto ret = allocator.alloc<SetLocal>();
        ret->id = ast[2][1]->getIString();
        ret->value = process(ast[3]);
        return ret;
      } else {
        abort();
      }
    } else if (what == BINARY) {
      auto ret = allocator.alloc<Binary>();
      ret->op = wasmBinaryOp(ast[1]->getIString(), ast[2], ast[3]);
      ret->left = process(ast[2]);
      ret->right = process(ast[3]);
      return ret;
    } else if (what == NUM) {
      auto ret = allocator.alloc<Const>();
      ret->value.type = BasicType::i32;
      ret->value.i32 = ast[1]->getInteger();
      return ret;
    } else if (what == UNARY_PREFIX) {
      if (ast[2][0] == NUM) {
        auto ret = allocator.alloc<Const>();
        ret->value.type = BasicType::f64;
        ret->value.f64 = ast[2][1]->getNumber();
        return ret;
      }
      abort();
    }
    abort();
  };
  auto processStatements = [&](Ref ast, unsigned from) {
    auto block = allocator.alloc<Block>();
    for (unsigned i = from; i < ast->size(); i++) {
      block->list.push_back(process(ast[i]));
    }
    return block;
  };
  // body
  function->body = processStatements(body, start);
  return function;
}

int main(int argc, char **argv) {
  char *infile = argv[1];

  printf("loading '%s'...\n", infile);
  FILE *f = fopen(argv[1], "r");
  assert(f);
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  char *input = new char[size+1];
  rewind(f);
  int num = fread(input, 1, size, f);
  // On Windows, ftell() gives the byte position (\r\n counts as two bytes), but when
  // reading, fread() returns the number of characters read (\r\n is read as one char \n, and counted as one),
  // so return value of fread can be less than size reported by ftell, and that is normal.
  assert((num > 0 || size == 0) && num <= size);
  fclose(f);
  input[num] = 0;

  /*
  // Separate asm modules look like
  //
  //    Module["asm"] = (function(global, env, buffer) {
  //
  // , we can remove the part until the function.
  */

  printf("parsing...\n");
  cashew::Parser<Ref, ValueBuilder> builder;
  Ref asmjs = builder.parseToplevel(input);

  //asmjs->stringify(std::cout);
  //std::cout << "\n";

  printf("wasming...\n");
  Asm2WasmModule wasm;
  wasm.processAsm(asmjs);

  printf("done.\n");
  printf("TODO: get memory for globals, and clear it to zero\n");
}

