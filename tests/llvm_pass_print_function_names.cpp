#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct PrintFunctionNames : public FunctionPass {
  static char ID;
  PrintFunctionNames() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    errs() << "PrintFunctionNames: ";
    errs().write_escaped(F.getName()) << '\n';
    return false;
  }
}; // end of struct Hello
}  // end of anonymous namespace

char PrintFunctionNames::ID = 0;
static RegisterPass<PrintFunctionNames> X("print-function-names", "Print function names",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
