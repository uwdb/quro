#include "ControlFlowGraph.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/Analysis/Analyses/PostOrderCFGView.h"
#include "clang/Analysis/AnalysisContext.h"
#include "clang/Analysis/CFG.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <string.h>
using namespace std;
using namespace clang;

void DataflowWorklist::enqueueBlock(const clang::CFGBlock *block) {
  if (block && !enqueuedBlocks[block->getBlockID()]) {
    enqueuedBlocks[block->getBlockID()] = true;
    worklist.push_back(block);
  }
}

void DataflowWorklist::enqueuePredecessors(const clang::CFGBlock *block) {
  const unsigned OldWorklistSize = worklist.size();
  for (CFGBlock::const_pred_iterator I = block->pred_begin(),
       E = block->pred_end(); I != E; ++I) {
    enqueueBlock(*I);
  }
  
  if (OldWorklistSize == 0 || OldWorklistSize == worklist.size())
    return;

//  sortWorklist();
}

void DataflowWorklist::enqueueSuccessors(const clang::CFGBlock *block) {
  const unsigned OldWorklistSize = worklist.size();
  for (CFGBlock::const_succ_iterator I = block->succ_begin(),
  		E = block->succ_end(); I != E; ++I){
	enqueueBlock(*I);
  }
//  reverseSortWorklist();
//  if (OldWorklistSize == 0 || OldWorklistSize == worklist.size())
//  	return ;
//  sortWorklist();
}

void DataflowWorklist::sortWorklist() {
  std::sort(worklist.begin(), worklist.end(), POV->getComparator());
}

void DataflowWorklist::reverseSortWorklist() {
  sortWorklist();
  std::vector<const CFGBlock *> temp_vec;
  for(int i=0; i<worklist.size(); i++){
 	temp_vec.push_back(worklist[i]);
  }
  worklist.clear();
  for(int i=temp_vec.size()-1; i>=0; i--){
  	worklist.push_back(temp_vec[i]);
  }
}

const CFGBlock *DataflowWorklist::dequeue() {
  if (worklist.empty())
    return nullptr;
  const CFGBlock *b = worklist.pop_back_val();
  enqueuedBlocks[b->getBlockID()] = false;
  return b;
}

static LiveVariablesImpl &getImpl(void *x) {
  return *((LiveVariablesImpl *) x);
}

//===----------------------------------------------------------------------===//
// Operations and queries on LivenessValues.
//===----------------------------------------------------------------------===//

bool myLiveVariables::LivenessValues::isLive(const Stmt *S) const {
  return liveStmts.contains(S);
}

bool myLiveVariables::LivenessValues::isLive(const VarDecl *D) const {
  return liveDecls.contains(D);
}

  template <typename SET>
  SET mergeSets(SET A, SET B) {
    if (A.isEmpty())
      return B;
    
    for (typename SET::iterator it = B.begin(), ei = B.end(); it != ei; ++it) {
      A = A.add(*it);
    }
    return A;
  }

void myLiveVariables::Observer::anchor() { }

myLiveVariables::LivenessValues
LiveVariablesImpl::merge(myLiveVariables::LivenessValues valsA,
                         myLiveVariables::LivenessValues valsB) {  
  
  llvm::ImmutableSetRef<const Stmt *>
    SSetRefA(valsA.liveStmts.getRootWithoutRetain(), SSetFact.getTreeFactory()),
    SSetRefB(valsB.liveStmts.getRootWithoutRetain(), SSetFact.getTreeFactory());
                                                
  
  llvm::ImmutableSetRef<const VarDecl *>
    DSetRefA(valsA.liveDecls.getRootWithoutRetain(), DSetFact.getTreeFactory()),
    DSetRefB(valsB.liveDecls.getRootWithoutRetain(), DSetFact.getTreeFactory());
  

  SSetRefA = mergeSets(SSetRefA, SSetRefB);
  DSetRefA = mergeSets(DSetRefA, DSetRefB);
  
  // asImmutableSet() canonicalizes the tree, allowing us to do an easy
  // comparison afterwards.
  return myLiveVariables::LivenessValues(SSetRefA.asImmutableSet(),
                                       DSetRefA.asImmutableSet());  
}

bool myLiveVariables::LivenessValues::equals(const LivenessValues &V) const {
  return liveStmts == V.liveStmts && liveDecls == V.liveDecls;
}

//===----------------------------------------------------------------------===//
// Query methods.
//===----------------------------------------------------------------------===//

static bool isAlwaysAlive(const VarDecl *D) {
  return D->hasGlobalStorage();
}

bool myLiveVariables::isLive(const CFGBlock *B, const VarDecl *D) {
  return isAlwaysAlive(D) || getImpl(impl).blocksEndToLiveness[B].isLive(D);
}

bool myLiveVariables::isLive(const Stmt *S, const VarDecl *D) {
  return isAlwaysAlive(D) || getImpl(impl).stmtsToLiveness[S].isLive(D);
}

bool myLiveVariables::isLive(const Stmt *Loc, const Stmt *S) {
  return getImpl(impl).stmtsToLiveness[Loc].isLive(S);
}

//===----------------------------------------------------------------------===//
// Dataflow computation.
//===----------------------------------------------------------------------===//

//class TransferFunctions : public StmtVisitor<TransferFunctions> {
//  LiveVariablesImpl &LV;
//  LiveVariables::LivenessValues &val;
//  LiveVariables::Observer *observer;
//  const CFGBlock *currentBlock;
//public:
//  TransferFunctions(LiveVariablesImpl &im,
//                    LiveVariables::LivenessValues &Val,
//                    LiveVariables::Observer *Observer,
//                    const CFGBlock *CurrentBlock)
//  : LV(im), val(Val), observer(Observer), currentBlock(CurrentBlock) {}
//
//  void VisitBinaryOperator(BinaryOperator *BO);
//  void VisitBlockExpr(BlockExpr *BE);
//  void VisitDeclRefExpr(DeclRefExpr *DR);  
//  void VisitDeclStmt(DeclStmt *DS);
//  void VisitObjCForCollectionStmt(ObjCForCollectionStmt *OS);
//  void VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *UE);
//  void VisitUnaryOperator(UnaryOperator *UO);
//  void Visit(Stmt *S);
//};
//
static const VariableArrayType *FindVA(QualType Ty) {
  const Type *ty = Ty.getTypePtr();
  while (const ArrayType *VT = dyn_cast<ArrayType>(ty)) {
    if (const VariableArrayType *VAT = dyn_cast<VariableArrayType>(VT))
      if (VAT->getSizeExpr())
        return VAT;
    
    ty = VT->getElementType().getTypePtr();
  }

  return nullptr;
}

static const Stmt *LookThroughStmt(const Stmt *S) {
  while (S) {
    if (const Expr *Ex = dyn_cast<Expr>(S))
      S = Ex->IgnoreParens();    
    if (const ExprWithCleanups *EWC = dyn_cast<ExprWithCleanups>(S)) {
      S = EWC->getSubExpr();
      continue;
    }
    if (const OpaqueValueExpr *OVE = dyn_cast<OpaqueValueExpr>(S)) {
      S = OVE->getSourceExpr();
      continue;
    }
    break;
  }
  return S;
}

static void AddLiveStmt(llvm::ImmutableSet<const Stmt *> &Set,
                        llvm::ImmutableSet<const Stmt *>::Factory &F,
                        const Stmt *S) {
  Set = F.add(Set, LookThroughStmt(S));
}

void TransferFunctions::Visit(Stmt *S) {
  if (observer)
    observer->observeStmt(S, currentBlock, val);
  
  StmtVisitor<TransferFunctions>::Visit(S);
  
  if (isa<Expr>(S)) {
    val.liveStmts = LV.SSetFact.remove(val.liveStmts, S);
  }

  // Mark all children expressions live.
  
  switch (S->getStmtClass()) {
    default:
      break;
    case Stmt::StmtExprClass: {
      // For statement expressions, look through the compound statement.
      S = cast<StmtExpr>(S)->getSubStmt();
      break;
    }
    case Stmt::CXXMemberCallExprClass: {
      // Include the implicit "this" pointer as being live.
      CXXMemberCallExpr *CE = cast<CXXMemberCallExpr>(S);
      if (Expr *ImplicitObj = CE->getImplicitObjectArgument()) {
        AddLiveStmt(val.liveStmts, LV.SSetFact, ImplicitObj);
      }
      break;
    }
    case Stmt::ObjCMessageExprClass: {
      // In calls to super, include the implicit "self" pointer as being live.
      ObjCMessageExpr *CE = cast<ObjCMessageExpr>(S);
      if (CE->getReceiverKind() == ObjCMessageExpr::SuperInstance)
        val.liveDecls = LV.DSetFact.add(val.liveDecls,
                                        LV.analysisContext.getSelfDecl());
      break;
    }
    case Stmt::DeclStmtClass: {
      const DeclStmt *DS = cast<DeclStmt>(S);
      if (const VarDecl *VD = dyn_cast<VarDecl>(DS->getSingleDecl())) {
        for (const VariableArrayType* VA = FindVA(VD->getType());
             VA != nullptr; VA = FindVA(VA->getElementType())) {
          AddLiveStmt(val.liveStmts, LV.SSetFact, VA->getSizeExpr());
        }
      }
      break;
    }
    case Stmt::PseudoObjectExprClass: {
      // A pseudo-object operation only directly consumes its result
      // expression.
      Expr *child = cast<PseudoObjectExpr>(S)->getResultExpr();
      if (!child) return;
      if (OpaqueValueExpr *OV = dyn_cast<OpaqueValueExpr>(child))
        child = OV->getSourceExpr();
      child = child->IgnoreParens();
      val.liveStmts = LV.SSetFact.add(val.liveStmts, child);
      return;
    }

    // FIXME: These cases eventually shouldn't be needed.
    case Stmt::ExprWithCleanupsClass: {
      S = cast<ExprWithCleanups>(S)->getSubExpr();
      break;
    }
    case Stmt::CXXBindTemporaryExprClass: {
      S = cast<CXXBindTemporaryExpr>(S)->getSubExpr();
      break;
    }
    case Stmt::UnaryExprOrTypeTraitExprClass: {
      // No need to unconditionally visit subexpressions.
      return;
    }
  }
  
  for (Stmt::child_iterator it = S->child_begin(), ei = S->child_end();
       it != ei; ++it) {
    if (Stmt *child = *it)
      AddLiveStmt(val.liveStmts, LV.SSetFact, child);
  }
}
void TransferFunctions::VisitBinaryOperator(BinaryOperator *B) {
  if (B->isAssignmentOp()) {
    if (!LV.killAtAssign)
      return;
    
    // Assigning to a variable?
    Expr *LHS = B->getLHS()->IgnoreParens();
    
    if (DeclRefExpr *DR = dyn_cast<DeclRefExpr>(LHS))
      if (const VarDecl *VD = dyn_cast<VarDecl>(DR->getDecl())) {
        // Assignments to references don't kill the ref's address
        if (VD->getType()->isReferenceType())
          return;

        if (!isAlwaysAlive(VD)) {
          // The variable is now dead.
          val.liveDecls = LV.DSetFact.remove(val.liveDecls, VD);
        }

        if (observer)
          observer->observerKill(DR);
      }
  }
}

void TransferFunctions::VisitBlockExpr(BlockExpr *BE) {
  AnalysisDeclContext::referenced_decls_iterator I, E;
  std::tie(I, E) =
    LV.analysisContext.getReferencedBlockVars(BE->getBlockDecl());
  for ( ; I != E ; ++I) {
    const VarDecl *VD = *I;
    if (isAlwaysAlive(VD))
      continue;
    val.liveDecls = LV.DSetFact.add(val.liveDecls, VD);
  }
}

void TransferFunctions::VisitDeclRefExpr(DeclRefExpr *DR) {
  if (const VarDecl *D = dyn_cast<VarDecl>(DR->getDecl()))
    if (!isAlwaysAlive(D) && LV.inAssignment.find(DR) == LV.inAssignment.end())
      val.liveDecls = LV.DSetFact.add(val.liveDecls, D);
}

void TransferFunctions::VisitDeclStmt(DeclStmt *DS) {
  for (const auto *DI : DS->decls())
    if (const auto *VD = dyn_cast<VarDecl>(DI)) {
      if (!isAlwaysAlive(VD))
        val.liveDecls = LV.DSetFact.remove(val.liveDecls, VD);
    }
}

void TransferFunctions::VisitObjCForCollectionStmt(ObjCForCollectionStmt *OS) {
  // Kill the iteration variable.
  DeclRefExpr *DR = nullptr;
  const VarDecl *VD = nullptr;

  Stmt *element = OS->getElement();
  if (DeclStmt *DS = dyn_cast<DeclStmt>(element)) {
    VD = cast<VarDecl>(DS->getSingleDecl());
  }
  else if ((DR = dyn_cast<DeclRefExpr>(cast<Expr>(element)->IgnoreParens()))) {
    VD = cast<VarDecl>(DR->getDecl());
  }
  
  if (VD) {
    val.liveDecls = LV.DSetFact.remove(val.liveDecls, VD);
    if (observer && DR)
      observer->observerKill(DR);
  }
}

void TransferFunctions::
VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *UE)
{
  // While sizeof(var) doesn't technically extend the liveness of 'var', it
  // does extent the liveness of metadata if 'var' is a VariableArrayType.
  // We handle that special case here.
  if (UE->getKind() != UETT_SizeOf || UE->isArgumentType())
    return;

  const Expr *subEx = UE->getArgumentExpr();
  if (subEx->getType()->isVariableArrayType()) {
    assert(subEx->isLValue());
    val.liveStmts = LV.SSetFact.add(val.liveStmts, subEx->IgnoreParens());
  }
}

void TransferFunctions::VisitUnaryOperator(UnaryOperator *UO) {
  // Treat ++/-- as a kill.
  // Note we don't actually have to do anything if we don't have an observer,
  // since a ++/-- acts as both a kill and a "use".
  if (!observer)
    return;
  
  switch (UO->getOpcode()) {
  default:
    return;
  case UO_PostInc:
  case UO_PostDec:    
  case UO_PreInc:
  case UO_PreDec:
    break;
  }
  
  if (DeclRefExpr *DR = dyn_cast<DeclRefExpr>(UO->getSubExpr()->IgnoreParens()))
    if (isa<VarDecl>(DR->getDecl())) {
      // Treat ++/-- as a kill.
      observer->observerKill(DR);
    }
}

myLiveVariables::LivenessValues
LiveVariablesImpl::runOnBlock(const CFGBlock *block,
                              myLiveVariables::LivenessValues val,
                              myLiveVariables::Observer *obs) {
  TransferFunctions TF(*this, val, obs, block);
  
  // Visit the terminator (if any).
  if (const Stmt *term = block->getTerminator())
    TF.Visit(const_cast<Stmt*>(term));
  
  // Apply the transfer function for all Stmts in the block.
  for (CFGBlock::const_reverse_iterator it = block->rbegin(),
       ei = block->rend(); it != ei; ++it) {
    const CFGElement &elem = *it;

    if (Optional<CFGAutomaticObjDtor> Dtor =
            elem.getAs<CFGAutomaticObjDtor>()) {
      val.liveDecls = DSetFact.add(val.liveDecls, Dtor->getVarDecl());
      continue;
    }

    if (!elem.getAs<CFGStmt>())
      continue;
    
    const Stmt *S = elem.castAs<CFGStmt>().getStmt();
    TF.Visit(const_cast<Stmt*>(S));
    stmtsToLiveness[S] = val;
  }
  return val;
}

void myLiveVariables::runOnAllBlocks(myLiveVariables::Observer &obs) {
  const CFG *cfg = getImpl(impl).analysisContext.getCFG();
  for (CFG::const_iterator it = cfg->begin(), ei = cfg->end(); it != ei; ++it)
    getImpl(impl).runOnBlock(*it, getImpl(impl).blocksEndToLiveness[*it], &obs);    
}

myLiveVariables::myLiveVariables(void *im) : impl(im) {} 

myLiveVariables::~myLiveVariables() {
  delete (LiveVariablesImpl*) impl;
}

myLiveVariables *
myLiveVariables::computeLiveness(AnalysisDeclContext &AC,
                                 bool killAtAssign,
								 SourceManager& M) {

  // No CFG?  Bail out.
  CFG *cfg = AC.getCFG();
  if (!cfg){
    return nullptr;
  }

 // The analysis currently has scalability issues for very large CFGs.
  // Bail out if it looks too large.
  if (cfg->getNumBlockIDs() > 300000)
    return nullptr;


  LiveVariablesImpl *LV = new LiveVariablesImpl(AC, killAtAssign);

  // Construct the dataflow worklist.  Enqueue the exit block as the
  // start of the analysis.
  DataflowWorklist worklist(*cfg, AC);
  llvm::BitVector everAnalyzedBlock(cfg->getNumBlockIDs());

  // FIXME: we should enqueue using post order.
  for (CFG::const_iterator it = cfg->begin(), ei = cfg->end(); it != ei; ++it) {
    const CFGBlock *block = *it;
    worklist.enqueueBlock(block);

		cout<<"*****   CFG BLOCK:  "<<block->getBlockID()<<" *****"<<endl;
		SourceLocation begin;
		SourceLocation end;
		vector<codeBlock*> cbs;
		LV->CFG_to_stmts[block] = cbs;
		//DefUseValues temp_def;
		Vars temp_var;
		//gen[block] = temp_def;
		LV->gen[block] = temp_var;
    // FIXME: Scan for DeclRefExprs using in the LHS of an assignment.
    // We need to do this because we lack context in the reverse analysis
    // to determine if a DeclRefExpr appears in such a context, and thus
    // doesn't constitute a "use".
		BeforeThanCompare<SourceLocation> Compare(M);

    if (killAtAssign)
      for (CFGBlock::const_iterator bi = block->begin(), be = block->end();
           bi != be; ++bi) {
        if (Optional<CFGStmt> cs = bi->getAs<CFGStmt>()) {
#ifdef DEBUG_CFG
					cout<<"\t\t";
					printSourceRange(cs->getStmt()->getLocStart(), cs->getStmt()->getLocEnd(), M);
#endif
					if(begin.isValid()==false || (cs->getStmt()->getLocStart().isValid() && Compare(cs->getStmt()->getLocStart(), begin))){
							begin = cs->getStmt()->getLocStart();
					}
					if(end.isValid()==false || (cs->getStmt()->getLocEnd().isValid() && Compare(end, cs->getStmt()->getLocEnd())))
						end = cs->getStmt()->getLocEnd();
        }
      }
			//assert(begin.isValid());
			//assert(end.isValid());
			SourceRange sr(begin, end);
			LV->blocks_range[block] = sr;
 	} 
  worklist.sortWorklist();
	for(int i=0; i<code_blocks.size(); i++){
			for(CFG::const_iterator it = cfg->begin(), ei = cfg->end(); it != ei; ++it){
					const CFGBlock *block = *it;
					if(rangeIsWithin(code_blocks[i]->begin, code_blocks[i]->end, LV->blocks_range[block].getBegin(), LV->blocks_range[block].getEnd(), M)){
#ifdef DEBUG_CFG
					cout<<"code block ("<<code_blocks[i]->label<<") belongs to CFGBlock "<<block->getBlockID()<<endl;
#endif
					LV->CFG_to_stmts[block].push_back(code_blocks[i]);
					for(set<const VarDecl*>::iterator vit = code_blocks[i]->defs.begin(); vit != code_blocks[i]->defs.end(); vit++){
							LV->gen[block].add(*vit, code_blocks[i]);
					}
				}
			}
	}

	for(int i=0; i<cond_blocks.size(); i++){
			cout<<"CONDITIONAL block: ";
			printSourceRange(cond_blocks[i]->stmt->getLocStart(), cond_blocks[i]->stmt->getLocEnd(), M);
			printSourceRange(cond_blocks[i]->cond->getLocStart(), cond_blocks[i]->cond->getLocEnd(), M);
			for(CFG::const_iterator it = cfg->begin(), ei = cfg->end(); it != ei; ++it){
					const CFGBlock *block = *it;
					if(rangeIsWithin(cond_blocks[i]->cond->getLocStart(), LV->blocks_range[block].getBegin(), LV->blocks_range[block].getEnd(), M)
							||rangeIsWithin(cond_blocks[i]->cond->getLocEnd(), LV->blocks_range[block].getBegin(), LV->blocks_range[block].getEnd(), M)){
#ifdef DEBUG_CFG
					cout<<"conditional block: \t";
					//printSourceRange(cond_blocks[i]->stmt->getLocStart(), cond_blocks[i]->stmt->getLocEnd(), M);
					cout<<"\tits condition belongs to CFGBlock "<<block->getBlockID()<<endl;
#endif
					//LV->CFG_to_stmts[block].push_back(code_blocks[i]);
					LV->CFG_to_conds[block].push_back(cond_blocks[i]);
				}
			}
	}

 
//compute reaching definition
  for (CFG::const_iterator it = cfg->begin(), ei = cfg->end(); it != ei; ++it) {
    //compute kill set
    const CFGBlock *block = *it;

    DataflowWorklist temp_list(*cfg, AC);
	std::unordered_set<const CFGBlock*> counted_pred;
	counted_pred.insert(block);
	temp_list.enqueuePredecessors(block);
	DefUseValues temp_def;
	LV->kill[block] = temp_def;

	Vars myvar = LV->gen[block];
	while(const CFGBlock *pred_block = temp_list.dequeue()){
		for (std::map<const VarDecl*, codeBlock*>::iterator v_it = myvar.variables.begin();
			v_it != myvar.variables.end(); v_it++){
			//defined variable is defined a predecessor
			//v_it->first is the key (VarDecl*)
				if(LV->gen[pred_block].variables.find(v_it->first)!=
					LV->gen[pred_block].variables.end()){
					//add to its kill set
					//FIXME: if a variable is used before a define statement in this block, the previous define of this variable should not be in the kill set
						LV->kill[block].add(LV->gen[pred_block].variables[v_it->first]);
				}
		}
		for (CFGBlock::const_pred_iterator b_it = pred_block->pred_begin(),
                                       b_ei = pred_block->pred_end(); b_it != b_ei; ++b_it) {
			if(counted_pred.find(*b_it)!=counted_pred.end())
				continue;
			temp_list.enqueueBlock(*b_it);
			counted_pred.insert(*b_it);
		}
	}
	//block->dump();
#ifdef DEBUG_CFG
	cout<<"BLOCK: "<<block->getBlockID()<<endl;
	std::cout<<"gen set::"<<std::endl;
	LV->gen[block].print(M);
	std::cout<<"kill set::"<<std::endl;
	LV->kill[block].print(M);
#endif

	//initial defuse in/out set
    worklist.enqueueBlock(block);
		DefUseValues temp1, temp2;
		LV->blocksBeginToDefuse[block] = temp1;
		LV->blocksEndToDefuse[block] = temp2;
  }
  llvm::BitVector everAnalyzedinDefUse(cfg->getNumBlockIDs());
  //FIXME: Is reaching definition a forward analysis or backward analysis??
  worklist.sortWorklist();
  //worklist.reverseSortWorklist();

  while( const CFGBlock* block = worklist.dequeue()){
 	//stops when out set not changes
  	DefUseValues &prevVal = LV->blocksEndToDefuse[block];
	//val is in set
		DefUseValues val;
		for (CFGBlock::const_pred_iterator it = block->pred_begin(),
                                       ei = block->pred_end(); it != ei; ++it) {
      	if (const CFGBlock *pred = *it) {  
	  			//in = U_pred(out)
//        val = LV->merge(val, LV->blocksEndToDefUse[pred]);
					val.add_list(LV->blocksEndToDefuse[pred]);
      	}
    }
		LV->blocksBeginToDefuse[block] = val;
		if (!everAnalyzedinDefUse[block->getBlockID()])
      	everAnalyzedinDefUse[block->getBlockID()] = true;
    else if (prevVal.equals(val))
      	continue;

    prevVal = val;
		//update out set
		//out = gen U (in - kill)
		DefUseValues out_val;
		out_val = LV->blocksBeginToDefuse[block];
		out_val.add_list(LV->gen[block].getStmtList());
		out_val = out_val.subtract_list(LV->kill[block]);

		LV->blocksEndToDefuse[block] = out_val;
		// Enqueue the value to the successors.
    worklist.enqueueSuccessors(block);

  }

#ifdef DEBUG_CFG
	cout<<"**** FINAL block defuse ***"<<endl;
#endif
  for (CFG::const_reverse_iterator it = cfg->rbegin(), ei = cfg->rend(); it != ei; ++it) {
 	  const CFGBlock *block = *it;
		DefUseValues reaching_def = LV->blocksBeginToDefuse[block];
#ifdef DEBUG_CFG
		cout<<"CFGBLOCK "<<block->getBlockID()<<endl;
		printSourceRange(LV->blocks_range[block].getBegin(), LV->blocks_range[block].getEnd(), M);
		for(int k=0; k<LV->CFG_to_stmts[block].size(); k++)
			cout<<"\t [ "<<LV->CFG_to_stmts[block][k]->label<<" ]"<<endl;
#endif

		map<const VarDecl*, codeBlock*> inblock_defs;
		for(int i=0; i<LV->CFG_to_stmts[block].size(); i++){
				codeBlock* cb = LV->CFG_to_stmts[block][i];
#ifdef DEBUG_CFG
				cout<<"\t\t-\t code "<<cb->label<<endl;
#endif
				for(set<const VarDecl*>::iterator vit = cb->uses.begin(); vit != cb->uses.end(); vit++){
						set<codeBlock*> temp_set;
						cb->def_stmts[*vit] = temp_set;
						if(inblock_defs.find(*vit) != inblock_defs.end()){
#ifdef DEBUG_CFG
								cout<<"\t\t-\t-\tvariable "<<(*vit)->getNameAsString()<<" defined by (own block)"<<inblock_defs[*vit]->label<<", ";	
#endif
								if(inblock_defs[*vit] != cb)
										cb->def_stmts[*vit].insert(inblock_defs[*vit]);
						}
						else{
								for(unordered_set<codeBlock*>::iterator cit = reaching_def.stmts.begin(); cit != reaching_def.stmts.end(); cit++){
										if((*cit)->defs.find(*vit)!=(*cit)->defs.end()){
#ifdef DEBUG_CFG
												cout<<"\t\t-\t-\tvariable "<<(*vit)->getNameAsString()<<" defined by "<<(*cit)->label<<", ";
#endif
												cb->def_stmts[*vit].insert(*cit);
										}
								}
						}
						cout<<endl;
				}
				for(set<const VarDecl*>::iterator vit = cb->defs.begin(); vit != cb->defs.end(); vit++){
						inblock_defs[*vit] = cb;
				}
		}
		if(LV->CFG_to_conds.find(block)!=LV->CFG_to_conds.end()){
				for(int i=0; i<LV->CFG_to_conds[block].size(); i++){

						condBlock* condb = LV->CFG_to_conds[block][i];
#ifdef DEBUG_CFG
						if(condb->type == IF)cout<<"IF block:"<<endl;
						else if(condb->type == ELSE)cout<<"Else block:"<<endl;
						else if(condb->type == FOR)cout<<"For block:"<<endl;
#endif

						for(set<const VarDecl*>::iterator vit = condb->uses.begin(); vit != condb->uses.end(); vit++){
								if(condb->def_stmts.find(*vit) ==condb->def_stmts.end()){
										set<codeBlock*> temp_set;
										condb->def_stmts[*vit] = temp_set;
								}
								if(inblock_defs.find(*vit) != inblock_defs.end()){
#ifdef DEBUG_CFG
												cout<<"\t\tcondition var "<<(*vit)->getNameAsString()<<" defined by (own block)"<<inblock_defs[*vit]->label<<endl;	
#endif		
												condb->def_stmts[*vit].insert(inblock_defs[*vit]);
								}else{
										for(unordered_set<codeBlock*>::iterator cit = reaching_def.stmts.begin(); cit != reaching_def.stmts.end(); cit++){
												if((*cit)->defs.find(*vit) != (*cit)->defs.end()){
#ifdef DEBUG_CFG
														cout<<"\t\tcondition var "<<(*vit)->getNameAsString()<<" defined by "<<(*cit)->label<<endl;
#endif		
														condb->def_stmts[*vit].insert(*cit);
												}
										}
								}
						}
				}
		}
/*
	  BlockDepGraphNode node;
	  //compute def-use dependency chain

	  //check in set, find dependent blocks
	  DefUseValues defuse = LV->blocksBeginToDefuse[block];
	  std::queue<const Stmt*> que;
      for (CFGBlock::const_reverse_iterator bi = block->rbegin(), be = block->rend();
           bi != be; ++bi) {
		   if (Optional<CFGStmt> cs = bi->getAs<CFGStmt>()) {
		   		que.push(cs->getStmt());
		   }
	  }
	  std::unordered_set<const VarDecl*> uses;
	  //recursively compute use set
	  //block->dump();
	  LV->blockDepGraph[block] = node;
*/
  }
  return new myLiveVariables(LV);
}
void myLiveVariables::dumpBlockLiveness(const SourceManager &M) {
  getImpl(impl).dumpBlockLiveness(M);
}

void LiveVariablesImpl::dumpBlockLiveness(const SourceManager &M) {
  std::vector<const CFGBlock *> vec;
  for (llvm::DenseMap<const CFGBlock *, myLiveVariables::LivenessValues>::iterator
       it = blocksEndToLiveness.begin(), ei = blocksEndToLiveness.end();
       it != ei; ++it) {
    vec.push_back(it->first);    
  }
  std::sort(vec.begin(), vec.end(), [](const CFGBlock *A, const CFGBlock *B) {
    return A->getBlockID() < B->getBlockID();
  });

  std::vector<const VarDecl*> declVec;

  for (std::vector<const CFGBlock *>::iterator
        it = vec.begin(), ei = vec.end(); it != ei; ++it) {
    llvm::errs() << "\n[ B" << (*it)->getBlockID()
                 << " (live variables at block exit) ]\n";
//   (*it)->dump(); 
 //   myLiveVariables::LivenessValues vals = blocksEndToLiveness[*it];
   myLiveVariables::LivenessValues vals = blocksBeginToLiveness[*it];
   declVec.clear();
    
    for (llvm::ImmutableSet<const VarDecl *>::iterator si =
          vals.liveDecls.begin(),
          se = vals.liveDecls.end(); si != se; ++si) {
      declVec.push_back(*si);      
    }
 //   for (llvm::ImmutableSet<const Stmt *>::iterator si =
 //         vals.liveStmts.begin(),
 //         se = vals.liveStmts.end(); si != se; ++si) {
 //   	cout<<"LiveStmt dump: "<<std::endl;
 //   	//(*si)->dump();
 //     	const Stmt *st = *si;
 //       string sStart = st->getLocStart().printToString(M);
 //   	string sEnd = st->getLocEnd().printToString(M);
 //       cout<<"LiveStmt location: "<<sStart<<", to "<<sEnd<<std::endl;	
 //   }
//  	for (CFGBlock::const_reverse_iterator _it = (*it)->rbegin(),
//       ei = (*it)->rend(); _it != ei; ++_it) {
//    	const CFGElement &elem = *_it;
//		if (!elem.getAs<CFGStmt>())
//      		continue;
//		const Stmt *S = elem.castAs<CFGStmt>().getStmt();
//		string sStart = S->getLocStart().printToString(M);
//		string sEnd = S->getLocEnd().printToString(M);
//	    cout<<"----block stmt location: "<<sStart<<", to "<<sEnd<<std::endl;
//		myLiveVariables::LivenessValues _val = stmtsToLiveness[S];
//		for (llvm::ImmutableSet<const VarDecl *>::iterator si =
//          _val.liveDecls.begin(),
//          se = _val.liveDecls.end(); si != se; ++si) {
//		  std::cout<<"\t----in stmt, liveness var: "<<(*si)->getDeclName().getAsString()<<std::endl;
//		}
//	}



    std::sort(declVec.begin(), declVec.end(), [](const Decl *A, const Decl *B) {
      return A->getLocStart() < B->getLocStart();
    });

//    for (std::vector<const VarDecl*>::iterator di = declVec.begin(),
//         de = declVec.end(); di != de; ++di) {
//      llvm::errs() << " " << (*di)->getDeclName().getAsString()
//                   << " <";
//      (*di)->getLocation().dump(M);
//      llvm::errs() << ">\n";
//    }
  }
  llvm::errs() << "\n";  
}

const void *myLiveVariables::getTag() { static int x; return &x; }
const void *RelaxedMyLiveVariables::getTag() { static int x; return &x; }
