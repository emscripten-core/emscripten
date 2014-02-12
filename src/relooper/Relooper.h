/*
This is an optimized C++ implemention of the Relooper algorithm originally
developed as part of Emscripten. This implementation includes optimizations
added since the original academic paper [1] was published about it, and is
written in an LLVM-friendly way with the goal of inclusion in upstream
LLVM.

[1] Alon Zakai. 2011. Emscripten: an LLVM-to-JavaScript compiler. In Proceedings of the ACM international conference companion on Object oriented programming systems languages and applications companion (SPLASH '11). ACM, New York, NY, USA, 301-312. DOI=10.1145/2048147.2048224 http://doi.acm.org/10.1145/2048147.2048224
*/

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus

#include <map>
#include <deque>
#include <set>

struct Block;
struct Shape;

// Info about a branching from one block to another
struct Branch {
  enum FlowType {
    Direct = 0, // We will directly reach the right location through other means, no need for continue or break
    Break = 1,
    Continue = 2
  };
  Shape *Ancestor; // If not NULL, this shape is the relevant one for purposes of getting to the target block. We break or continue on it
  Branch::FlowType Type; // If Ancestor is not NULL, this says whether to break or continue
  bool Labeled; // If a break or continue, whether we need to use a label
  const char *Condition; // The condition for which we branch. For example, "my_var == 1". Conditions are checked one by one. One of the conditions should have NULL as the condition, in which case it is the default
  const char *Code; // If provided, code that is run right before the branch is taken. This is useful for phis

  Branch(const char *ConditionInit, const char *CodeInit=NULL);
  ~Branch();

  // Prints out the branch
  void Render(Block *Target, bool SetLabel);
};

typedef std::set<Block*> BlockSet;
typedef std::map<Block*, Branch*> BlockBranchMap;

// Represents a basic block of code - some instructions that end with a
// control flow modifier (a branch, return or throw).
struct Block {
  // Branches become processed after we finish the shape relevant to them. For example,
  // when we recreate a loop, branches to the loop start become continues and are now
  // processed. When we calculate what shape to generate from a set of blocks, we ignore
  // processed branches.
  // Blocks own the Branch objects they use, and destroy them when done.
  BlockBranchMap BranchesOut;
  BlockSet BranchesIn;
  BlockBranchMap ProcessedBranchesOut;
  BlockSet ProcessedBranchesIn;
  Shape *Parent; // The shape we are directly inside
  int Id; // A unique identifier, defined when added to relooper. Note that this uniquely identifies a *logical* block - if we split it, the two instances have the same content *and* the same Id
  const char *Code; // The string representation of the code in this block. Owning pointer (we copy the input)
  const char *BranchVar; // If we have more than one branch out, the variable whose value determines where we go
  bool IsCheckedMultipleEntry; // If true, we are a multiple entry, so reaching us requires setting the label variable

  Block(const char *CodeInit, const char *BranchVarInit);
  ~Block();

  void AddBranchTo(Block *Target, const char *Condition, const char *Code=NULL);

  // Prints out the instructions code and branchings
  void Render(bool InLoop);
};

// Represents a structured control flow shape, one of
//
//  Simple: No control flow at all, just instructions. If several
//          blocks, then 
//
//  Multiple: A shape with more than one entry. If the next block to
//            be entered is among them, we run it and continue to
//            the next shape, otherwise we continue immediately to the
//            next shape.
//
//  Loop: An infinite loop.
//
//  Emulated: Control flow is managed by a switch in a loop. This
//            is necessary in some cases, for example when control
//            flow is not known until runtime (indirect branches,
//            setjmp returns, etc.)
//

struct SimpleShape;
struct LabeledShape;
struct MultipleShape;
struct LoopShape;
struct EmulatedShape;

struct Shape {
  int Id; // A unique identifier. Used to identify loops, labels are Lx where x is the Id. Defined when added to relooper
  Shape *Next; // The shape that will appear in the code right after this one
  Shape *Natural; // The shape that control flow gets to naturally (if there is Next, then this is Next)

  enum ShapeType {
    Simple,
    Multiple,
    Loop,
    Emulated
  };
  ShapeType Type;

  Shape(ShapeType TypeInit) : Id(-1), Next(NULL), Type(TypeInit) {}
  virtual ~Shape() {}

  virtual void Render(bool InLoop) = 0;

  static SimpleShape *IsSimple(Shape *It) { return It && It->Type == Simple ? (SimpleShape*)It : NULL; }
  static MultipleShape *IsMultiple(Shape *It) { return It && It->Type == Multiple ? (MultipleShape*)It : NULL; }
  static LoopShape *IsLoop(Shape *It) { return It && It->Type == Loop ? (LoopShape*)It : NULL; }
  static LabeledShape *IsLabeled(Shape *It) { return IsMultiple(It) || IsLoop(It) ? (LabeledShape*)It : NULL; }
  static EmulatedShape *IsEmulated(Shape *It) { return It && It->Type == Emulated ? (EmulatedShape*)It : NULL; }
};

struct SimpleShape : public Shape {
  Block *Inner;

  SimpleShape() : Shape(Simple), Inner(NULL) {}
  void Render(bool InLoop) {
    Inner->Render(InLoop);
    if (Next) Next->Render(InLoop);
  }
};

typedef std::map<Block*, Shape*> BlockShapeMap;

// A shape that may be implemented with a labeled loop.
struct LabeledShape : public Shape {
  bool Labeled; // If we have a loop, whether it needs to be labeled

  LabeledShape(ShapeType TypeInit) : Shape(TypeInit), Labeled(false) {}
};

struct MultipleShape : public LabeledShape {
  BlockShapeMap InnerMap; // entry block -> shape
  int NeedLoop; // If we have branches, we need a loop. This is a counter of loop requirements,
                // if we optimize it to 0, the loop is unneeded

  MultipleShape() : LabeledShape(Multiple), NeedLoop(0) {}

  void RenderLoopPrefix();
  void RenderLoopPostfix();

  void Render(bool InLoop);
};

struct LoopShape : public LabeledShape {
  Shape *Inner;

  LoopShape() : LabeledShape(Loop), Inner(NULL) {}
  void Render(bool InLoop);
};

// TODO EmulatedShape is only partially functional. Currently it can be used for the
//      entire set of blocks being relooped, but not subsets.
struct EmulatedShape : public LabeledShape {
  Block *Entry;
  BlockSet Blocks;

  EmulatedShape() : LabeledShape(Emulated) { Labeled = true; }
  void Render(bool InLoop);
};

// Implements the relooper algorithm for a function's blocks.
//
// Usage:
//  1. Instantiate this struct.
//  2. Call AddBlock with the blocks you have. Each should already
//     have its branchings in specified (the branchings out will
//     be calculated by the relooper).
//  3. Call Render().
//
// Implementation details: The Relooper instance has
// ownership of the blocks and shapes, and frees them when done.
struct Relooper {
  std::deque<Block*> Blocks;
  std::deque<Shape*> Shapes;
  Shape *Root;
  bool Emulate;
  int BlockIdCounter;
  int ShapeIdCounter;

  Relooper();
  ~Relooper();

  void AddBlock(Block *New, int Id=-1);

  // Calculates the shapes
  void Calculate(Block *Entry);

  // Renders the result.
  void Render();

  // Sets the global buffer all printing goes to. Must call this or MakeOutputBuffer.
  // XXX: this is deprecated, see MakeOutputBuffer
  static void SetOutputBuffer(char *Buffer, int Size);

  // Creates an internal output buffer. Must call this or SetOutputBuffer. Size is
  // a hint for the initial size of the buffer, it can be resized later one demand.
  // For that reason this is more recommended than SetOutputBuffer.
  static void MakeOutputBuffer(int Size);

  static char *GetOutputBuffer();

  // Sets asm.js mode on or off (default is off)
  static void SetAsmJSMode(int On);

  // Sets whether we must emulate everything with switch-loop code
  void SetEmulate(int E) { Emulate = E; }
};

typedef std::map<Block*, BlockSet> BlockBlockSetMap;

#if DEBUG
struct Debugging {
  static void Dump(BlockSet &Blocks, const char *prefix=NULL);
  static void Dump(Shape *S, const char *prefix=NULL);
};
#endif

#endif // __cplusplus

// C API - useful for binding to other languages

#ifdef _WIN32
  #ifdef RELOOPERDLL_EXPORTS
    #define RELOOPERDLL_API __declspec(dllexport)
  #else
    #define RELOOPERDLL_API __declspec(dllimport)
  #endif
#else
  #define RELOOPERDLL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

RELOOPERDLL_API void  rl_set_output_buffer(char *buffer, int size);
RELOOPERDLL_API void  rl_make_output_buffer(int size);
RELOOPERDLL_API void  rl_set_asm_js_mode(int on);
RELOOPERDLL_API void *rl_new_block(const char *text, const char *branch_var);
RELOOPERDLL_API void  rl_delete_block(void *block);
RELOOPERDLL_API void  rl_block_add_branch_to(void *from, void *to, const char *condition, const char *code);
RELOOPERDLL_API void *rl_new_relooper();
RELOOPERDLL_API void  rl_delete_relooper(void *relooper);
RELOOPERDLL_API void  rl_relooper_add_block(void *relooper, void *block);
RELOOPERDLL_API void  rl_relooper_calculate(void *relooper, void *entry);
RELOOPERDLL_API void  rl_relooper_render(void *relooper);

#ifdef __cplusplus
}
#endif

