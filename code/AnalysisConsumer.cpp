#include "AnalysisConsumer.h"
#include <iostream>
#include <string.h>
using namespace std;
//===--- AnalysisConsumer.cpp - ASTConsumer for running Analyses ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// "Meta" ASTConsumer for running different source analyses.
//
//===----------------------------------------------------------------------===//


//
//STATISTIC(NumFunctionTopLevel, "The # of functions at top level.");
//STATISTIC(NumFunctionsAnalyzed,
//                      "The # of functions and blocks analyzed (as top level "
//                      "with inlining turned on).");
//STATISTIC(NumBlocksInAnalyzedFunctions,
//                      "The # of basic blocks in the analyzed functions.");
//STATISTIC(PercentReachableBlocks, "The % of reachable basic blocks.");
//STATISTIC(MaxCFGSize, "The maximum number of basic blocks in a function.");
uint64_t NumFunctionTopLevel = 0;
uint64_t NumFunctionsAnalyzed = 0;
uint64_t NumBlockInAnalyzedFunctions = 0;
uint64_t PercentReachableBlocks = 0;
uint64_t MaxCFGSize = 0;
//===----------------------------------------------------------------------===//
// Special PathDiagnosticConsumers.
//===----------------------------------------------------------------------===//

//void ento::createPlistHTMLDiagnosticConsumer(AnalyzerOptions &AnalyzerOpts,
//                                             PathDiagnosticConsumers &C,
//                                             const std::string &prefix,
//                                             const Preprocessor &PP) {
//  createHTMLDiagnosticConsumer(AnalyzerOpts, C,
//                               llvm::sys::path::parent_path(prefix), PP);
//  createPlistDiagnosticConsumer(AnalyzerOpts, C, prefix, PP);
//}
//
//void ento::createTextPathDiagnosticConsumer(AnalyzerOptions &AnalyzerOpts,
//                                            PathDiagnosticConsumers &C,
//                                            const std::string &Prefix,
//                                            const clang::Preprocessor &PP) {
//  llvm_unreachable("'text' consumer should be enabled on ClangDiags");
//}
//
//class ClangDiagPathDiagConsumer : public PathDiagnosticConsumer {
//  DiagnosticsEngine &Diag;
//  bool IncludePath;
//public:
//  ClangDiagPathDiagConsumer(DiagnosticsEngine &Diag)
//    : Diag(Diag), IncludePath(false) {}
//  virtual ~ClangDiagPathDiagConsumer() {}
//  StringRef getName() const override { return "ClangDiags"; }
//
//  bool supportsLogicalOpControlFlow() const override { return true; }
//  bool supportsCrossFileDiagnostics() const override { return true; }
//
//  PathGenerationScheme getGenerationScheme() const override {
//    return IncludePath ? Minimal : None;
//  }
//
//  void enablePaths() {
//    IncludePath = true;
//  }
//
//  void FlushDiagnosticsImpl(std::vector<const PathDiagnostic *> &Diags,
//                            FilesMade *filesMade) override {
//    unsigned WarnID = Diag.getCustomDiagID(DiagnosticsEngine::Warning, "%0");
//    unsigned NoteID = Diag.getCustomDiagID(DiagnosticsEngine::Note, "%0");
//
//    for (std::vector<const PathDiagnostic*>::iterator I = Diags.begin(),
//         E = Diags.end(); I != E; ++I) {
//      const PathDiagnostic *PD = *I;
//      SourceLocation WarnLoc = PD->getLocation().asLocation();
//      Diag.Report(WarnLoc, WarnID) << PD->getShortDescription()
//                                   << PD->path.back()->getRanges();
//
//      if (!IncludePath)
//        continue;
//
//      PathPieces FlatPath = PD->path.flatten(/*ShouldFlattenMacros=*/true);
//      for (PathPieces::const_iterator PI = FlatPath.begin(),
//                                      PE = FlatPath.end();
//           PI != PE; ++PI) {
//        SourceLocation NoteLoc = (*PI)->getLocation().asLocation();
//        Diag.Report(NoteLoc, NoteID) << (*PI)->getString()
//                                     << (*PI)->getRanges();
//      }
//    }
//  }
//};



//===----------------------------------------------------------------------===//
// AnalysisConsumer implementation.
//===----------------------------------------------------------------------===//
llvm::Timer* AnalysisConsumer::TUTotalTimer = nullptr;

bool AnalysisConsumer::HandleTopLevelDecl(DeclGroupRef DG) {
 storeTopLevelDecls(DG);
 for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; i++) {
		Decl *D = *i;
		string ssStart = D->getLocStart().printToString(Mgr->getASTContext().getSourceManager());
		//std::cout<<"HandleTopLevelDecl: "<<ssStart<<std::endl;
 
  }
  return true;
}

void AnalysisConsumer::HandleTopLevelDeclInObjCContainer(DeclGroupRef DG) {
  storeTopLevelDecls(DG);
}

void AnalysisConsumer::storeTopLevelDecls(DeclGroupRef DG) {
  for (DeclGroupRef::iterator I = DG.begin(), E = DG.end(); I != E; ++I) {

    // Skip ObjCMethodDecl, wait for the objc container to avoid
    // analyzing twice.
    if (isa<ObjCMethodDecl>(*I))
      continue;

    LocalTUDecls.push_back(*I);
  }
}

static bool shouldSkipFunction(const Decl *D,
                               const SetOfConstDecls &Visited,
                               const SetOfConstDecls &VisitedAsTopLevel) {
  if (VisitedAsTopLevel.count(D))
    return true;

  // We want to re-analyse the functions as top level in the following cases:
  // - The 'init' methods should be reanalyzed because
  //   ObjCNonNilReturnValueChecker assumes that '[super init]' never returns
  //   'nil' and unless we analyze the 'init' functions as top level, we will
  //   not catch errors within defensive code.
  // - We want to reanalyze all ObjC methods as top level to report Retain
  //   Count naming convention errors more aggressively.
  if (isa<ObjCMethodDecl>(D))
    return false;

  // Otherwise, if we visited the function before, do not reanalyze it.
  return Visited.count(D);
}

//ExprEngine::InliningModes
//AnalysisConsumer::getInliningModeForFunction(const Decl *D,
//                                             const SetOfConstDecls &Visited) {
//  // We want to reanalyze all ObjC methods as top level to report Retain
//  // Count naming convention errors more aggressively. But we should tune down
//  // inlining when reanalyzing an already inlined function.
//  if (Visited.count(D)) {
//    assert(isa<ObjCMethodDecl>(D) &&
//           "We are only reanalyzing ObjCMethods.");
//    const ObjCMethodDecl *ObjCM = cast<ObjCMethodDecl>(D);
//    if (ObjCM->getMethodFamily() != OMF_init)
//      return ExprEngine::Inline_Minimal;
//  }
//
//  return ExprEngine::Inline_Regular;
//}

void AnalysisConsumer::HandleDeclsCallGraph(const unsigned LocalTUDeclsSize) {
  // Build the Call Graph by adding all the top level declarations to the graph.
  // Note: CallGraph can trigger deserialization of more items from a pch
  // (though HandleInterestingDecl); triggering additions to LocalTUDecls.
  // We rely on random access to add the initially processed Decls to CG.
  CallGraph CG;
  for (unsigned i = 0 ; i < LocalTUDeclsSize ; ++i) {
    CG.addToCallGraph(LocalTUDecls[i]);
  }

  // Walk over all of the call graph nodes in topological order, so that we
  // analyze parents before the children. Skip the functions inlined into
  // the previously processed functions. Use external Visited set to identify
  // inlined functions. The topological order allows the "do not reanalyze
  // previously inlined function" performance heuristic to be triggered more
  // often.
  SetOfConstDecls Visited;
  SetOfConstDecls VisitedAsTopLevel;
  llvm::ReversePostOrderTraversal<clang::CallGraph*> RPOT(&CG);
  for (llvm::ReversePostOrderTraversal<clang::CallGraph*>::rpo_iterator
         I = RPOT.begin(), E = RPOT.end(); I != E; ++I) {
    NumFunctionTopLevel++;
	
    CallGraphNode *N = *I;
    Decl *D = N->getDecl();
	string ssStart = D->getLocStart().printToString(Mgr->getASTContext().getSourceManager());

    // Skip the abstract root node.
    if (!D)
      continue;

    // Skip the functions which have been processed already or previously
    // inlined.
    if (shouldSkipFunction(D, Visited, VisitedAsTopLevel))
      continue;

    // Analyze the function.
    SetOfConstDecls VisitedCallees;

    HandleCode(D, AM_Path, 
               (Mgr->options.InliningMode == All ? nullptr : &VisitedCallees));

    // Add the visited callees to the global visited set.
    for (SetOfConstDecls::iterator I = VisitedCallees.begin(),
                                   E = VisitedCallees.end(); I != E; ++I) {
        Visited.insert(*I);
    }
    VisitedAsTopLevel.insert(D);
  }
}

void AnalysisConsumer::HandleTranslationUnit(ASTContext &C) {
  // Don't run the actions if an error has occurred with parsing the file.
  DiagnosticsEngine &Diags = PP.getDiagnostics();
  if (Diags.hasErrorOccurred() || Diags.hasFatalErrorOccurred())
    return;

  // Don't analyze if the user explicitly asked for no checks to be performed
  // on this file.
  if (Opts->DisableAllChecks)
    return;

  {
    if (TUTotalTimer) TUTotalTimer->startTimer();

    // Introduce a scope to destroy BR before Mgr.
//    BugReporter BR(*Mgr);
    TranslationUnitDecl *TU = C.getTranslationUnitDecl();
//    checkerMgr->runCheckersOnASTDecl(TU, *Mgr, BR);

    // Run the AST-only checks using the order in which functions are defined.
    // If inlining is not turned on, use the simplest function order for path
    // sensitive analyzes as well.
    RecVisitorMode = AM_Syntax;
//    if (!Mgr->shouldInlineCall())
//      RecVisitorMode |= AM_Path;
//    RecVisitorBR = &BR;

    // Process all the top level declarations.
    //
    // Note: TraverseDecl may modify LocalTUDecls, but only by appending more
    // entries.  Thus we don't use an iterator, but rely on LocalTUDecls
    // random access.  By doing so, we automatically compensate for iterators
    // possibly being invalidated, although this is a bit slower.
    const unsigned LocalTUDeclsSize = LocalTUDecls.size();
    for (unsigned i = 0 ; i < LocalTUDeclsSize ; ++i) {
      TraverseDecl(LocalTUDecls[i]);
    }

  //  if (Mgr->shouldInlineCall())
  //    HandleDeclsCallGraph(LocalTUDeclsSize);

    // After all decls handled, run checkers on the entire TranslationUnit.
//    checkerMgr->runCheckersOnEndOfTranslationUnit(TU, *Mgr, BR);

    RecVisitorBR = nullptr;
  }

  // Explicitly destroy the PathDiagnosticConsumer.  This will flush its output.
  // FIXME: This should be replaced with something that doesn't rely on
  // side-effects in PathDiagnosticConsumer's destructor. This is required when
  // used with option -disable-free.
  Mgr.reset();

  if (TUTotalTimer) TUTotalTimer->stopTimer();

  // Count how many basic blocks we have not covered.
//  NumBlocksInAnalyzedFunctions = FunctionSummaries.getTotalNumBasicBlocks();
//  if (NumBlocksInAnalyzedFunctions > 0)
//    PercentReachableBlocks =
//      (FunctionSummaries.getTotalNumVisitedBasicBlocks() * 100) /
//        NumBlocksInAnalyzedFunctions;

}

static std::string getFunctionName(const Decl *D) {
  if (const ObjCMethodDecl *ID = dyn_cast<ObjCMethodDecl>(D)) {
    return ID->getSelector().getAsString();
  }
  if (const FunctionDecl *ND = dyn_cast<FunctionDecl>(D)) {
    IdentifierInfo *II = ND->getIdentifier();
    if (II)
      return II->getName();
  }
  return "";
}

AnalysisConsumer::AnalysisMode
AnalysisConsumer::getModeForDecl(Decl *D, AnalysisMode Mode) {
  if (!Opts->AnalyzeSpecificFunction.empty() &&
      getFunctionName(D) != Opts->AnalyzeSpecificFunction)
    return AM_None;

  // Unless -analyze-all is specified, treat decls differently depending on
  // where they came from:
  // - Main source file: run both path-sensitive and non-path-sensitive checks.
  // - Header files: run non-path-sensitive checks only.
  // - System headers: don't run any checks.
  SourceManager &SM = Ctx->getSourceManager();
  SourceLocation SL = SM.getExpansionLoc(D->getLocation());
  if (!Opts->AnalyzeAll && !SM.isInMainFile(SL)) {
    if (SL.isInvalid() || SM.isInSystemHeader(SL))
      return AM_None;
    return Mode & ~AM_Path;
  }

  return Mode;
}

void AnalysisConsumer::HandleCode(Decl *D, AnalysisMode Mode,
                                  SetOfConstDecls *VisitedCallees) {
  if (!D->hasBody())
    return;
  Mode = getModeForDecl(D, Mode);
  if (Mode == AM_None)
    return;
//  DisplayFunction(D, Mode, IMode);
  CFG *DeclCFG = Mgr->getCFG(D);
  if (DeclCFG) {
    unsigned CFGSize = DeclCFG->size();
    MaxCFGSize = MaxCFGSize < CFGSize ? CFGSize : MaxCFGSize;
  }

  // Clear the AnalysisManager of old AnalysisDeclContexts.
  Mgr->ClearContexts();
  myLiveVariables* L = /*Mgr->getAnalysis<LiveVariables>(D)*/myLiveVariables::create(*(Mgr->getAnalysisDeclContext(D)), Mgr->getASTContext().getSourceManager());
  L->dumpBlockLiveness(Mgr->getASTContext().getSourceManager());
//  BugReporter BR(*Mgr);

//  if (Mode & AM_Syntax)
//    checkerMgr->runCheckersOnASTBody(D, *Mgr, BR);
//  if ((Mode & AM_Path) && checkerMgr->hasPathSensitiveCheckers()) {
//    RunPathSensitiveChecks(D, IMode, VisitedCallees);
//    if (IMode != ExprEngine::Inline_Minimal)
//      NumFunctionsAnalyzed++;
//  }
}

//===----------------------------------------------------------------------===//
// Path-sensitive checking.
//===----------------------------------------------------------------------===//

//void AnalysisConsumer::ActionExprEngine(Decl *D, bool ObjCGCEnabled,
//                                        ExprEngine::InliningModes IMode,
//                                        SetOfConstDecls *VisitedCallees) {
//  // Construct the analysis engine.  First check if the CFG is valid.
//  // FIXME: Inter-procedural analysis will need to handle invalid CFGs.
//  if (!Mgr->getCFG(D))
//    return;
//
//  // See if the LiveVariables analysis scales.
//  if (!Mgr->getAnalysisDeclContext(D)->getAnalysis<RelaxedLiveVariables>())
//    return;
//
//  ExprEngine Eng(*Mgr, ObjCGCEnabled, VisitedCallees, &FunctionSummaries,IMode);
//
//  // Set the graph auditor.
//  std::unique_ptr<ExplodedNode::Auditor> Auditor;
//  if (Mgr->options.visualizeExplodedGraphWithUbiGraph) {
//    Auditor = CreateUbiViz();
//    ExplodedNode::SetAuditor(Auditor.get());
//  }
//
//  // Execute the worklist algorithm.
//  Eng.ExecuteWorkList(Mgr->getAnalysisDeclContextManager().getStackFrame(D),
//                      Mgr->options.getMaxNodesPerTopLevelFunction());
//
//  // Release the auditor (if any) so that it doesn't monitor the graph
//  // created BugReporter.
//  ExplodedNode::SetAuditor(nullptr);
//
//  // Visualize the exploded graph.
//  if (Mgr->options.visualizeExplodedGraphWithGraphViz)
//    Eng.ViewGraph(Mgr->options.TrimGraph);
//
//  // Display warnings.
//  Eng.getBugReporter().FlushReports();
//}

//void AnalysisConsumer::RunPathSensitiveChecks(Decl *D,
//                                              SetOfConstDecls *Visited) {
//
//  switch (Mgr->getLangOpts().getGC()) {
//  case LangOptions::NonGC:
//    ActionExprEngine(D, false, IMode, Visited);
//    break;
//  
//  case LangOptions::GCOnly:
//    ActionExprEngine(D, true, IMode, Visited);
//    break;
//  
//  case LangOptions::HybridGC:
//    ActionExprEngine(D, false, IMode, Visited);
//    ActionExprEngine(D, true, IMode, Visited);
//    break;
//  }
//}
