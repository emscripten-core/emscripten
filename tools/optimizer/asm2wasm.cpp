
#include "simple_ast.h"
#include "wasm.h"

IString GLOBAL("global"), NAN_("NaN"), INFINITY_("Infinity");

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
  struct MappedGlobal {
    unsigned address;
    BasicType type;
    MappedGlobal() : address(0), type(none) {}
    MappedGlobal(unsigned address, BasicType type) : address(address), type(type) {}
  };
  std::map<IString, MappedGlobal> mappedGlobals;

  void allocateGlobal(IString name, BasicType type) {
    assert(mappedGlobals.find(name) == mappedGlobals.end());
    mappedGlobals.emplace(name, MappedGlobal(nextGlobal, type));
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
  BasicType detectWasmType(Ref ast) {
    if (ast[0] == BINARY || ast[0] == NUM) return BasicType::i32;
    if (ast[0] == UNARY_PREFIX) return BasicType::f64;
    abort_on("confused detectWasmType", ast);
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
    // special-case some asm builtins
    if (import.module == GLOBAL && (import.base == NAN_ || import.base == INFINITY_)) {
      import.type.basic = BasicType::f64;
    } else if (type != BasicType::none) {
      import.type.basic = type;
    } else {
      assert(importedFunctionTypes.find(name) != importedFunctionTypes.end());
      import.type = importedFunctionTypes[name];
    }
    imports.emplace(name, import);
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
          allocateGlobal(name, BasicType::i32);
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
            allocateGlobal(name, BasicType::f64);
          } else {
            // import
            addImport(name, import, BasicType::f64);
          }
        } else if (value[0] == DOT) {
          // function import
          // we have to do this later, since we don't know the type yet.
        } else if (value[0] == NEW) {
          // ignore imports of typed arrays, but note the names of the arrays
          // XXX
        } else if (value[0] == ARRAY) {
          // function table
          Ref contents = value[1];
          for (unsigned k = 0; k < contents->size(); k++) {
            table.vars.push_back(contents[k][1]->getIString());
          }
        } else {
          abort_on("invalid var element", pair);
        }
      }
    } else if (curr[0] == DEFUN) {
      // function
      functions.push_back(processFunction(curr));
    } else if (curr[0] == RETURN) {
      // exports
      Ref object = curr[1];
      Ref contents = curr[1][1];
      for (unsigned k = 0; k < contents->size(); k++) {
        Ref pair = contents[k];
        IString key = pair[0]->getIString();
        Ref value = pair[1];
        assert(value[0] == NAME);
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

  IStringSet functionVariables; // params or locals 

  for (unsigned i = 0; i < params->size(); i++) {
    Ref curr = body[i];
    assert(curr[0] == STAT);
    curr = curr[1];
    assert(curr[0] == ASSIGN && curr[2][0] == NAME);
    IString name = curr[2][1]->getIString();
    function->params.emplace_back(name, detectWasmType(curr[3]));
    functionVariables.insert(name);
  }
  unsigned start = params->size();
  while (start < body->size() && body[start][0] == VAR) {
    Ref curr = body[start];
    for (unsigned j = 0; j < curr[1]->size(); j++) {
      Ref pair = curr[1][j];
      IString name = pair[0]->getIString();
      function->locals.emplace_back(name, detectWasmType(pair[1]));
      functionVariables.insert(name);
    }
    start++;
  }
  function->result = BasicType::none; // updated if we see a return
  // processors
  std::function<Expression* (Ref)> process = [&](Ref ast) -> Expression* {
    IString what = ast[0]->getIString();
    if (what == STAT) {
      return process(ast[1]); // and drop return value, if any
    } else if (what == ASSIGN) {
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
    } else if (what == NAME) {
      IString name = ast[1]->getIString();
      if (functionVariables.has(name)) {
        // var in scope
        auto ret = allocator.alloc<GetLocal>();
        ret->id = name;
        return ret;
      } else if (imports.find(name) != imports.end()) {
        // imported var
        Import& import = imports[name];
        abort(); // XXX
      }
      // global var, do a load from memory
      assert(mappedGlobals.find(name) != mappedGlobals.end());
      MappedGlobal global = mappedGlobals[name];
      auto ret = allocator.alloc<Load>();
      ret->bytes = getBasicTypeSize(global.type);
      ret->signed_ = true; // but doesn't matter
      ret->offset = 0;
      ret->align = ret->bytes;
      auto ptr = allocator.alloc<Const>();
      ptr->value.type = BasicType::i32; // XXX for wasm64, need 64
      ptr->value.i32 = global.address;
      ret->ptr = ptr;
      return ret;
    } else if (what == UNARY_PREFIX) {
      if (ast[2][0] == NUM) {
        auto ret = allocator.alloc<Const>();
        ret->value.type = BasicType::f64;
        ret->value.f64 = ast[2][1]->getNumber();
        return ret;
      }
      abort_on("confusing unary-prefix", ast[2]);
    }
    abort_on("confusing expression", ast);
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

  printf("printing...\n");
  wasm.print(std::cout);

  printf("done.\n");
  printf("TODO: get memory for globals, and clear it to zero\n");
  printf("TODO: assert on no aliasing function pointers\n");
}

