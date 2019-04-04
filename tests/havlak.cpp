// Copyright 2011 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// seen at https://github.com/kostya/benchmarks/blob/master/havlak/havlak.cpp

#include "stdio.h"
#include <set>
#include <unordered_set> // because set of pointers fails in cheerp
#include <unordered_map> // because map of pointers fails in cheerp
#include <map>
#include <list>
#include <vector>
#include <algorithm>

// Forward Decls
class BasicBlock;
class MaoCFG;

//--- MOCKING CODE begin -------------------
//
// These data structures are stubbed out to make the code below easier
// to review.
//
// BasicBlockEdge only maintains two pointers to BasicBlocks.
//
class BasicBlockEdge {
 public:
  inline BasicBlockEdge(MaoCFG *cfg, int from, int to);

  BasicBlock *GetSrc() { return from_; }
  BasicBlock *GetDst() { return to_; }

 private:
  BasicBlock *from_, *to_;
};

// BasicBlock only maintains a vector of in-edges and
// a vector of out-edges.
//
class BasicBlock {
 public:
  typedef std::vector<BasicBlock *> EdgeVector;

  explicit BasicBlock(int name) : name_(name) {
  }

  EdgeVector *in_edges() { return &in_edges_; }
  EdgeVector *out_edges() { return &out_edges_; }

  int GetNumPred() { return in_edges_.size(); }
  int GetNumSucc() { return out_edges_.size(); }

  void AddOutEdge(BasicBlock *to) { out_edges_.push_back(to); }
  void AddInEdge(BasicBlock *from) { in_edges_.push_back(from); }

 private:
  EdgeVector in_edges_, out_edges_;
  int name_;
};

// MaoCFG maintains a list of nodes.
//
class MaoCFG {
 public:
  typedef std::unordered_map<int, BasicBlock *> NodeMap;
  typedef std::list<BasicBlockEdge *> EdgeList;

  MaoCFG() : start_node_(NULL) {
  }

  ~MaoCFG() {
    for (NodeMap::iterator it = basic_block_map_.begin();
         it != basic_block_map_.end(); ++it)
      delete (*it).second;

    for (EdgeList::iterator edge_it = edge_list_.begin();
         edge_it != edge_list_.end(); ++edge_it)
      delete (*edge_it);
  }

  BasicBlock *CreateNode(int name) {
    BasicBlock *node;

    NodeMap::iterator it = basic_block_map_.find(name);

    if (it == basic_block_map_.end()) {
      node = new BasicBlock(name);
      basic_block_map_[name] = node;
    } else {
      node = (*it).second;
    }

    if (GetNumNodes() == 1)
      start_node_ = node;

    return node;
  }

  void AddEdge(BasicBlockEdge *edge) {
    edge_list_.push_back(edge);
  }

  int GetNumNodes() {
    return basic_block_map_.size();
  }

  BasicBlock *GetStartBasicBlock() {
    return start_node_;
  }

  BasicBlock *GetDst(BasicBlockEdge *edge) {
    return edge->GetDst();
  }

  BasicBlock *GetSrc(BasicBlockEdge *edge) {
    return edge->GetSrc();
  }

  NodeMap *GetBasicBlocks() {
    return &basic_block_map_;
  }

 private:
  NodeMap       basic_block_map_;
  BasicBlock   *start_node_;
  EdgeList      edge_list_;
};

//
//--- MOCKING CODE end  -------------------


//
// SimpleLoop
//
// Basic representation of loops, a loop has an entry point,
// one or more exit edges, a set of basic blocks, and potentially
// an outer loop - a "parent" loop.
//
// Furthermore, it can have any set of properties, e.g.,
// it can be an irreducible loop, have control flow, be
// a candidate for transformations, and what not.
//
//
class SimpleLoop {
 public:
  typedef std::unordered_set<BasicBlock *> BasicBlockSet;
  typedef std::unordered_set<SimpleLoop *> LoopSet;


  SimpleLoop() : parent_(NULL), is_root_(false), nesting_level_(0),
                 depth_level_(0) {
  }

  void AddNode(BasicBlock *basic_block) {
    basic_blocks_.insert(basic_block);
  }

  void AddChildLoop(SimpleLoop *loop) {
    children_.insert(loop);
  }

  void Dump() {
    // Simplified for readability purposes.
    fprintf(stderr, "loop-%d, nest: %d, depth: %d\n",
            counter_, nesting_level_, depth_level_);
  }

  LoopSet *GetChildren() {
    return &children_;
  }

  // Getters/Setters
  SimpleLoop  *parent() { return parent_; }
  int          nesting_level() const { return nesting_level_; }
  int          depth_level() const { return depth_level_; }
  int          counter() const { return counter_; }
  bool         is_root() const { return is_root_; }

  void set_parent(SimpleLoop *parent) {
    parent_ = parent;
    parent->AddChildLoop(this);
  }

  void set_is_root() { is_root_ = true; }
  void set_counter(int value) { counter_ = value; }
  void set_nesting_level(int level) {
    nesting_level_ = level;
    if (level == 0)
      set_is_root();
  }
  void set_depth_level(int level) { depth_level_ = level; }

 private:
  BasicBlockSet          basic_blocks_;
  std::unordered_set<SimpleLoop *> children_;
  SimpleLoop            *parent_;

  bool         is_root_: 1;
  int          counter_;
  int          nesting_level_;
  int          depth_level_;
};



//
// LoopStructureGraph
//
// Maintain loop structure for a given CFG.
//
// Two values are maintained for this loop graph, depth, and nesting level.
// For example:
//
// loop        nesting level    depth
//----------------------------------------
// loop-0      2                0
//   loop-1    1                1
//   loop-3    1                1
//     loop-2  0                2
//
class LoopStructureGraph {
 public:
  typedef std::list<SimpleLoop *> LoopList;

  LoopStructureGraph() : root_(new SimpleLoop()),
                         loop_counter_(0) {
    root_->set_nesting_level(0);  // make it the root node
    root_->set_counter(loop_counter_++);
    AddLoop(root_);
  }

  ~LoopStructureGraph() {
    KillAll();
  }

  SimpleLoop *CreateNewLoop() {
    SimpleLoop *loop = new SimpleLoop();
    loop->set_counter(loop_counter_++);
    return loop;
  }

  void KillAll() {
    for (LoopList::iterator it = loops_.begin(); it != loops_.end(); ++it)
      delete (*it);
  }

  void AddLoop(SimpleLoop *loop) {
    loops_.push_back(loop);
  }

  void Dump() {
    DumpRec(root_, 0);
  }

  void DumpRec(SimpleLoop *loop, int indent) {
    // Simplified for readability purposes.
    loop->Dump();

    for (SimpleLoop::LoopSet::iterator liter = loop->GetChildren()->begin();
         liter != loop->GetChildren()->end(); ++liter)
      DumpRec(*liter,  indent+1);
  }

  void CalculateNestingLevel() {
    // link up all 1st level loops to artificial root node.
    for (LoopList::iterator liter = loops_.begin();
         liter != loops_.end(); ++liter) {
      SimpleLoop *loop = *liter;
      if (loop->is_root()) continue;
      if (!loop->parent()) loop->set_parent(root_);
    }

    // recursively traverse the tree and assign levels.
    CalculateNestingLevelRec(root_, 0);
  }


  void CalculateNestingLevelRec(SimpleLoop *loop, int depth) {
    loop->set_depth_level(depth);
    for (SimpleLoop::LoopSet::iterator liter = loop->GetChildren()->begin();
         liter != loop->GetChildren()->end(); ++liter) {
      CalculateNestingLevelRec(*liter, depth+1);

      loop->set_nesting_level(std::max(loop->nesting_level(),
                                       1+(*liter)->nesting_level()));
    }
  }

  int GetNumLoops() const { return loops_.size(); }

  SimpleLoop *root() const { return root_; }

 private:
  SimpleLoop   *root_;
  LoopList      loops_;
  int           loop_counter_;
};

inline
BasicBlockEdge::BasicBlockEdge(MaoCFG     *cfg,
                               int         from_name,
                               int         to_name) {
  from_ = cfg->CreateNode(from_name);
  to_ = cfg->CreateNode(to_name);

  from_->AddOutEdge(to_);
  to_->AddInEdge(from_);

  cfg->AddEdge(this);
}


// External entry point.
int FindHavlakLoops(MaoCFG *CFG, LoopStructureGraph *LSG);


#include <stdio.h>
#include <list>
#include <set>
#include <vector>
#include <algorithm>

//======================================================
// Main Algorithm
//======================================================

//
// Union/Find algorithm after Tarjan, R.E., 1983, Data Structures
// and Network Algorithms.
//
class UnionFindNode {
 public:
  UnionFindNode() : parent_(NULL), bb_(NULL), loop_(NULL), dfs_number_(0) {
  }

  // Initialize this node.
  //
  void Init(BasicBlock *bb, int dfs_number) {
    parent_     = this;
    bb_         = bb;
    dfs_number_ = dfs_number;
  }

  // Union/Find Algorithm - The find routine.
  //
  // Implemented with Path Compression (inner loops are only
  // visited and collapsed once, however, deep nests would still
  // result in significant traversals).
  //
  UnionFindNode *FindSet() {
    typedef std::list<UnionFindNode *> NodeListType;
    NodeListType nodeList;

    UnionFindNode *node = this;
    while (node != node->parent()) {
      if (node->parent() != node->parent()->parent())
        nodeList.push_back(node);
      node = node->parent();
    }

    // Path Compression, all nodes' parents point to the 1st level parent.
    NodeListType::iterator iter = nodeList.begin();
    NodeListType::iterator end  = nodeList.end();
    for (; iter != end; ++iter)
      (*iter)->set_parent(node->parent());

    return node;
  }

  // Union/Find Algorithm - The union routine.
  //
  // We rely on path compression.
  //
  void Union(UnionFindNode *B) {
    set_parent(B);
  }


  // Getters/Setters
  //
  UnionFindNode *parent() const { return parent_; }
  BasicBlock    *bb() const { return bb_; }
  SimpleLoop    *loop() const { return loop_; }
  int            dfs_number() const { return dfs_number_; }

  void           set_parent(UnionFindNode *parent) { parent_ = parent; }
  void           set_loop(SimpleLoop *loop) { loop_ = loop; }

 private:
  UnionFindNode *parent_;
  BasicBlock    *bb_;
  SimpleLoop    *loop_;
  int            dfs_number_;
};

//------------------------------------------------------------------
// Loop Recognition
//
// based on:
//   Paul Havlak, Nesting of Reducible and Irreducible Loops,
//      Rice University.
//
//   We avoid doing tree balancing and instead use path compression
//   to avoid traversing parent pointers over and over.
//
//   Most of the variable names and identifiers are taken literally
//   from this paper (and the original Tarjan paper mentioned above).
//-------------------------------------------------------------------
class HavlakLoopFinder {
 public:
  HavlakLoopFinder(MaoCFG *cfg, LoopStructureGraph *lsg) :
    CFG_(cfg), lsg_(lsg) {
  }

  enum BasicBlockClass {
    BB_TOP,          // uninitialized
    BB_NONHEADER,    // a regular BB
    BB_REDUCIBLE,    // reducible loop
    BB_SELF,         // single BB loop
    BB_IRREDUCIBLE,  // irreducible loop
    BB_DEAD,         // a dead BB
    BB_LAST          // Sentinel
  };

  //
  // Constants
  //
  // Marker for uninitialized nodes.
  static const int kUnvisited = -1;
  // Safeguard against pathologic algorithm behavior.
  static const int kMaxNonBackPreds = (32*1024);

  //
  // Local types used for Havlak algorithm, all carefully
  // selected to guarantee minimal complexity.
  //
  typedef std::vector<UnionFindNode>           NodeVector;
  typedef std::unordered_map<BasicBlock*, int> BasicBlockMap;
  typedef std::list<int>                       IntList;
  typedef std::set<int>                        IntSet;
  typedef std::list<UnionFindNode*>            NodeList;
  typedef std::vector<IntList>                 IntListVector;
  typedef std::vector<IntSet>                  IntSetVector;
  typedef std::vector<int>                     IntVector;
  typedef std::vector<char>                    CharVector;

  //
  // IsAncestor
  //
  // As described in the paper, determine whether a node 'w' is a
  // "true" ancestor for node 'v'.
  //
  // Dominance can be tested quickly using a pre-order trick
  // for depth-first spanning trees. This is why DFS is the first
  // thing we run below.
  //
  bool IsAncestor(int w, int v, IntVector *last) {
    return ((w <= v) && (v <= (*last)[w]));
  }

  // Iterators
  //
  typedef BasicBlock::EdgeVector::iterator BasicBlockIter;

  //
  // DFS - Depth-First-Search
  //
  // DESCRIPTION:
  // Simple depth first traversal along out edges with node numbering.
  //
  int DFS(BasicBlock      *current_node,
          NodeVector      *nodes,
          BasicBlockMap   *number,
          IntVector       *last,
          const int       current) {
    (*nodes)[current].Init(current_node, current);
    (*number)[current_node] = current;

    int lastid = current;
    for (BasicBlockIter outedges = current_node->out_edges()->begin();
         outedges != current_node->out_edges()->end(); ++outedges) {
      BasicBlock *target = *outedges;

      if ((*number)[target] == kUnvisited)
        lastid = DFS(target, nodes, number, last, lastid + 1);
    }
    (*last)[(*number)[current_node]] = lastid;
    return lastid;
  }

  //
  // FindLoops
  //
  // Find loops and build loop forest using Havlak's algorithm, which
  // is derived from Tarjan. Variable names and step numbering has
  // been chosen to be identical to the nomenclature in Havlak's
  // paper (which is similar to the one used by Tarjan).
  //
  void FindLoops() {
    if (!CFG_->GetStartBasicBlock()) return;

    int                size = CFG_->GetNumNodes();

    IntSetVector       non_back_preds(size);
    IntListVector      back_preds(size);
    IntVector          header(size);
    CharVector         type(size);
    IntVector          last(size);
    NodeVector         nodes(size);
    BasicBlockMap      number;

    // Step a:
    //   - initialize all nodes as unvisited.
    //   - depth-first traversal and numbering.
    //   - unreached BB's are marked as dead.
    //
    for (MaoCFG::NodeMap::iterator bb_iter =
           CFG_->GetBasicBlocks()->begin();
         bb_iter != CFG_->GetBasicBlocks()->end(); ++bb_iter) {
      number[(*bb_iter).second] = kUnvisited;
    }

    DFS(CFG_->GetStartBasicBlock(), &nodes, &number, &last, 0);

    // Step b:
    //   - iterate over all nodes.
    //
    //   A backedge comes from a descendant in the DFS tree, and non-backedges
    //   from non-descendants (following Tarjan).
    //
    //   - check incoming edges 'v' and add them to either
    //     - the list of backedges (back_preds) or
    //     - the list of non-backedges (non_back_preds)
    //
    for (int w = 0; w < size; w++) {
      header[w] = 0;
      type[w] = BB_NONHEADER;

      BasicBlock *node_w = nodes[w].bb();
      if (!node_w) {
        type[w] = BB_DEAD;
        continue;  // dead BB
      }

      if (node_w->GetNumPred()) {
        for (BasicBlockIter inedges = node_w->in_edges()->begin();
             inedges != node_w->in_edges()->end(); ++inedges) {
          BasicBlock     *node_v = *inedges;

          int v = number[ node_v ];
          if (v == kUnvisited) continue;  // dead node

          if (IsAncestor(w, v, &last))
            back_preds[w].push_back(v);
          else
            non_back_preds[w].insert(v);
        }
      }
    }

    // Start node is root of all other loops.
    header[0] = 0;

    // Step c:
    //
    // The outer loop, unchanged from Tarjan. It does nothing except
    // for those nodes which are the destinations of backedges.
    // For a header node w, we chase backward from the sources of the
    // backedges adding nodes to the set P, representing the body of
    // the loop headed by w.
    //
    // By running through the nodes in reverse of the DFST preorder,
    // we ensure that inner loop headers will be processed before the
    // headers for surrounding loops.
    //
    for (int w = size-1; w >= 0; w--) {
      NodeList    node_pool;  // this is 'P' in Havlak's paper
      BasicBlock *node_w = nodes[w].bb();
      if (!node_w) continue;  // dead BB

      // Step d:
      IntList::iterator back_pred_iter  = back_preds[w].begin();
      IntList::iterator back_pred_end   = back_preds[w].end();
      for (; back_pred_iter != back_pred_end; back_pred_iter++) {
        int v = *back_pred_iter;
        if (v != w)
          node_pool.push_back(nodes[v].FindSet());
        else
          type[w] = BB_SELF;
      }

      // Copy node_pool to worklist.
      //
      NodeList worklist;
      NodeList::iterator niter  = node_pool.begin();
      NodeList::iterator nend   = node_pool.end();
      for (;  niter != nend; ++niter)
        worklist.push_back(*niter);

      if (!node_pool.empty())
        type[w] = BB_REDUCIBLE;

      // work the list...
      //
      while (!worklist.empty()) {
        UnionFindNode x = *worklist.front();
        worklist.pop_front();

        // Step e:
        //
        // Step e represents the main difference from Tarjan's method.
        // Chasing upwards from the sources of a node w's backedges. If
        // there is a node y' that is not a descendant of w, w is marked
        // the header of an irreducible loop, there is another entry
        // into this loop that avoids w.
        //

        // The algorithm has degenerated. Break and
        // return in this case.
        //
        size_t non_back_size = non_back_preds[x.dfs_number()].size();
        if (non_back_size > kMaxNonBackPreds) {
          lsg_->KillAll();
          return;
        }

        IntSet::iterator non_back_pred_iter =
          non_back_preds[x.dfs_number()].begin();
        IntSet::iterator non_back_pred_end  =
          non_back_preds[x.dfs_number()].end();
        for (; non_back_pred_iter != non_back_pred_end; non_back_pred_iter++) {
          UnionFindNode  y     = nodes[*non_back_pred_iter];
          UnionFindNode *ydash = y.FindSet();

          if (!IsAncestor(w, ydash->dfs_number(), &last)) {
            type[w] = BB_IRREDUCIBLE;
            non_back_preds[w].insert(ydash->dfs_number());
          } else {
            if (ydash->dfs_number() != w) {
              NodeList::iterator nfind = find(node_pool.begin(),
                                              node_pool.end(), ydash);
              if (nfind == node_pool.end()) {
                worklist.push_back(ydash);
                node_pool.push_back(ydash);
              }
            }
          }
        }
      }

      // Collapse/Unionize nodes in a SCC to a single node
      // For every SCC found, create a loop descriptor and link it in.
      //
      if (!node_pool.empty() || (type[w] == BB_SELF)) {
        SimpleLoop* loop = lsg_->CreateNewLoop();

        // At this point, one can set attributes to the loop, such as:
        //
        // the bottom node:
        //    IntList::iterator iter  = back_preds[w].begin();
        //    loop bottom is: nodes[*backp_iter].node);
        //
        // the number of backedges:
        //    back_preds[w].size()
        //
        // whether this loop is reducible:
        //    type[w] != BB_IRREDUCIBLE
        //
        // TODO(rhundt): Define those interfaces in the Loop Forest.
        //
        nodes[w].set_loop(loop);

        for (niter = node_pool.begin(); niter != node_pool.end(); niter++) {
          UnionFindNode  *node = (*niter);

          // Add nodes to loop descriptor.
          header[node->dfs_number()] = w;
          node->Union(&nodes[w]);

          // Nested loops are not added, but linked together.
          if (node->loop())
            node->loop()->set_parent(loop);
          else
            loop->AddNode(node->bb());
        }

        lsg_->AddLoop(loop);
      }  // node_pool.size
    }  // Step c
  }  // FindLoops

 private:
  MaoCFG             *CFG_;      // current control flow graph.
  LoopStructureGraph *lsg_;      // loop forest.
};  // HavlakLoopFinder


// Constant instantiations.
//
const int HavlakLoopFinder::kUnvisited;
const int HavlakLoopFinder::kMaxNonBackPreds;

// External entry point.
int FindHavlakLoops(MaoCFG *CFG, LoopStructureGraph *LSG) {
  HavlakLoopFinder finder(CFG, LSG);
  finder.FindLoops();
  return LSG->GetNumLoops();
}

#include <stdio.h>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

int buildDiamond(MaoCFG *cfg, int start) {
  int bb0 = start;

  new BasicBlockEdge(cfg, bb0, bb0 + 1);
  new BasicBlockEdge(cfg, bb0, bb0 + 2);
  new BasicBlockEdge(cfg, bb0 + 1, bb0 + 3);
  new BasicBlockEdge(cfg, bb0 + 2, bb0 + 3);

  return bb0 + 3;
}

void buildConnect(MaoCFG *cfg, int start, int end) {
  new BasicBlockEdge(cfg, start, end);
}

int buildStraight(MaoCFG *cfg, int start, int n) {
  for (int i = 0; i < n; i++) {
    buildConnect(cfg, start + i, start + i + 1);
  }

  return start + n;
}

int buildBaseLoop(MaoCFG *cfg, int from) {
  int header = buildStraight(cfg, from, 1);
  int diamond1 = buildDiamond(cfg, header);
  int d11 = buildStraight(cfg, diamond1, 1);
  int diamond2 = buildDiamond(cfg, d11);
  int footer = buildStraight(cfg, diamond2, 1);
  buildConnect(cfg, diamond2, d11);
  buildConnect(cfg, diamond1, header);

  buildConnect(cfg, footer, from);
  footer = buildStraight(cfg, footer, 1);
  return footer;
}


int main(int argc, char **argv) {
  int NUM;

  int arg = argc > 1 ? argv[1][0] - '0' : 3;
  switch(arg) {
    case 0: return 0; break;
    case 1: NUM = 10; break;
    case 2: NUM = 30; break;
    case 3: NUM = 60; break;
    case 4: NUM = 100; break;
    case 5: NUM = 150; break;
    default: printf("error: %d\\n", arg); return -1;
  }

  fprintf(stderr, "Welcome to LoopTesterApp, C++ edition\n");
  MaoCFG cfg;
  LoopStructureGraph lsg;

  fprintf(stderr, "Constructing Simple CFG...\n");
  cfg.CreateNode(0);  // top
  buildBaseLoop(&cfg, 0);
  cfg.CreateNode(1);  // bottom
  new BasicBlockEdge(&cfg, 0,  2);

  fprintf(stderr, "15000 dummy loops\n");
  for (int dummyloops = 0; dummyloops < 15000; ++dummyloops) {
    LoopStructureGraph * lsglocal = new LoopStructureGraph();
    FindHavlakLoops(&cfg, lsglocal);
    delete(lsglocal);
  }

  fprintf(stderr, "Constructing CFG...\n");
  int n = 2;

  for (int parlooptrees = 0; parlooptrees < 10; parlooptrees++) {
    cfg.CreateNode(n + 1);
    buildConnect(&cfg, 2, n + 1);
    n = n + 1;

    for (int i = 0; i < 20; i++) {
      int top = n;
      n = buildStraight(&cfg, n, 1);
      for (int j = 0; j < 25; j++) {
        n = buildBaseLoop(&cfg, n);
      }
      int bottom = buildStraight(&cfg, n, 1);
      buildConnect(&cfg, n, top);
      n = bottom;
    }
    buildConnect(&cfg, n, 1);
  }

  fprintf(stderr, "Performing Loop Recognition\n1 Iteration\n");
  int num_loops = FindHavlakLoops(&cfg, &lsg);

  fprintf(stderr, "Another %d iterations...\n", NUM);
  int sum = 0;
  for (int i = 0; i < NUM; i++) {
    LoopStructureGraph lsg;
    //fprintf(stderr, ".");
    sum += FindHavlakLoops(&cfg, &lsg);
  }
  fprintf(stderr,
          "\nFound %d loops (including artificial root node)"
          "(%d)\n", num_loops, sum);
}

