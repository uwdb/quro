//===- LiveVariables.h - Live Variable Analysis for Source CFGs -*- C++ --*-//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements Live Variables analysis for source-level CFGs.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_ANALYSIS_ANALYSES_LIVEVARIABLES_H
#define LLVM_CLANG_ANALYSIS_ANALYSES_LIVEVARIABLES_H


#include "clang/AST/Decl.h"
#include "clang/Analysis/AnalysisContext.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/ImmutableSet.h"
#include "clang/Analysis/CFG.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Analysis/Analyses/PostOrderCFGView.h" 
#include "clang/AST/StmtVisitor.h"
#include <unordered_set>
#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <set>
#include "QueryBlock.h"
using namespace std;

using namespace clang;



class myLiveVariables : public ManagedAnalysis {
public:
 class BlockDepGraphNode{
 public:
	unordered_set<const CFGBlock *> parents;
	unordered_set<const CFGBlock *> children;
	int level;
	BlockDepGraphNode(){
		level = 0;
	}
	void addParent(const CFGBlock * b){
		parents.insert(b);
	}
	void addChild(const CFGBlock * b){
		children.insert(b);
	}
 };
 class DependentSet{
 public:
	const Stmt* stmt;
	const VarDecl* var;
	DependentSet(const Stmt* _stmt, const VarDecl* _var){
		stmt = _stmt;
		var = _var;
	}
 };
  
  class DefUseValues {
  public:
  	std::unordered_set<codeBlock*> stmts;
	void add(codeBlock* cb){
		stmts.insert(cb);
	}
	void print(SourceManager& M){
			cout<<"defuse value:"<<endl;
			for(unordered_set<codeBlock*>::iterator it = stmts.begin(); it != stmts.end(); it++){
					cout<<"\t"<<(*it)->label<<endl;
			}
	}
	DefUseValues subtract_list(const DefUseValues& b){
		DefUseValues val;
		for(std::unordered_set<codeBlock*>::iterator it = stmts.begin();
			it != stmts.end(); it++){
			//in b's set, don't add
			if (b.stmts.find(*it)==b.stmts.end()){
				val.add(*it);
			}
		}
		return val;
	}
	DefUseValues merge_list(DefUseValues b){
		DefUseValues val;
		for(std::unordered_set<codeBlock*>::iterator it = stmts.begin();
			it != stmts.end(); it++){
			val.add(*it);
		}
		for(std::unordered_set<codeBlock*>::iterator it = b.stmts.begin();
			it != b.stmts.end(); it++){
			val.add(*it);
		}
		return val;
	}
	void add_list(DefUseValues b){
		for(std::unordered_set<codeBlock*>::iterator it = b.stmts.begin();
			it != b.stmts.end(); it++){
			add(*it);
		}
	}
	bool equals(DefUseValues &d){
		if(stmts.size() != d.stmts.size())
			return false;
		for(std::unordered_set<codeBlock*>::iterator it = stmts.begin();
		it != stmts.end(); it++){
			if(d.stmts.find(*it)==d.stmts.end())
				return false;
		}
		return true;
	}
  };
  class Vars {
  public:
  	std::map<const VarDecl *, codeBlock*> variables;
	void add(const VarDecl* _var, codeBlock* _cb){
		variables[_var] = _cb;
	}
	void print(SourceManager& M){
			cout<<"vars: "<<endl;
			for(std::map<const VarDecl*, codeBlock*>::iterator it = variables.begin(); it != variables.end(); it++){
					cout<<"\t"<<it->first->getNameAsString()<<" : "<<it->second->label<<endl;
			}
	}
	DefUseValues getStmtList(){
		DefUseValues val;
		for(std::map<const VarDecl*, codeBlock*>::iterator it = variables.begin();
			it != variables.end(); it++){
				val.add(it->second);
		}
		return val;
	}

  };
 class LivenessValues {
  public:

    llvm::ImmutableSet<const Stmt *> liveStmts;
    llvm::ImmutableSet<const VarDecl *> liveDecls;
    
    bool equals(const LivenessValues &V) const;

    LivenessValues()
      : liveStmts(nullptr), liveDecls(nullptr) {}

    LivenessValues(llvm::ImmutableSet<const Stmt *> LiveStmts,
                   llvm::ImmutableSet<const VarDecl *> LiveDecls)
      : liveStmts(LiveStmts), liveDecls(LiveDecls) {}

    ~LivenessValues() {}
    
    bool isLive(const Stmt *S) const;
    bool isLive(const VarDecl *D) const;
    
    friend class myLiveVariables;    
  };
  
  class Observer {
    virtual void anchor();
  public:
    virtual ~Observer() {}
    
    /// A callback invoked right before invoking the
    ///  liveness transfer function on the given statement.
    virtual void observeStmt(const Stmt *S,
                             const CFGBlock *currentBlock,
                             const LivenessValues& V) {}
    
    /// Called when the live variables analysis registers
    /// that a variable is killed.
    virtual void observerKill(const DeclRefExpr *DR) {}
  };    


  virtual ~myLiveVariables();
  
  /// Compute the liveness information for a given CFG.
  static myLiveVariables *computeLiveness(AnalysisDeclContext &analysisContext,
                                        bool killAtAssign,
										SourceManager& M);
  
  /// Return true if a variable is live at the end of a
  /// specified block.
  bool isLive(const CFGBlock *B, const VarDecl *D);
  
  /// Returns true if a variable is live at the beginning of the
  ///  the statement.  This query only works if liveness information
  ///  has been recorded at the statement level (see runOnAllBlocks), and
  ///  only returns liveness information for block-level expressions.
  bool isLive(const Stmt *S, const VarDecl *D);
  
  /// Returns true the block-level expression "value" is live
  ///  before the given block-level expression (see runOnAllBlocks).
  bool isLive(const Stmt *Loc, const Stmt *StmtVal);
    
  /// Print to stderr the liveness information associated with
  /// each basic block.
  void dumpBlockLiveness(const SourceManager& M);

  void runOnAllBlocks(Observer &obs);
  
  static myLiveVariables *create(AnalysisDeclContext &analysisContext, SourceManager& M) {
    return computeLiveness(analysisContext, true, M);
  }
  
  static const void *getTag();
  
private:
  myLiveVariables(void *impl);
  void *impl;
};
  
class RelaxedMyLiveVariables : public myLiveVariables {
public:
  static myLiveVariables *create(AnalysisDeclContext &analysisContext, SourceManager& M) {
    return computeLiveness(analysisContext, false, M);
  }
  
  static const void *getTag();
};
  
class LiveVariablesImpl {
public:  
  AnalysisDeclContext &analysisContext;
  llvm::ImmutableSet<const Stmt *>::Factory SSetFact;
  llvm::ImmutableSet<const VarDecl *>::Factory DSetFact;
	llvm::DenseMap<const CFGBlock *, myLiveVariables::LivenessValues> blocksEndToLiveness;
  llvm::DenseMap<const CFGBlock *, myLiveVariables::LivenessValues> blocksBeginToLiveness;
  llvm::DenseMap<const Stmt *, myLiveVariables::LivenessValues> stmtsToLiveness;
  //reaching definition
  llvm::DenseMap<const CFGBlock *, myLiveVariables::DefUseValues> blocksBeginToDefuse; 
  llvm::DenseMap<const CFGBlock *, myLiveVariables::DefUseValues> blocksEndToDefuse;
  llvm::DenseMap<const CFGBlock *, myLiveVariables::Vars> gen;
  llvm::DenseMap<const CFGBlock *, myLiveVariables::DefUseValues> kill;
	llvm::DenseMap<const CFGBlock *, SourceRange> blocks_range;

	llvm::DenseMap<const CFGBlock *, vector<codeBlock*> > CFG_to_stmts;
	llvm::DenseMap<const CFGBlock *, vector<condBlock*> > CFG_to_conds;


  //FIXME: mapping each statement to its CFGBlock, should store this info in a more efficient way!
  llvm::DenseMap<const Stmt *, const CFGBlock *> stmtToBlock;
  llvm::DenseMap<const DeclRefExpr *, unsigned> inAssignment;
  const bool killAtAssign;
  
  myLiveVariables::LivenessValues
  merge(myLiveVariables::LivenessValues valsA,
        myLiveVariables::LivenessValues valsB);

  myLiveVariables::DefUseValues 
  merge(myLiveVariables::DefUseValues valsA,
  		myLiveVariables::DefUseValues valsB);

  myLiveVariables::LivenessValues
  runOnBlock(const CFGBlock *block, myLiveVariables::LivenessValues val,
             myLiveVariables::Observer *obs = nullptr);

  void dumpBlockLiveness(const SourceManager& M);

//Helper functions

  LiveVariablesImpl(AnalysisDeclContext &ac, bool KillAtAssign)
    : analysisContext(ac),
      SSetFact(false), // Do not canonicalize ImmutableSets by default.
      DSetFact(false), // This is a *major* performance win.
      killAtAssign(KillAtAssign) {}
};

class DataflowWorklist {
  SmallVector<const CFGBlock *, 20> worklist;
  llvm::BitVector enqueuedBlocks;
  PostOrderCFGView *POV;
public:
  DataflowWorklist(const CFG &cfg, AnalysisDeclContext &Ctx)
    : enqueuedBlocks(cfg.getNumBlockIDs()),
      POV(Ctx.getAnalysis<PostOrderCFGView>()) {}
  
  void enqueueBlock(const CFGBlock *block);
  void enqueuePredecessors(const CFGBlock *block);
  void enqueueSuccessors(const CFGBlock *block);

  const CFGBlock *dequeue();

  void sortWorklist();
  void reverseSortWorklist();
};

class TransferFunctions : public StmtVisitor<TransferFunctions> {
  LiveVariablesImpl &LV;
  myLiveVariables::LivenessValues &val;
  myLiveVariables::Observer *observer;
  const CFGBlock *currentBlock;
public:
  TransferFunctions(LiveVariablesImpl &im,
                    myLiveVariables::LivenessValues &Val,
                    myLiveVariables::Observer *Observer,
                    const CFGBlock *CurrentBlock)
  : LV(im), val(Val), observer(Observer), currentBlock(CurrentBlock) {}

  void VisitBinaryOperator(BinaryOperator *BO);
  void VisitBlockExpr(BlockExpr *BE);
  void VisitDeclRefExpr(DeclRefExpr *DR);  
  void VisitDeclStmt(DeclStmt *DS);
  void VisitObjCForCollectionStmt(ObjCForCollectionStmt *OS);
  void VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *UE);
  void VisitUnaryOperator(UnaryOperator *UO);
  void Visit(Stmt *S);
};


#endif
