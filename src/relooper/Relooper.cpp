
#include "Relooper.h"

#include <string.h>
#include <stdlib.h>
#include <list>
#include <stack>

#include "ministring.h"

// TODO: move all set to unorderedset

#if DEBUG
static void PrintDebug(const char *Format, ...);
#define DebugDump(x, ...) Debugging::Dump(x, __VA_ARGS__)
#else
#define PrintDebug(x, ...)
#define DebugDump(x, ...)
#endif

struct Indenter {
  static int CurrIndent;

  static void Indent() { CurrIndent++; }
  static void Unindent() { CurrIndent--; }
};

static void PrintIndented(const char *Format, ...);
static void PutIndented(const char *String);

static char *OutputBufferRoot = NULL;
static char *OutputBuffer = NULL;
static int OutputBufferSize = 0;

void PrintIndented(const char *Format, ...) {
  assert(OutputBuffer);
  assert(OutputBuffer + Indenter::CurrIndent*2 - OutputBufferRoot < OutputBufferSize);
  for (int i = 0; i < Indenter::CurrIndent*2; i++, OutputBuffer++) *OutputBuffer = ' ';
  va_list Args;
  va_start(Args, Format);
  int left = OutputBufferSize - (OutputBuffer - OutputBufferRoot);
  int written = vsnprintf(OutputBuffer, left, Format, Args);
  assert(written < left);
  OutputBuffer += written;
  va_end(Args);
}

void PutIndented(const char *String) {
  assert(OutputBuffer);
  assert(OutputBuffer + Indenter::CurrIndent*2 - OutputBufferRoot < OutputBufferSize);
  for (int i = 0; i < Indenter::CurrIndent*2; i++, OutputBuffer++) *OutputBuffer = ' ';
  int left = OutputBufferSize - (OutputBuffer - OutputBufferRoot);
  int needed = strlen(String)+1;
  assert(needed < left);
  strcpy(OutputBuffer, String);
  OutputBuffer += strlen(String);
  *OutputBuffer++ = '\n';
  *OutputBuffer = 0;
}

static int AsmJS = 0;

// Indenter

#if EMSCRIPTEN
int Indenter::CurrIndent = 1;
#else
int Indenter::CurrIndent = 0;
#endif

// Branch

Branch::Branch(const char *ConditionInit, const char *CodeInit) : Ancestor(NULL), Labeled(false) {
  Condition = ConditionInit ? strdup(ConditionInit) : NULL;
  Code = CodeInit ? strdup(CodeInit) : NULL;
}

Branch::~Branch() {
  if (Condition) free((void*)Condition);
  if (Code) free((void*)Code);
}

void Branch::Render(Block *Target, bool SetLabel) {
  if (Code) PrintIndented("%s\n", Code);
  if (SetLabel) PrintIndented("label = %d;\n", Target->Id);
  if (Ancestor) {
    if (Type != Direct) {
      if (Labeled) {
        PrintIndented("%s L%d;\n", Type == Break ? "break" : "continue", Ancestor->Id);
      } else {
        PrintIndented("%s;\n", Type == Break ? "break" : "continue");
      }
    }
  }
}

// Block

int Block::IdCounter = 1; // 0 is reserved for clearings

Block::Block(const char *CodeInit) : Parent(NULL), Id(Block::IdCounter++), DefaultTarget(NULL), IsCheckedMultipleEntry(false) {
  Code = strdup(CodeInit);
}

Block::~Block() {
  if (Code) free((void*)Code);
  for (BlockBranchMap::iterator iter = ProcessedBranchesIn.begin(); iter != ProcessedBranchesIn.end(); iter++) {
    delete iter->second;
  }
  for (BlockBranchMap::iterator iter = ProcessedBranchesOut.begin(); iter != ProcessedBranchesOut.end(); iter++) {
    delete iter->second;
  }
  // XXX If not reachable, expected to have branches here. But need to clean them up to prevent leaks!
}

void Block::AddBranchTo(Block *Target, const char *Condition, const char *Code) {
  assert(BranchesOut.find(Target) == BranchesOut.end()); // cannot add more than one branch to the same target
  BranchesOut[Target] = new Branch(Condition, Code);
}

void Block::Render(bool InLoop) {
  if (IsCheckedMultipleEntry && InLoop) {
    PrintIndented("label = 0;\n");
  }

  if (Code) {
    // Print code in an indented manner, even over multiple lines
    char *Start = const_cast<char*>(Code);
    while (*Start) {
      char *End = strchr(Start, '\n');
      if (End) *End = 0;
      PutIndented(Start);
      if (End) *End = '\n'; else break;
      Start = End+1;
    }
  }

  if (!ProcessedBranchesOut.size()) return;

  bool SetLabel = true; // in some cases it is clear we can avoid setting label, see later

  if (ProcessedBranchesOut.size() == 1 && ProcessedBranchesOut.begin()->second->Type == Branch::Direct) {
    SetLabel = false;
  }

  // A setting of the label variable (label = x) is necessary if it can
  // cause an impact. The main case is where we set label to x, then elsewhere
  // we check if label is equal to that value, i.e., that label is an entry
  // in a multiple block. We also need to reset the label when we enter
  // that block, so that each setting is a one-time action: consider
  //
  //    while (1) {
  //      if (check) label = 1;
  //      if (label == 1) { label = 0 }
  //    }
  //
  // (Note that this case is impossible due to fusing, but that is not
  // material here.) So setting to 0 is important just to clear the 1 for
  // future iterations.
  // TODO: When inside a loop, if necessary clear the label variable
  //       once on the top, and never do settings that are in effect clears

  // Fusing: If the next is a Multiple, we can fuse it with this block. Note
  // that we must be the Inner of a Simple, so fusing means joining a Simple
  // to a Multiple. What happens there is that all options in the Multiple
  // *must* appear in the Simple (the Simple is the only one reaching the
  // Multiple), so we can remove the Multiple and add its independent groups
  // into the Simple's branches.
  MultipleShape *Fused = Shape::IsMultiple(Parent->Next);
  if (Fused) {
    PrintDebug("Fusing Multiple to Simple\n");
    Parent->Next = Parent->Next->Next;
    Fused->RenderLoopPrefix();

    // When the Multiple has the same number of groups as we have branches,
    // they will all be fused, so it is safe to not set the label at all
    if (SetLabel && Fused->InnerMap.size() == ProcessedBranchesOut.size()) {
      SetLabel = false;
    }
  }

  // We must do this here, because blocks can be split and even comparing their Ids is not enough. We must check the conditions.
  for (BlockBranchMap::iterator iter = ProcessedBranchesOut.begin(); iter != ProcessedBranchesOut.end(); iter++) {
    if (!iter->second->Condition) {
      assert(!DefaultTarget); // Must be exactly one default
      DefaultTarget = iter->first;
    }
  }
  assert(DefaultTarget); // Must be a default

  ministring RemainingConditions;
  bool First = true;
  for (BlockBranchMap::iterator iter = ProcessedBranchesOut.begin();; iter++) {
    Block *Target;
    Branch *Details;
    if (iter != ProcessedBranchesOut.end()) {
      Target = iter->first;
      if (Target == DefaultTarget) continue; // done at the end
      Details = iter->second;
      assert(Details->Condition); // must have a condition if this is not the default target
    } else {
      Target = DefaultTarget;
      Details = ProcessedBranchesOut[DefaultTarget];
    }
    bool SetCurrLabel = SetLabel && Target->IsCheckedMultipleEntry;
    bool HasFusedContent = Fused && Fused->InnerMap.find(Target) != Fused->InnerMap.end();
    bool HasContent = SetCurrLabel || Details->Type != Branch::Direct || HasFusedContent || Details->Code;
    if (iter != ProcessedBranchesOut.end()) {
      // If there is nothing to show in this branch, omit the condition
      if (HasContent) {
        PrintIndented("%sif (%s) {\n", First ? "" : "} else ", Details->Condition);
        First = false;
      } else {
        if (RemainingConditions.size() > 0) RemainingConditions += " && ";
        RemainingConditions += "!(";
        RemainingConditions += Details->Condition;
        RemainingConditions += ")";
      }
    } else {
      if (HasContent) {
        if (RemainingConditions.size() > 0) {
          if (First) {
            PrintIndented("if (%s) {\n", RemainingConditions.c_str());
            First = false;
          } else {
            PrintIndented("} else if (%s) {\n", RemainingConditions.c_str());
          }
        } else if (!First) {
          PrintIndented("} else {\n");
        }
      }
    }
    if (!First) Indenter::Indent();
    Details->Render(Target, SetCurrLabel);
    if (HasFusedContent) {
      Fused->InnerMap.find(Target)->second->Render(InLoop);
    }
    if (!First) Indenter::Unindent();
    if (iter == ProcessedBranchesOut.end()) break;
  }
  if (!First) PrintIndented("}\n");

  if (Fused) {
    Fused->RenderLoopPostfix();
  }
}

// Shape

int Shape::IdCounter = 0;

// MultipleShape

void MultipleShape::RenderLoopPrefix() {
  if (NeedLoop) {
    if (Labeled) {
      PrintIndented("L%d: do {\n", Id);
    } else {
      PrintIndented("do {\n");
    }
    Indenter::Indent();
  }
}

void MultipleShape::RenderLoopPostfix() {
  if (NeedLoop) {
    Indenter::Unindent();
    PrintIndented("} while(0);\n");
  }
}

void MultipleShape::Render(bool InLoop) {
  RenderLoopPrefix();
  bool First = true;
  for (BlockShapeMap::iterator iter = InnerMap.begin(); iter != InnerMap.end(); iter++) {
    if (AsmJS) {
      PrintIndented("%sif ((label|0) == %d) {\n", First ? "" : "else ", iter->first->Id);
    } else {
      PrintIndented("%sif (label == %d) {\n", First ? "" : "else ", iter->first->Id);
    }
    First = false;
    Indenter::Indent();
    iter->second->Render(InLoop);
    Indenter::Unindent();
    PrintIndented("}\n");
  }
  RenderLoopPostfix();
  if (Next) Next->Render(InLoop);
};

// LoopShape

void LoopShape::Render(bool InLoop) {
  if (Labeled) {
    PrintIndented("L%d: while(1) {\n", Id);
  } else {
    PrintIndented("while(1) {\n");
  }
  Indenter::Indent();
  Inner->Render(true);
  Indenter::Unindent();
  PrintIndented("}\n");
  if (Next) Next->Render(InLoop);
};

/*
// EmulatedShape

void EmulatedShape::Render(bool InLoop) {
  PrintIndented("while(1) {\n");
  Indenter::Indent();
  PrintIndented("switch(label) {\n");
  Indenter::Indent();
  for (int i = 0; i < Blocks.size(); i++) {
    Block *Curr = Blocks[i];
    PrintIndented("case %d: {\n", Curr->Id);
    Indenter::Indent();
    Curr->Render(InLoop);
    PrintIndented("break;\n");
    Indenter::Unindent();
    PrintIndented("}\n");
  }
  Indenter::Unindent();
  PrintIndented("}\n");
  Indenter::Unindent();
  PrintIndented("}\n");
  if (Next) Next->Render(InLoop);
};
*/

// Relooper

Relooper::Relooper() : Root(NULL) {
}

Relooper::~Relooper() {
  for (int i = 0; i < Blocks.size(); i++) delete Blocks[i];
  for (int i = 0; i < Shapes.size(); i++) delete Shapes[i];
}

void Relooper::AddBlock(Block *New) {
  Blocks.push_back(New);
}

struct RelooperRecursor {
  Relooper *Parent;
  RelooperRecursor(Relooper *ParentInit) : Parent(ParentInit) {}
};

typedef std::list<Block*> BlockList;

void Relooper::Calculate(Block *Entry) {
  // Scan and optimize the input
  struct PreOptimizer : public RelooperRecursor {
    PreOptimizer(Relooper *Parent) : RelooperRecursor(Parent) {}
    BlockSet Live;

    void FindLive(Block *Root) {
      BlockList ToInvestigate;
      ToInvestigate.push_back(Root);
      while (ToInvestigate.size() > 0) {
        Block *Curr = ToInvestigate.front();
        ToInvestigate.pop_front();
        if (Live.find(Curr) != Live.end()) continue;
        Live.insert(Curr);
        for (BlockBranchMap::iterator iter = Curr->BranchesOut.begin(); iter != Curr->BranchesOut.end(); iter++) {
          ToInvestigate.push_back(iter->first);
        }
      }
    }

    // If a block has multiple entries but no exits, and it is small enough, it is useful to split it.
    // A common example is a C++ function where everything ends up at a final exit block and does some
    // RAII cleanup. Without splitting, we will be forced to introduce labelled loops to allow
    // reaching the final block
    void SplitDeadEnds() {
      int TotalCodeSize = 0;
      for (BlockSet::iterator iter = Live.begin(); iter != Live.end(); iter++) {
        Block *Curr = *iter;
        TotalCodeSize += strlen(Curr->Code);
      }

      for (BlockSet::iterator iter = Live.begin(); iter != Live.end(); iter++) {
        Block *Original = *iter;
        if (Original->BranchesIn.size() <= 1 || Original->BranchesOut.size() > 0) continue;
        if (strlen(Original->Code)*(Original->BranchesIn.size()-1) > TotalCodeSize/5) continue; // if splitting increases raw code size by a significant amount, abort
        // Split the node (for simplicity, we replace all the blocks, even though we could have reused the original)
        for (BlockBranchMap::iterator iter = Original->BranchesIn.begin(); iter != Original->BranchesIn.end(); iter++) {
          Block *Prior = iter->first;
          Block *Split = new Block(Original->Code);
          Split->BranchesIn[Prior] = new Branch(NULL);
          Prior->BranchesOut[Split] = new Branch(Prior->BranchesOut[Original]->Condition, Prior->BranchesOut[Original]->Code);
          Prior->BranchesOut.erase(Original);
          Parent->AddBlock(Split);
          Live.insert(Split);
        }
      }
    }
  };
  PreOptimizer Pre(this);
  Pre.FindLive(Entry);

  // Add incoming branches from live blocks, ignoring dead code
  for (int i = 0; i < Blocks.size(); i++) {
    Block *Curr = Blocks[i];
    if (Pre.Live.find(Curr) == Pre.Live.end()) continue;
    for (BlockBranchMap::iterator iter = Curr->BranchesOut.begin(); iter != Curr->BranchesOut.end(); iter++) {
      iter->first->BranchesIn[Curr] = new Branch(NULL);
    }
  }

  Pre.SplitDeadEnds();

  // Recursively process the graph

  struct Analyzer : public RelooperRecursor {
    Analyzer(Relooper *Parent) : RelooperRecursor(Parent) {}

    // Add a shape to the list of shapes in this Relooper calculation
    void Notice(Shape *New) {
      Parent->Shapes.push_back(New);
    }

    // Create a list of entries from a block. If LimitTo is provided, only results in that set
    // will appear
    void GetBlocksOut(Block *Source, BlockSet& Entries, BlockSet *LimitTo=NULL) {
      for (BlockBranchMap::iterator iter = Source->BranchesOut.begin(); iter != Source->BranchesOut.end(); iter++) {
        if (!LimitTo || LimitTo->find(iter->first) != LimitTo->end()) {
          Entries.insert(iter->first);
        }
      }
    }

    // Converts/processes all branchings to a specific target
    void Solipsize(Block *Target, Branch::FlowType Type, Shape *Ancestor, BlockSet &From) {
      PrintDebug("Solipsizing branches into %d\n", Target->Id);
      DebugDump(From, "  relevant to solipsize: ");
      for (BlockBranchMap::iterator iter = Target->BranchesIn.begin(); iter != Target->BranchesIn.end();) {
        Block *Prior = iter->first;
        if (From.find(Prior) == From.end()) {
          iter++;
          continue;
        }
        Branch *TargetIn = iter->second;
        Branch *PriorOut = Prior->BranchesOut[Target];
        PriorOut->Ancestor = Ancestor; // Do we need this info
        PriorOut->Type = Type;         // on TargetIn too?
        if (MultipleShape *Multiple = Shape::IsMultiple(Ancestor)) {
          Multiple->NeedLoop++; // We are breaking out of this Multiple, so need a loop
        }
        iter++; // carefully increment iter before erasing
        Target->BranchesIn.erase(Prior);
        Target->ProcessedBranchesIn[Prior] = TargetIn;
        Prior->BranchesOut.erase(Target);
        Prior->ProcessedBranchesOut[Target] = PriorOut;
        PrintDebug("  eliminated branch from %d\n", Prior->Id);
      }
    }

    Shape *MakeSimple(BlockSet &Blocks, Block *Inner, BlockSet &NextEntries) {
      PrintDebug("creating simple block with block #%d\n", Inner->Id);
      SimpleShape *Simple = new SimpleShape;
      Notice(Simple);
      Simple->Inner = Inner;
      Inner->Parent = Simple;
      if (Blocks.size() > 1) {
        Blocks.erase(Inner);
        GetBlocksOut(Inner, NextEntries, &Blocks);
        BlockSet JustInner;
        JustInner.insert(Inner);
        for (BlockSet::iterator iter = NextEntries.begin(); iter != NextEntries.end(); iter++) {
          Solipsize(*iter, Branch::Direct, Simple, JustInner);
        }
      }
      return Simple;
    }

    Shape *MakeLoop(BlockSet &Blocks, BlockSet& Entries, BlockSet &NextEntries) {
      // Find the inner blocks in this loop. Proceed backwards from the entries until
      // you reach a seen block, collecting as you go.
      BlockSet InnerBlocks;
      BlockSet Queue = Entries;
      while (Queue.size() > 0) {
        Block *Curr = *(Queue.begin());
        Queue.erase(Queue.begin());
        if (InnerBlocks.find(Curr) == InnerBlocks.end()) {
          // This element is new, mark it as inner and remove from outer
          InnerBlocks.insert(Curr);
          Blocks.erase(Curr);
          // Add the elements prior to it
          for (BlockBranchMap::iterator iter = Curr->BranchesIn.begin(); iter != Curr->BranchesIn.end(); iter++) {
            Queue.insert(iter->first);
          }
        }
      }
      assert(InnerBlocks.size() > 0);

      for (BlockSet::iterator iter = InnerBlocks.begin(); iter != InnerBlocks.end(); iter++) {
        Block *Curr = *iter;
        for (BlockBranchMap::iterator iter = Curr->BranchesOut.begin(); iter != Curr->BranchesOut.end(); iter++) {
          Block *Possible = iter->first;
          if (InnerBlocks.find(Possible) == InnerBlocks.end()) {
            NextEntries.insert(Possible);
          }
        }
      }

      PrintDebug("creating loop block:\n");
      DebugDump(InnerBlocks, "  inner blocks:");
      DebugDump(Entries, "  inner entries:");
      DebugDump(Blocks, "  outer blocks:");
      DebugDump(NextEntries, "  outer entries:");

      // TODO: Optionally hoist additional blocks into the loop

      LoopShape *Loop = new LoopShape();
      Notice(Loop);

      // Solipsize the loop, replacing with break/continue and marking branches as Processed (will not affect later calculations)
      // A. Branches to the loop entries become a continue to this shape
      for (BlockSet::iterator iter = Entries.begin(); iter != Entries.end(); iter++) {
        Solipsize(*iter, Branch::Continue, Loop, InnerBlocks);
      }
      // B. Branches to outside the loop (a next entry) become breaks on this shape
      for (BlockSet::iterator iter = NextEntries.begin(); iter != NextEntries.end(); iter++) {
        Solipsize(*iter, Branch::Break, Loop, InnerBlocks);
      }
      // Finish up
      Shape *Inner = Process(InnerBlocks, Entries, NULL);
      Loop->Inner = Inner;
      return Loop;
    }

    // For each entry, find the independent group reachable by it. The independent group is
    // the entry itself, plus all the blocks it can reach that cannot be directly reached by another entry. Note that we
    // ignore directly reaching the entry itself by another entry.
    void FindIndependentGroups(BlockSet &Blocks, BlockSet &Entries, BlockBlockSetMap& IndependentGroups) {
      typedef std::map<Block*, Block*> BlockBlockMap;

      struct HelperClass {
        BlockBlockSetMap& IndependentGroups;
        BlockBlockMap Ownership; // For each block, which entry it belongs to. We have reached it from there.

        HelperClass(BlockBlockSetMap& IndependentGroupsInit) : IndependentGroups(IndependentGroupsInit) {}
        void InvalidateWithChildren(Block *New) { // TODO: rename New
          BlockList ToInvalidate; // Being in the list means you need to be invalidated
          ToInvalidate.push_back(New);
          while (ToInvalidate.size() > 0) {
            Block *Invalidatee = ToInvalidate.front();
            ToInvalidate.pop_front();
            Block *Owner = Ownership[Invalidatee];
            if (IndependentGroups.find(Owner) != IndependentGroups.end()) { // Owner may have been invalidated, do not add to IndependentGroups!
              IndependentGroups[Owner].erase(Invalidatee);
            }
            if (Ownership[Invalidatee]) { // may have been seen before and invalidated already
              Ownership[Invalidatee] = NULL;
              for (BlockBranchMap::iterator iter = Invalidatee->BranchesOut.begin(); iter != Invalidatee->BranchesOut.end(); iter++) {
                Block *Target = iter->first;
                BlockBlockMap::iterator Known = Ownership.find(Target);
                if (Known != Ownership.end()) {
                  Block *TargetOwner = Known->second;
                  if (TargetOwner) {
                    ToInvalidate.push_back(Target);
                  }
                }
              }
            }
          }
        }
      };
      HelperClass Helper(IndependentGroups);

      // We flow out from each of the entries, simultaneously.
      // When we reach a new block, we add it as belonging to the one we got to it from.
      // If we reach a new block that is already marked as belonging to someone, it is reachable by
      // two entries and is not valid for any of them. Remove it and all it can reach that have been
      // visited.

      BlockList Queue; // Being in the queue means we just added this item, and we need to add its children
      for (BlockSet::iterator iter = Entries.begin(); iter != Entries.end(); iter++) {
        Block *Entry = *iter;
        Helper.Ownership[Entry] = Entry;
        IndependentGroups[Entry].insert(Entry);
        Queue.push_back(Entry);
      }
      while (Queue.size() > 0) {
        Block *Curr = Queue.front();
        Queue.pop_front();
        Block *Owner = Helper.Ownership[Curr]; // Curr must be in the ownership map if we are in the queue
        if (!Owner) continue; // we have been invalidated meanwhile after being reached from two entries
        // Add all children
        for (BlockBranchMap::iterator iter = Curr->BranchesOut.begin(); iter != Curr->BranchesOut.end(); iter++) {
          Block *New = iter->first;
          BlockBlockMap::iterator Known = Helper.Ownership.find(New);
          if (Known == Helper.Ownership.end()) {
            // New node. Add it, and put it in the queue
            Helper.Ownership[New] = Owner;
            IndependentGroups[Owner].insert(New);
            Queue.push_back(New);
            continue;
          }
          Block *NewOwner = Known->second;
          if (!NewOwner) continue; // We reached an invalidated node
          if (NewOwner != Owner) {
            // Invalidate this and all reachable that we have seen - we reached this from two locations
            Helper.InvalidateWithChildren(New);
          }
          // otherwise, we have the same owner, so do nothing
        }
      }

      // Having processed all the interesting blocks, we remain with just one potential issue:
      // If a->b, and a was invalidated, but then b was later reached by someone else, we must
      // invalidate b. To check for this, we go over all elements in the independent groups,
      // if an element has a parent which does *not* have the same owner, we must remove it
      // and all its children.

      for (BlockSet::iterator iter = Entries.begin(); iter != Entries.end(); iter++) {
        BlockSet &CurrGroup = IndependentGroups[*iter];
        BlockList ToInvalidate;
        for (BlockSet::iterator iter = CurrGroup.begin(); iter != CurrGroup.end(); iter++) {
          Block *Child = *iter;
          for (BlockBranchMap::iterator iter = Child->BranchesIn.begin(); iter != Child->BranchesIn.end(); iter++) {
            Block *Parent = iter->first;
            if (Helper.Ownership[Parent] != Helper.Ownership[Child]) {
              ToInvalidate.push_back(Child);
            }
          }
        }
        while (ToInvalidate.size() > 0) {
          Block *Invalidatee = ToInvalidate.front();
          ToInvalidate.pop_front();
          Helper.InvalidateWithChildren(Invalidatee);
        }
      }

      // Remove empty groups
      for (BlockSet::iterator iter = Entries.begin(); iter != Entries.end(); iter++) {
        if (IndependentGroups[*iter].size() == 0) {
          IndependentGroups.erase(*iter);
        }
      }

#if DEBUG
      PrintDebug("Investigated independent groups:\n");
      for (BlockBlockSetMap::iterator iter = IndependentGroups.begin(); iter != IndependentGroups.end(); iter++) {
        DebugDump(iter->second, " group: ");
      }
#endif
    }

    Shape *MakeMultiple(BlockSet &Blocks, BlockSet& Entries, BlockBlockSetMap& IndependentGroups, Shape *Prev, BlockSet &NextEntries) {
      PrintDebug("creating multiple block with %d inner groups\n", IndependentGroups.size());
      bool Fused = !!(Shape::IsSimple(Prev));
      MultipleShape *Multiple = new MultipleShape();
      Notice(Multiple);
      BlockSet CurrEntries;
      for (BlockBlockSetMap::iterator iter = IndependentGroups.begin(); iter != IndependentGroups.end(); iter++) {
        Block *CurrEntry = iter->first;
        BlockSet &CurrBlocks = iter->second;
        PrintDebug("  multiple group with entry %d:\n", CurrEntry->Id);
        DebugDump(CurrBlocks, "    ");
        // Create inner block
        CurrEntries.clear();
        CurrEntries.insert(CurrEntry);
        for (BlockSet::iterator iter = CurrBlocks.begin(); iter != CurrBlocks.end(); iter++) {
          Block *CurrInner = *iter;
          // Remove the block from the remaining blocks
          Blocks.erase(CurrInner);
          // Find new next entries and fix branches to them
          for (BlockBranchMap::iterator iter = CurrInner->BranchesOut.begin(); iter != CurrInner->BranchesOut.end();) {
            Block *CurrTarget = iter->first;
            BlockBranchMap::iterator Next = iter;
            Next++;
            if (CurrBlocks.find(CurrTarget) == CurrBlocks.end()) {
              NextEntries.insert(CurrTarget);
              Solipsize(CurrTarget, Branch::Break, Multiple, CurrBlocks); 
            }
            iter = Next; // increment carefully because Solipsize can remove us
          }
        }
        Multiple->InnerMap[CurrEntry] = Process(CurrBlocks, CurrEntries, NULL);
        // If we are not fused, then our entries will actually be checked
        if (!Fused) {
          CurrEntry->IsCheckedMultipleEntry = true;
        }
      }
      DebugDump(Blocks, "  remaining blocks after multiple:");
      // Add entries not handled as next entries, they are deferred
      for (BlockSet::iterator iter = Entries.begin(); iter != Entries.end(); iter++) {
        Block *Entry = *iter;
        if (IndependentGroups.find(Entry) == IndependentGroups.end()) {
          NextEntries.insert(Entry);
        }
      }
      return Multiple;
    }

    // Main function.
    // Process a set of blocks with specified entries, returns a shape
    // The Make* functions receive a NextEntries. If they fill it with data, those are the entries for the
    //   ->Next block on them, and the blocks are what remains in Blocks (which Make* modify). In this way
    //   we avoid recursing on Next (imagine a long chain of Simples, if we recursed we could blow the stack).
    Shape *Process(BlockSet &Blocks, BlockSet& InitialEntries, Shape *Prev) {
      PrintDebug("Process() called\n");
      BlockSet *Entries = &InitialEntries;
      BlockSet TempEntries[2];
      int CurrTempIndex = 0;
      BlockSet *NextEntries;
      Shape *Ret = NULL;
      #define Make(call) \
        Shape *Temp = call; \
        if (Prev) Prev->Next = Temp; \
        if (!Ret) Ret = Temp; \
        if (!NextEntries->size()) { PrintDebug("Process() returning\n"); return Ret; } \
        Prev = Temp; \
        Entries = NextEntries; \
        continue;
      while (1) {
        PrintDebug("Process() running\n");
        DebugDump(Blocks, "  blocks : ");
        DebugDump(*Entries, "  entries: ");

        CurrTempIndex = 1-CurrTempIndex;
        NextEntries = &TempEntries[CurrTempIndex];
        NextEntries->clear();

        if (Entries->size() == 0) return Ret;
        if (Entries->size() == 1) {
          Block *Curr = *(Entries->begin());
          if (Curr->BranchesIn.size() == 0) {
            // One entry, no looping ==> Simple
            Make(MakeSimple(Blocks, Curr, *NextEntries));
          }
          // One entry, looping ==> Loop
          Make(MakeLoop(Blocks, *Entries, *NextEntries));
        }
        // More than one entry, try to eliminate through a Multiple groups of
        // independent blocks from an entry/ies. It is important to remove through
        // multiples as opposed to looping since the former is more performant.
        BlockBlockSetMap IndependentGroups;
        FindIndependentGroups(Blocks, *Entries, IndependentGroups);

        PrintDebug("Independent groups: %d\n", IndependentGroups.size());

        if (IndependentGroups.size() > 0) {
          // We can handle a group in a multiple if its entry cannot be reached by another group.
          // Note that it might be reachable by itself - a loop. But that is fine, we will create
          // a loop inside the multiple block (which is the performant order to do it).
          for (BlockBlockSetMap::iterator iter = IndependentGroups.begin(); iter != IndependentGroups.end();) {
            Block *Entry = iter->first;
            BlockSet &Group = iter->second;
            BlockBlockSetMap::iterator curr = iter++; // iterate carefully, we may delete
            for (BlockBranchMap::iterator iterBranch = Entry->BranchesIn.begin(); iterBranch != Entry->BranchesIn.end(); iterBranch++) {
              Block *Origin = iterBranch->first;
              if (Group.find(Origin) == Group.end()) {
                // Reached from outside the group, so we cannot handle this
                PrintDebug("Cannot handle group with entry %d because of incoming branch from %d\n", Entry->Id, Origin->Id);
                IndependentGroups.erase(curr);
                break;
              }
            }
          }

          // As an optimization, if we have 2 independent groups, and one is a small dead end, we can handle only that dead end.
          // The other then becomes a Next - without nesting in the code and recursion in the analysis.
          // TODO: if the larger is the only dead end, handle that too
          // TODO: handle >2 groups
          // TODO: handle not just dead ends, but also that do not branch to the NextEntries. However, must be careful
          //       there since we create a Next, and that Next can prevent eliminating a break (since we no longer
          //       naturally reach the same place), which may necessitate a one-time loop, which makes the unnesting
          //       pointless.
          if (IndependentGroups.size() == 2) {
            // Find the smaller one
            BlockBlockSetMap::iterator iter = IndependentGroups.begin();
            Block *SmallEntry = iter->first;
            int SmallSize = iter->second.size();
            iter++;
            Block *LargeEntry = iter->first;
            int LargeSize = iter->second.size();
            if (SmallSize != LargeSize) { // ignore the case where they are identical - keep things symmetrical there
              if (SmallSize > LargeSize) {
                Block *Temp = SmallEntry;
                SmallEntry = LargeEntry;
                LargeEntry = Temp; // Note: we did not flip the Sizes too, they are now invalid. TODO: use the smaller size as a limit?
              }
              // Check if dead end
              bool DeadEnd = true;
              BlockSet &SmallGroup = IndependentGroups[SmallEntry];
              for (BlockSet::iterator iter = SmallGroup.begin(); iter != SmallGroup.end(); iter++) {
                Block *Curr = *iter;
                for (BlockBranchMap::iterator iter = Curr->BranchesOut.begin(); iter != Curr->BranchesOut.end(); iter++) {
                  Block *Target = iter->first;
                  if (SmallGroup.find(Target) == SmallGroup.end()) {
                    DeadEnd = false;
                    break;
                  }
                }
                if (!DeadEnd) break;
              }
              if (DeadEnd) {
                PrintDebug("Removing nesting by not handling large group because small group is dead end\n");
                IndependentGroups.erase(LargeEntry);
              }
            }
          }

          PrintDebug("Handleable independent groups: %d\n", IndependentGroups.size());

          if (IndependentGroups.size() > 0) {
            // Some groups removable ==> Multiple
            Make(MakeMultiple(Blocks, *Entries, IndependentGroups, Prev, *NextEntries));
          }
        }
        // No independent groups, must be loopable ==> Loop
        Make(MakeLoop(Blocks, *Entries, *NextEntries));
      }
    }
  };

  // Main

  BlockSet AllBlocks;
  for (int i = 0; i < Blocks.size(); i++) {
    AllBlocks.insert(Blocks[i]);
#if DEBUG
    PrintDebug("Adding block %d (%s)\n", Blocks[i]->Id, Blocks[i]->Code);
    for (BlockBranchMap::iterator iter = Blocks[i]->BranchesOut.begin(); iter != Blocks[i]->BranchesOut.end(); iter++) {
      PrintDebug("  with branch out to %d\n", iter->first->Id);
    }
#endif
  }

  BlockSet Entries;
  Entries.insert(Entry);
  Root = Analyzer(this).Process(AllBlocks, Entries, NULL);

  // Post optimizations

  struct PostOptimizer {
    Relooper *Parent;
    void *Closure;

    PostOptimizer(Relooper *ParentInit) : Parent(ParentInit), Closure(NULL) {}

    #define RECURSE_MULTIPLE_MANUAL(func, manual) \
      for (BlockShapeMap::iterator iter = manual->InnerMap.begin(); iter != manual->InnerMap.end(); iter++) { \
        func(iter->second); \
      }
    #define RECURSE_MULTIPLE(func) RECURSE_MULTIPLE_MANUAL(func, Multiple);
    #define RECURSE_LOOP(func) \
      func(Loop->Inner);

    #define SHAPE_SWITCH(var, simple, multiple, loop) \
      if (SimpleShape *Simple = Shape::IsSimple(var)) { \
        simple; \
      } else if (MultipleShape *Multiple = Shape::IsMultiple(var)) { \
        multiple; \
      } else if (LoopShape *Loop = Shape::IsLoop(var)) { \
        loop; \
      }

    #define SHAPE_SWITCH_AUTO(var, simple, multiple, loop, func) \
      if (SimpleShape *Simple = Shape::IsSimple(var)) { \
        simple; \
        func(Simple->Next); \
      } else if (MultipleShape *Multiple = Shape::IsMultiple(var)) { \
        multiple; \
        RECURSE_MULTIPLE(func) \
        func(Multiple->Next); \
      } else if (LoopShape *Loop = Shape::IsLoop(var)) { \
        loop; \
        RECURSE_LOOP(func); \
        func(Loop->Next); \
      }

    // Remove unneeded breaks and continues.
    // A flow operation is trivially unneeded if the shape we naturally get to by normal code
    // execution is the same as the flow forces us to.
    void RemoveUnneededFlows(Shape *Root, Shape *Natural=NULL) {
      Shape *Next = Root;
      while (Next) {
        Root = Next;
        Next = NULL;
        SHAPE_SWITCH(Root, {
          // If there is a next block, we already know at Simple creation time to make direct branches,
          // and we can do nothing more. If there is no next however, then Natural is where we will
          // go to by doing nothing, so we can potentially optimize some branches to direct.
          if (Simple->Next) {
            Next = Simple->Next;
          } else {
            for (BlockBranchMap::iterator iter = Simple->Inner->ProcessedBranchesOut.begin(); iter != Simple->Inner->ProcessedBranchesOut.end(); iter++) {
              Block *Target = iter->first;
              Branch *Details = iter->second;
              if (Details->Type != Branch::Direct && Target->Parent == Natural) {
                Details->Type = Branch::Direct;
                if (MultipleShape *Multiple = Shape::IsMultiple(Details->Ancestor)) {
                  Multiple->NeedLoop--;
                }
              }
            }
          }
        }, {
          for (BlockShapeMap::iterator iter = Multiple->InnerMap.begin(); iter != Multiple->InnerMap.end(); iter++) {
            RemoveUnneededFlows(iter->second, Multiple->Next);
          }
          Next = Multiple->Next;
        }, {
          RemoveUnneededFlows(Loop->Inner, Loop->Inner);
          Next = Loop->Next;
        });
      }
    }

    // After we know which loops exist, we can calculate which need to be labeled
    void FindLabeledLoops(Shape *Root) {
      bool First = Closure == NULL;
      if (First) {
        Closure = (void*)(new std::stack<Shape*>);
      }
      std::stack<Shape*> &LoopStack = *((std::stack<Shape*>*)Closure);

      Shape *Next = Root;
      while (Next) {
        Root = Next;
        Next = NULL;

        SHAPE_SWITCH(Root, {
          MultipleShape *Fused = Shape::IsMultiple(Root->Next);
          // If we are fusing a Multiple with a loop into this Simple, then visit it now
          if (Fused && Fused->NeedLoop) {
            LoopStack.push(Fused);
            RECURSE_MULTIPLE_MANUAL(FindLabeledLoops, Fused);
          }
          for (BlockBranchMap::iterator iter = Simple->Inner->ProcessedBranchesOut.begin(); iter != Simple->Inner->ProcessedBranchesOut.end(); iter++) {
            Block *Target = iter->first;
            Branch *Details = iter->second;
            if (Details->Type != Branch::Direct) {
              assert(LoopStack.size() > 0);
              if (Details->Ancestor != LoopStack.top()) {
                LabeledShape *Labeled = Shape::IsLabeled(Details->Ancestor);
                Labeled->Labeled = true;
                Details->Labeled = true;
              } else {
                Details->Labeled = false;
              }
            }
          }
          if (Fused && Fused->NeedLoop) {
            LoopStack.pop();
            Next = Fused->Next;
          } else {
            Next = Root->Next;
          }
        }, {
          if (Multiple->NeedLoop) {
            LoopStack.push(Multiple);
          }
          RECURSE_MULTIPLE(FindLabeledLoops);
          if (Multiple->NeedLoop) {
            LoopStack.pop();
          }
          Next = Root->Next;
        }, {
          LoopStack.push(Loop);
          RECURSE_LOOP(FindLabeledLoops);
          LoopStack.pop();
          Next = Root->Next;
        });
      }

      if (First) {
        delete (std::stack<Shape*>*)Closure;
      }
    }

    void Process(Shape *Root) {
      RemoveUnneededFlows(Root);
      FindLabeledLoops(Root);
    }
  };

  PrintDebug("=== Optimizing shapes ===\n");

  PostOptimizer(this).Process(Root);
}

void Relooper::Render() {
  OutputBuffer = OutputBufferRoot;
  Root->Render(false);
}

void Relooper::SetOutputBuffer(char *Buffer, int Size) {
  OutputBufferRoot = OutputBuffer = Buffer;
  OutputBufferSize = Size;
}

void Relooper::MakeOutputBuffer(int Size) {
  OutputBufferRoot = OutputBuffer = (char*)malloc(Size);
  OutputBufferSize = Size;
}

void Relooper::SetAsmJSMode(int On) {
  AsmJS = On;
}

#if DEBUG
// Debugging

void DebugDump(BlockSet &Blocks, const char *prefix) {
  if (prefix) printf("%s ", prefix);
  for (BlockSet::iterator iter = Blocks.begin(); iter != Blocks.end(); iter++) {
    printf("%d ", (*iter)->Id);
  }
  printf("\n");
}

static void PrintDebug(const char *Format, ...) {
  printf("// ");
  va_list Args;
  va_start(Args, Format);
  vprintf(Format, Args);
  va_end(Args);
}
#endif

// C API - useful for binding to other languages

typedef std::map<void*, int> VoidIntMap;
VoidIntMap __blockDebugMap__; // maps block pointers in currently running code to block ids, for generated debug output

extern "C" {

void rl_set_output_buffer(char *buffer, int size) {
#if DEBUG
  printf("#include \"Relooper.h\"\n");
  printf("int main() {\n");
  printf("  char buffer[100000];\n");
  printf("  rl_set_output_buffer(buffer);\n");
#endif
  Relooper::SetOutputBuffer(buffer, size);
}

void rl_make_output_buffer(int size) {
  Relooper::SetOutputBuffer((char*)malloc(size), size);
}

void rl_set_asm_js_mode(int on) {
  Relooper::SetAsmJSMode(on);
}

void *rl_new_block(const char *text) {
  Block *ret = new Block(text);
#if DEBUG
  printf("  void *b%d = rl_new_block(\"// code %d\");\n", ret->Id, ret->Id);
  __blockDebugMap__[ret] = ret->Id;
  printf("  block_map[%d] = b%d;\n", ret->Id, ret->Id);
#endif
  return ret;
}

void rl_delete_block(void *block) {
#if DEBUG
  printf("  rl_delete_block(block_map[%d]);\n", ((Block*)block)->Id);
#endif
  delete (Block*)block;
}

void rl_block_add_branch_to(void *from, void *to, const char *condition, const char *code) {
#if DEBUG
  printf("  rl_block_add_branch_to(block_map[%d], block_map[%d], %s%s%s, %s%s%s);\n", ((Block*)from)->Id, ((Block*)to)->Id, condition ? "\"" : "", condition ? condition : "NULL", condition ? "\"" : "", code ? "\"" : "", code ? code : "NULL", code ? "\"" : "");
#endif
  ((Block*)from)->AddBranchTo((Block*)to, condition, code);
}

void *rl_new_relooper() {
#if DEBUG
  printf("  void *block_map[10000];\n");
  printf("  void *rl = rl_new_relooper();\n");
#endif
  return new Relooper;
}

void rl_delete_relooper(void *relooper) {
  delete (Relooper*)relooper;
}

void rl_relooper_add_block(void *relooper, void *block) {
#if DEBUG
  printf("  rl_relooper_add_block(rl, block_map[%d]);\n", ((Block*)block)->Id);
#endif
  ((Relooper*)relooper)->AddBlock((Block*)block);
}

void rl_relooper_calculate(void *relooper, void *entry) {
#if DEBUG
  printf("  rl_relooper_calculate(rl, block_map[%d]);\n", ((Block*)entry)->Id);
  printf("  rl_relooper_render(rl);\n");
  printf("  rl_delete_relooper(rl);\n");
  printf("  puts(buffer);\n");
  printf("  return 0;\n");
  printf("}\n");
#endif
  ((Relooper*)relooper)->Calculate((Block*)entry);
}

void rl_relooper_render(void *relooper) {
  ((Relooper*)relooper)->Render();
}

}

