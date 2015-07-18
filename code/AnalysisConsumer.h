#include "clang/StaticAnalyzer/Frontend/AnalysisConsumer.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DataRecursiveASTVisitor.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/ParentMap.h"
#include "clang/Analysis/CodeInjector.h"
//#include "clang/Analysis/Analyses/LiveVariables.h"
#include "clang/Analysis/CFG.h"
#include "clang/Analysis/CallGraph.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/StaticAnalyzer/Checkers/LocalCheckers.h"
#include "clang/StaticAnalyzer/Core/AnalyzerOptions.h"
//#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/PathDiagnostic.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathDiagnosticConsumers.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
//#include "clang/StaticAnalyzer/Frontend/CheckerRegistration.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"

#include "ControlFlowGraph.h"
#include <memory>
#include <queue>

using namespace clang;
using namespace ento;
using llvm::SmallPtrSet;

std::unique_ptr<CheckerManager>
createCheckerManager(AnalyzerOptions &opts, 
					const LangOptions &langOpts,
                    ArrayRef<std::string> plugins,
                    DiagnosticsEngine &diags); 

class AnalysisConsumer : public AnalysisASTConsumer,
                         public DataRecursiveASTVisitor<AnalysisConsumer> {
  enum {
    AM_None = 0,
    AM_Syntax = 0x1,
    AM_Path = 0x2
  };
  typedef unsigned AnalysisMode;

  /// Mode of the analyzes while recursively visiting Decls.
  AnalysisMode RecVisitorMode;
  /// Bug Reporter to use while recursively visiting Decls.
  BugReporter *RecVisitorBR;

public:
  ASTContext *Ctx;
  const Preprocessor &PP;
  const std::string OutDir;
  AnalyzerOptionsRef Opts;
  ArrayRef<std::string> Plugins;
  CodeInjector *Injector;

  /// \brief Stores the declarations from the local translation unit.
  /// Note, we pre-compute the local declarations at parse time as an
  /// optimization to make sure we do not deserialize everything from disk.
  /// The local declaration to all declarations ratio might be very small when
  /// working with a PCH file.
  SetOfDecls LocalTUDecls;
                           
  // Set of PathDiagnosticConsumers.  Owned by AnalysisManager.
  PathDiagnosticConsumers PathConsumers;

  StoreManagerCreator CreateStoreMgr;
  ConstraintManagerCreator CreateConstraintMgr;

//  std::unique_ptr<CheckerManager> checkerMgr;
  std::unique_ptr<AnalysisManager> Mgr;

  /// Time the analyzes time of each translation unit.
  static llvm::Timer* TUTotalTimer;

  /// The information about analyzed functions shared throughout the
  /// translation unit.
  FunctionSummariesTy FunctionSummaries;

  AnalysisConsumer(const Preprocessor& pp,
                   const std::string& outdir,
                   AnalyzerOptionsRef opts,
                   ArrayRef<std::string> plugins)
    : RecVisitorMode(0), RecVisitorBR(nullptr), Ctx(nullptr), PP(pp),
      OutDir(outdir), Opts(opts), Plugins(plugins) {
//    DigestAnalyzerOptions();
//    if (Opts->PrintStats) {
//      llvm::EnableStatistics();
//      TUTotalTimer = new llvm::Timer("Analyzer Total Time");
//    }
  }

  ~AnalysisConsumer() {
//    if (Opts->PrintStats)
//      delete TUTotalTimer;
  }


  void Initialize(ASTContext &Context) override {
    Ctx = &Context;
//    checkerMgr = createCheckerManager(*Opts, PP.getLangOpts(), Plugins,
//                                      PP.getDiagnostics());

    Mgr = llvm::make_unique<AnalysisManager>(
        *Ctx, PP.getDiagnostics(), PP.getLangOpts(), PathConsumers,
        CreateStoreMgr, CreateConstraintMgr, /*checkerMgr.get(),*/nullptr, *Opts, Injector);
  }

  /// \brief Store the top level decls in the set to be processed later on.
  /// (Doing this pre-processing avoids deserialization of data from PCH.)
  bool HandleTopLevelDecl(DeclGroupRef D) override;
  void HandleTopLevelDeclInObjCContainer(DeclGroupRef D) override;

  void HandleTranslationUnit(ASTContext &C) override;

  /// \brief Determine which inlining mode should be used when this function is
  /// analyzed. This allows to redefine the default inlining policies when
  /// analyzing a given function.

  /// \brief Build the call graph for all the top level decls of this TU and
  /// use it to define the order in which the functions should be visited.
  void HandleDeclsCallGraph(const unsigned LocalTUDeclsSize);

  /// \brief Run analyzes(syntax or path sensitive) on the given function.
  /// \param Mode - determines if we are requesting syntax only or path
  /// sensitive only analysis.
  /// \param VisitedCallees - The output parameter, which is populated with the
  /// set of functions which should be considered analyzed after analyzing the
  /// given root function.
  void HandleCode(Decl *D, AnalysisMode Mode,
                  SetOfConstDecls *VisitedCallees = nullptr);

  //void RunPathSensitiveChecks(Decl *D,
  //                            ExprEngine::InliningModes IMode,
  //                            SetOfConstDecls *VisitedCallees);
//  void ActionExprEngine(Decl *D, bool ObjCGCEnabled,
//                        ExprEngine::InliningModes IMode,
//                        SetOfConstDecls *VisitedCallees);
//
  /// Visitors for the RecursiveASTVisitor.
  bool shouldWalkTypesOfTypeLocs() const { return false; }

  /// Handle callbacks for arbitrary Decls.
  bool VisitDecl(Decl *D) {
    AnalysisMode Mode = getModeForDecl(D, RecVisitorMode);
//    if (Mode & AM_Syntax)
//      checkerMgr->runCheckersOnASTDecl(D, *Mgr, *RecVisitorBR);
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    IdentifierInfo *II = FD->getIdentifier();
    if (II && II->getName().startswith("__inline"))
      return true;

    // We skip function template definitions, as their semantics is
    // only determined when they are instantiated.
    if (FD->isThisDeclarationADefinition() &&
        !FD->isDependentContext()) {
//      assert(RecVisitorMode == AM_Syntax || Mgr->shouldInlineCall() == false);
      HandleCode(FD, RecVisitorMode);
    }
    return true;
  }

  bool VisitObjCMethodDecl(ObjCMethodDecl *MD) {
    if (MD->isThisDeclarationADefinition()) {
//      assert(RecVisitorMode == AM_Syntax || Mgr->shouldInlineCall() == false);
      HandleCode(MD, RecVisitorMode);
    }
    return true;
  }
  
  bool VisitBlockDecl(BlockDecl *BD) {
    if (BD->hasBody()) {
//      assert(RecVisitorMode == AM_Syntax || Mgr->shouldInlineCall() == false);
      HandleCode(BD, RecVisitorMode);
    }
    return true;
  }

  virtual void
  AddDiagnosticConsumer(PathDiagnosticConsumer *Consumer) override {
    PathConsumers.push_back(Consumer);
  }

private:
  void storeTopLevelDecls(DeclGroupRef DG);

  /// \brief Check if we should skip (not analyze) the given function.
  AnalysisMode getModeForDecl(Decl *D, AnalysisMode Mode);

};



