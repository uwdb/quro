#include "clang/AST/ASTConsumer.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Frontend/FixItRewriter.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Signals.h"
#include "llvm/ADT/STLExtras.h"
#include "QueryBlock.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/Lex/Lexer.h"
#include "ControlFlowGraph.h"
#include "AnalysisConsumer.h"

#include <fstream>
#include <iostream>
#include <string.h>
using namespace std;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

Rewriter rewriter;
clang::LangOptions languageOptions;

map<Stmt*, defuseSets> defuses;
stack<Stmt*> expr_stack;

CmpUnorderedSet<SourceRange> sr_uses;
CmpUnorderedSet<SourceRange> sr_defs;
vector<codeBlock*> code_blocks;
vector<condBlock*> cond_blocks;

map<Stmt*, int> stmt_to_index;

stack<condBlock*> stack_cond_blocks;

map<string, TABLE_DESC> tables;

vector<vector<string> > query_words_list;
vector<vector<string> > query_table_names;
vector<OPERATION> query_operations;
vector<vector<string> > constraints_info;


char OPERATION_str[4][8] = {"INSERT", "DELETE", "UPDATE", "SELECT"};

class myVisitor : public RecursiveASTVisitor<myVisitor> {
	private: ASTContext *astContext;
	queryBlock* cur_query_block;
	int query_cnt;
	SourceLocation last_source_begin;
	bool in_query;
public:
	bool in_execute_function;
	bool first_pass;
	bool second_pass;
	bool third_pass;
	int execute_func_count;
	SourceLocation processedCodeBeginPlace;
public:
		bool in_execute_func;
		explicit myVisitor(CompilerInstance *CI)
			: astContext(&(CI->getASTContext())) // initialize private members
		{
			rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());
			cur_query_block == NULL;
			in_execute_func = false;
			query_cnt = 0;
			in_query = false;
			in_execute_function = false;
			first_pass = true;
			second_pass = false;
			third_pass = false;
			execute_func_count = 0;
		}
		void myTraverseReturnStmt(Stmt* st){
				if(st == NULL)
						return ;
				if(CompoundStmt* cst = dyn_cast<CompoundStmt>(st)){
							for(CompoundStmt::const_body_iterator it = cst->body_begin(); it != cst->body_end(); it++){
									myTraverseReturnStmt(*it);
							}
				}
				else if(ReturnStmt* rst = dyn_cast<ReturnStmt>(st)){
							reorderCode(st);
				}
		}
		bool myTraverseStmt(Stmt* st){
				SourceManager& M = astContext->getSourceManager();
				Stmt* cur_stmt = st;
				if(st == NULL)
						return false;
				if(CompoundStmt* cst = dyn_cast<CompoundStmt>(cur_stmt)){
							for(CompoundStmt::const_body_iterator it = cst->body_begin(); it != cst->body_end(); it++){
									myTraverseStmt(*it);
							}
				}else if(ForStmt* fst = dyn_cast<ForStmt>(cur_stmt)){
							myVisitStmt(fst);
							myTraverseStmt(fst->getBody());
				}else if(WhileStmt* wst = dyn_cast<WhileStmt>(cur_stmt)){
							myVisitStmt(wst);
							myTraverseStmt(wst->getBody());
				}else if(IfStmt* ist = dyn_cast<IfStmt>(cur_stmt)){
							myVisitStmt(ist);
							VisitStmt(ist->getCond());
							myTraverseStmt(ist->getThen());
							if(ist->getElse()!=NULL)
									myTraverseStmt(ist->getElse());
				}else if(ReturnStmt* rst = dyn_cast<ReturnStmt>(cur_stmt)){
							myVisitStmt(rst);
				}else if(BinaryOperator* bop = dyn_cast<BinaryOperator>(cur_stmt)){
							myVisitStmt(bop);
							VisitStmt(bop->getLHS());
							VisitStmt(bop->getRHS());
				}else if(BlockExpr* bexpr = dyn_cast<BlockExpr>(cur_stmt)){
							myTraverseStmt(bexpr->getBody());
				}else if(CallExpr* callexpr = dyn_cast<CallExpr>(cur_stmt)){
							myVisitStmt(callexpr);
							VisitStmt(callexpr);
							unsigned int nArgs = callexpr->getNumArgs();
							for(int i=0; i<nArgs; i++){
								Expr* expr = callexpr->getArg(i);
								VisitStmt(expr);
							}
				}else if(UnaryOperator* uop = dyn_cast<UnaryOperator>(cur_stmt)){
							myVisitStmt(uop);
				}else if(CXXThrowExpr* top = dyn_cast<CXXThrowExpr>(cur_stmt)){
							myVisitStmt(top);
				}else if(Expr* expr = dyn_cast<Expr>(cur_stmt)){
				}else{
				}
				return true;
		}
		virtual bool VisitStmt(Stmt *st){
			SourceManager& M = astContext->getSourceManager();
			if(first_pass && in_execute_function){
			//==============Analyze Query String==============
					if(StringLiteral *ss = dyn_cast<StringLiteral>(st)){
							SourceLocation sl = ss->getLocStart();
							string lit_str = (ss->getString()).str();
							if(lit_str.find("fail") == std::string::npos && lit_str.find("FAIL") == std::string::npos){
									vector<string> words;
									splitByBlank(words, lit_str);
									bool is_query = false;
									int i=0; 
									vector<string> table_names;
									OPERATION op;
									//TODO: Following is simple SQL statement analysis. Should have sth. like SQL parser
									//TODO: multiple tables?
									if(lit_str.find("INSERT") != std::string::npos){
										  while(i<words.size() && strcmp(words[i].c_str(), "INTO")!=0)
													i++;
											assert(i+1<words.size());
											i++;
											char c[100]={0};
											int j=0;
											while(j<words[i].length() && words[i][j] != '(')
													c[j] = words[i][j], j++;
											string temp_str(c);
											//cur_query_block->table_name = c;
											table_names.push_back(c);
											op = INSERT;
											is_query = true;
									}else if(lit_str.find("SELECT") != std::string::npos){
											while(i<words.size() && strcmp(words[i].c_str(), "FROM")!=0)
													i++;
											assert(i+1<words.size());
											i++;
											//cur_query_block->table_name.assign(words[i]);
											while(i<words.size() && words[i] != "WHERE"){
												if(words[i].length()>0)
													table_names.push_back(words[i]);
												i++;
											}
											is_query = true;
											op = SELECT;
									}else if(lit_str.find("UPDATE") != std::string::npos){
											i=1;
											//cur_query_block->table_name.assign(words[i]);
											table_names.push_back(words[i]);
											is_query = true;
											op = UPDATE;
									}else if(lit_str.find("DELETE") != std::string::npos){
											while(i<words.size() && strcmp(words[i].c_str(), "FROM")!=0)
													i++;
											assert(i+1<words.size());
											i++;
											//cur_query_block->table_name.assign(words[i]);
											table_names.push_back(words[i]);
											is_query = true;
											op = DELETE;
									}
									string this_table_name;
									int this_conflict_index = 0;
									if(is_query){
										for(int k=0; k<table_names.size(); k++){
#ifdef DEBUG
											if(tables.find(table_names[k])==tables.end()){
													cout<<"table "<<table_names[k]<<" couldn't be found"<<endl;
													cout<<lit_str<<endl;
													for(int l=0; l<words.size(); l++)
														cout<<words[l]<<" * ";
													cout<<endl;
											}
#endif
											if(tables.find(table_names[k]) == tables.end())
													cout<<"ERROR: table "<<table_names[k]<<" not found"<<endl;
											assert(tables.find(table_names[k])!=tables.end());
											this_table_name = table_names[k];
											this_conflict_index = getConflictIndex(tables[table_names[k]].num_rows, op);
											stmt_to_index[st] = this_conflict_index;
/*
											if(this_conflict_index > cur_query_block->conflict_index){
													cur_query_block->conflict_index = this_conflict_index;
													cur_query_block->query_name.assign("QUERY_");
													cur_query_block->query_name.append(OPERATION_str[cur_query_block->op]);
													cur_query_block->query_name.append(table_names[k]);
													cur_query_block->table_name = table_names[k];
											}
*/
										}
#ifdef DEBUG
											cout<<"Is_query "<<is_query<<endl;
											cout<<"type = "<<op<<", table_name = "<<this_table_name<<", num_rows = "<<tables[this_table_name].num_rows<<", conflict_index = "<<this_conflict_index<<endl;
#endif
									}
							}
				}
//==============End Analyze Query String==========

					//if(Expr* expr = dyn_cast<Expr>(st)){
					Stmt* expr = st;
							if(defuses.find(expr)==defuses.end()){
									defuseSets temp_set;
									defuses[expr] = temp_set; 
							}
							analyzeStmts(expr, defuses[expr].uses, defuses[expr].defs, M);
#ifdef DEBUG
							cout<<"anayzeStmt:"<<endl;
							printSourceRange(expr->getLocStart(), expr->getLocEnd(), M);
#endif
							while((!expr_stack.empty()) && rangeIsWithin(expr->getLocStart(), expr->getLocEnd(), expr_stack.top()->getLocStart(), expr_stack.top()->getLocEnd(), M)==false ){
										Stmt* temp_st = expr_stack.top();
										expr_stack.pop();
										if(expr_stack.empty())break;
										if(rangeIsWithin(temp_st->getLocStart(), temp_st->getLocEnd(), expr_stack.top()->getLocStart(), expr_stack.top()->getLocEnd(), M))
												addSets(defuses[expr_stack.top()].uses, defuses[temp_st].uses);
												addSets(defuses[expr_stack.top()].defs, defuses[temp_st].defs);

							}
							if(!expr_stack.empty()){
										addSets(defuses[expr_stack.top()].uses, defuses[expr].uses);
										addSets(defuses[expr_stack.top()].defs, defuses[expr].defs);
							}
							expr_stack.push(expr);
					//}
					if(ReturnStmt* ret_stmt = dyn_cast<ReturnStmt>(st)){
							while(!expr_stack.empty()){
									Stmt* temp_st = expr_stack.top();
									expr_stack.pop();
									if(expr_stack.empty())break;
									if(rangeIsWithin(temp_st->getLocStart(), temp_st->getLocEnd(), expr_stack.top()->getLocStart(), expr_stack.top()->getLocEnd(), M))
											addSets(defuses[expr_stack.top()].uses, defuses[temp_st].uses);
											addSets(defuses[expr_stack.top()].defs, defuses[temp_st].defs);
							}
							first_pass = false;
							second_pass = true;
							in_execute_function = false;
#ifdef DEBUG
							for(map<Stmt*, defuseSets>::iterator it = defuses.begin(); it != defuses.end(); it++){
									string stmtType(getStmtType(it->first));
									if(it->second.uses.size()==0 && it->second.defs.size()==0)continue;
									cout<<stmtType<<": ";
									printSourceRange(it->first->getLocStart(), it->first->getLocEnd(), M);
									for(set<const VarDecl*>::iterator v_it = it->second.uses.begin(); v_it != it->second.uses.end(); v_it++){
												cout<<(*v_it)->getNameAsString()<<", ";
									}
									cout<<" // ";
 									for(set<const VarDecl*>::iterator v_it = it->second.defs.begin(); v_it != it->second.defs.end(); v_it++){
												cout<<(*v_it)->getNameAsString()<<", ";
									}
									cout<<endl;
							}
							cout<<"finish first pass!"<<endl;
#endif
					}
			}

			else if(second_pass && in_execute_function){
					CallExpr* CE = NULL;
					bool stmt_processed = false;
					BeforeThanCompare<SourceLocation> Compare(M);

					if(CallExpr* CE = dyn_cast<CallExpr>(st)){
							string funcName = (CE->getDirectCallee()->getNameInfo().getAsString());
							if(strcmp(funcName.c_str(), "dbt5_sql_execute")==0){
									execute_func_count++;
									stmt_processed = true;

									SourceLocation sr_begin = st->getLocStart();
									SourceLocation sr_end = st->getLocEnd();
									queryBlock* q_block = new queryBlock();
									q_block->defs = defuses[st].defs;
									q_block->uses = defuses[st].uses;
									Stmt* sprintf_stmt = NULL;
									int this_conflict_index = 0;

									for(map<Stmt*, defuseSets>::iterator it = defuses.begin(); it != defuses.end(); it++){
												Stmt* this_stmt = it->first;
												if(rangeIsWithin(CE->getLocStart(), CE->getLocEnd(), this_stmt->getLocStart(), this_stmt->getLocEnd(), M)==false)
															continue;
												if(IfStmt* ifstmt = dyn_cast<IfStmt>(this_stmt)){
														if(rangeIsWithin(CE->getLocStart(), CE->getLocEnd(), ifstmt->getCond()->getLocStart(), ifstmt->getCond()->getLocEnd(), M)){
																sr_begin = ifstmt->getLocStart();
																sr_end = ifstmt->getLocEnd();
																addSets(q_block->defs, it->second.defs);
																addSets(q_block->uses, it->second.uses);
														}
												}
												else if(ForStmt* forstmt = dyn_cast<ForStmt>(this_stmt)){
														//deprecated so far
												}
												else if(WhileStmt* whilestmt = dyn_cast<WhileStmt>(this_stmt)){
														if(rangeIsWithin(CE->getLocStart(), CE->getLocEnd(), whilestmt->getCond()->getLocStart(), whilestmt->getCond()->getLocEnd(), M)){
																sr_begin = whilestmt->getLocStart();
																sr_end = whilestmt->getLocEnd();
																addSets(q_block->defs, it->second.defs);
																addSets(q_block->uses, it->second.uses);
														}
												}
												else if(BinaryOperator* bo = dyn_cast<BinaryOperator>(this_stmt)){
															sr_begin = bo->getLocStart();
															sr_end = bo->getLocEnd().getLocWithOffset(1);
															addSets(q_block->defs, it->second.defs);
															addSets(q_block->uses, it->second.uses);
												}
									}
									//relate the query sprintf stmt
									for(map<Stmt*, defuseSets>::iterator it = defuses.begin(); it != defuses.end(); it++){
											if(stmt_to_index.find(it->first) != stmt_to_index.end()){
													if(crossCompare(it->second.defs, q_block->uses)){
															sprintf_stmt = it->first;
															this_conflict_index = stmt_to_index[sprintf_stmt];
															//addSets(q_block->uses, it->second.uses);
#ifdef DEBUG
															cout<<"query, conflict_index = "<<this_conflict_index<<endl;
#endif
													}
											}
									}
#ifdef DEBUG
									cout<<"defs: "<<endl;
									for(set<const VarDecl*>::iterator it = q_block->defs.begin(); it != q_block->defs.end(); it++){
											cout<<(*it)->getNameAsString()<<", ";
									}
									cout<<endl;
									cout<<"uses: "<<endl;
									for(set<const VarDecl*>::iterator it = q_block->uses.begin(); it != q_block->uses.end(); it++){
											cout<<(*it)->getNameAsString()<<", ";
									}
									cout<<endl;
									cout<<"RANGE: "<<endl;
									printSourceRange(sr_begin, sr_end, M);
#endif
									//Remove the stmts in the code_block range
									vector<codeBlock*>::iterator it = code_blocks.begin();
									while(it!=code_blocks.end()){
											if(rangeIsWithin((*it)->begin, (*it)->end, sr_begin, sr_end, M) /*|| (*it)->stmt == sprintf_stmt*/){
#ifdef DEBUG
													cout<<"remove code block:";
													printSourceRange((*it)->begin, (*it)->end, M);
#endif
													it = code_blocks.erase(it);
											}else{
													it++;
											}
									}
									q_block->index = code_blocks.size();
									code_blocks.push_back(q_block);
									q_block->begin = sr_begin;
									q_block->end = sr_end;
									char c_label[100] = {0};
									sprintf(c_label, "Q%d", execute_func_count);
									q_block->label.assign(c_label);
									stack<condBlock*> temp_sta;
									while((!stack_cond_blocks.empty())){
										condBlock* c_block = stack_cond_blocks.top();
										c_block = c_block->isWithin(q_block->begin, q_block->end, M);
										if(c_block){
#ifdef DEBUG
											cout<<"outer if block range: ";
											printSourceRange(c_block->begin, c_block->end, M);
#endif
											q_block->cond_blocks.push_back(c_block);
											c_block->stmt_list.push_back(q_block);
											temp_sta.push(c_block);
											stack_cond_blocks.pop();
										}else{
											//break;
											stack_cond_blocks.pop();
										}
									}
									while(!temp_sta.empty()){
											stack_cond_blocks.push(temp_sta.top());
											temp_sta.pop();
									}
									SourceRange qblock_sr(sr_begin, sr_end);
									assert(sprintf_stmt != NULL);
									//SourceRange sprintf_sr(sprintf_stmt->getLocStart(), sprintf_stmt->getLocEnd().getLocWithOffset(1));
									char c_index_str[100] = {0};
									q_block->conflict_index = this_conflict_index;
									sprintf(c_index_str, "//conflict index = %d\n", this_conflict_index);
									q_block->source.assign(c_index_str);
									//q_block->source.append(rewriter.getRewrittenText(sprintf_sr));
									//rewriter.RemoveText(sprintf_sr);
									//q_block->source.append("\n");
									q_block->source.append(rewriter.getRewrittenText(qblock_sr));
							}
							else if(strcmp(funcName.c_str(), "sprintf")==0){
#ifdef DEBUG_CFG
									cout<<"sprintf function: "<<endl;
									printSourceRange(st->getLocStart(), st->getLocEnd(), M);
#endif
									for(map<Stmt*, int>::iterator it = stmt_to_index.begin(); it != stmt_to_index.end(); it++){
											if(rangeIsWithin(it->first->getLocStart(), it->first->getLocEnd(), st->getLocStart(), st->getLocEnd(), M)){
													stmt_to_index[st] = it->second;
											}
									}
							}
					}
			}
			return true;
		}

		void myVisitStmt(Stmt* st){
			SourceManager& M = astContext->getSourceManager();
			BeforeThanCompare<SourceLocation> Compare(M);
			bool stmt_processed = false;
			if(second_pass && in_execute_function){
					if(IfStmt* if_stmt = dyn_cast<IfStmt>(st)){
					if(inAnyQuery(if_stmt->getLocStart(), if_stmt->getLocEnd(), M)==false){
							stmt_processed = true;
							setFirstStmtPlace(st);
									
							ifBlock* if_block = new ifBlock();
							if_block->begin = if_stmt->getLocStart();
							if_block->cond_begin = if_stmt->getCond()->getLocStart();
							if_block->cond_end = if_stmt->getCond()->getLocEnd();
							if_block->stmt = if_stmt;
							if_block->body = if_stmt->getThen();
							if_block->cond = if_stmt->getCond();
							cond_blocks.push_back(if_block);

							if_block->uses = defuses[if_stmt->getCond()].uses;

							//TODO: this should be a while loop: 
							//			if(){} else if(){} else{}
							if(if_stmt->getElse()){
									if_block->end = if_stmt->getElseLoc().getLocWithOffset(-1);
									elseBlock* else_block = new elseBlock();
									if_block->matched_else = else_block;
									else_block->matched_if = if_block;
									else_block->uses = if_block->uses;
									else_block->begin = if_block->end.getLocWithOffset(1);
									else_block->end = if_stmt->getLocEnd();
									else_block->stmt = if_stmt->getElse();
									else_block->cond = if_stmt->getCond();
									cond_blocks.push_back(else_block);

									SourceRange remove_sr(if_stmt->getElseLoc(), if_stmt->getElseLoc().getLocWithOffset(4));
									rewriter.RemoveText(remove_sr);
									SourceRange cond_sr(if_stmt->getCond()->getLocStart(), if_stmt->getCond()->getLocEnd());
									string cond_str = rewriter.getRewrittenText(cond_sr);
									if_block->cond_str = cond_str;
									char new_if_cond[200] = {0};
									sprintf(new_if_cond, "\n\nif(!(%s))", cond_str.c_str());
									string new_if_cond_str(new_if_cond);
									else_block->cond_str = cond_str;
									rewriter.InsertTextBefore(if_stmt->getElseLoc(), new_if_cond_str);
							}else{
									if_block->end = if_stmt->getLocEnd();
									SourceRange cond_sr(if_stmt->getCond()->getLocStart(), if_stmt->getCond()->getLocEnd());
									string cond_str = rewriter.getRewrittenText(cond_sr);
									if_block->cond_str = cond_str;
									if_block->matched_else = NULL;
							}
#ifdef DEBUG
							cout<<"## If stmt:(stacksize = "<<stack_cond_blocks.size()<<") range ";
							printSourceRange(if_block->begin, if_block->end, M);
							if(if_stmt->getElse()){
									cout<<"\t## else stmt: range ";
									printSourceRange(if_block->matched_else->begin, if_block->matched_else->end, M);
							}
#endif
							while((!stack_cond_blocks.empty()) && rangeIsWithin(if_stmt->getLocStart(), if_stmt->getLocEnd(), stack_cond_blocks.top()->instack_getStart(), stack_cond_blocks.top()->instack_getEnd(), M)==false ){
#ifdef DEBUG
									cout<<"Pop out if/for/while stmt:";
									printSourceRange(stack_cond_blocks.top()->instack_getStart(), stack_cond_blocks.top()->instack_getEnd(), M);
#endif
									stack_cond_blocks.pop();
							}
							stack_cond_blocks.push(if_block);
					}
					}
					else if(ForStmt* for_stmt = dyn_cast<ForStmt>(st)){

							if(inAnyQuery(for_stmt->getLocStart(), for_stmt->getLocEnd(), M) == false){
									stmt_processed = true;
									setFirstStmtPlace(st);

									forBlock* for_block = new forBlock();
									for_block->begin = for_stmt->getLocStart();
									for_block->end = for_stmt->getLocEnd();
									for_block->stmt = for_stmt;
									for_block->body = for_stmt->getBody();
									for_block->uses = defuses[for_stmt->getCond()].uses;
									for_block->cond = for_stmt->getCond();
									cond_blocks.push_back(for_block);									

									addSets(for_block->uses, defuses[for_stmt->getInit()].uses);

									SourceRange init_sr(for_stmt->getInit()->getLocStart(), for_stmt->getInit()->getLocEnd());
									for_block->init_str = rewriter.getRewrittenText(init_sr);
									SourceRange cond_sr(for_stmt->getCond()->getLocStart(), for_stmt->getCond()->getLocEnd());
									for_block->cond_str = rewriter.getRewrittenText(cond_sr);
									SourceRange inc_sr(for_stmt->getInc()->getLocStart(), for_stmt->getInc()->getLocEnd());
									for_block->inc_str = rewriter.getRewrittenText(inc_sr);
	
									while((!stack_cond_blocks.empty()) && rangeIsWithin(for_stmt->getLocStart(), for_stmt->getLocEnd(), stack_cond_blocks.top()->instack_getStart(), stack_cond_blocks.top()->instack_getEnd(), M)==false ){
											stack_cond_blocks.pop();
									}
									stack_cond_blocks.push(for_block);
#ifdef DEBUG
							cout<<"## For stmt: (stack_cond.size = "<<stack_cond_blocks.size()<<") range ";
							printSourceRange(for_block->begin, for_block->end, M);
#endif
							}
					}

					else if(WhileStmt* while_stmt = dyn_cast<WhileStmt>(st)){

							if(inAnyQuery(while_stmt->getLocStart(), while_stmt->getLocEnd(), M) == false){
									stmt_processed = true;
									setFirstStmtPlace(st);

									whileBlock* while_block = new whileBlock();
									while_block->begin = while_stmt->getLocStart();
									while_block->end = while_stmt->getLocEnd();
									while_block->stmt = while_stmt;
									while_block->body = while_stmt->getBody();
									while_block->uses = defuses[while_stmt->getCond()].uses;
									while_block->cond = while_stmt->getCond();
									cond_blocks.push_back(while_block);

									addSets(while_block->uses, defuses[while_stmt->getCond()].uses);

									SourceRange cond_sr(while_stmt->getCond()->getLocStart(), while_stmt->getCond()->getLocEnd());
									while_block->cond_str = rewriter.getRewrittenText(cond_sr);
	
									while((!stack_cond_blocks.empty()) && rangeIsWithin(while_stmt->getLocStart(), while_stmt->getLocEnd(), stack_cond_blocks.top()->instack_getStart(), stack_cond_blocks.top()->instack_getEnd(), M)==false ){
											stack_cond_blocks.pop();
									}
									stack_cond_blocks.push(while_block);
#ifdef DEBUG
							cout<<"## While stmt:(stack_cond.size = "<<stack_cond_blocks.size()<<") range ";
							printSourceRange(while_block->begin, while_block->end, M);
#endif
							}
					}
					if(stmt_processed == false /*&& processedCodeBeginPlace.isValid()*/){
#ifdef DEBUG
						cout<<"Process code:";
						printSourceRange(st->getLocStart(), st->getLocEnd(), M);
#endif
						if(inAnyQuery(st->getLocStart(), st->getLocEnd(), M) == false){
								nonqueryBlock* nonq_block = new nonqueryBlock();
								nonq_block->stmt = st;
								nonq_block->begin = st->getLocStart();
								nonq_block->end = st->getLocEnd();
								SourceRange nonqblock_sr(st->getLocStart(), st->getLocEnd());
								nonq_block->source = rewriter.getRewrittenText(nonqblock_sr);
								nonq_block->source.append(";");
								nonq_block->defs = defuses[st].defs;
								nonq_block->uses = defuses[st].uses;
								nonq_block->label = nonq_block->source;
#ifdef DEBUG
								cout<<"nonquery stmt block: (cond_stack.size = "<<stack_cond_blocks.size()<<") ";
								printSourceRange(st->getLocStart(), st->getLocEnd(), M);
								cout<<"source = "<<nonq_block->source<<endl;
								cout<<"defs: "<<endl;
								for(set<const VarDecl*>::iterator it = nonq_block->defs.begin(); it != nonq_block->defs.end(); it++){
										cout<<(*it)->getNameAsString()<<", ";
								}
								cout<<endl;
								cout<<"uses: "<<endl;
								for(set<const VarDecl*>::iterator it = nonq_block->uses.begin(); it != nonq_block->uses.end(); it++){
										cout<<(*it)->getNameAsString()<<", ";
								}
								cout<<endl;
#endif
								stack<condBlock*> temp_sta;
								while((!stack_cond_blocks.empty())){
									condBlock* c_block = stack_cond_blocks.top();
									c_block = c_block->isWithin(nonq_block->begin, nonq_block->end, M);
									if(c_block){
#ifdef DEBUG
										cout<<"outer if block range: ";
										printSourceRange(c_block->begin, c_block->end, M);
#endif
										nonq_block->cond_blocks.push_back(c_block);
										c_block->stmt_list.push_back(nonq_block);
										temp_sta.push(c_block);
										stack_cond_blocks.pop();
									}else{
										//break;
										stack_cond_blocks.pop();
									}
								}
								while(!temp_sta.empty()){
										stack_cond_blocks.push(temp_sta.top());
										temp_sta.pop();
								}
								nonq_block->index = code_blocks.size();
								code_blocks.push_back(nonq_block);
						}
					}
			}
			if(ReturnStmt* ret_stmt = dyn_cast<ReturnStmt>(st)){
					second_pass = false;
					third_pass = true;
					in_execute_function = false;
			}
			return ;
		}
		void reorderCode(Stmt* stmt){
			SourceManager& M = astContext->getSourceManager();
			BeforeThanCompare<SourceLocation> Compare(M);
			bool stmt_processed = false;
			if(third_pass && in_execute_function){
					if(ReturnStmt* ret_stmt = dyn_cast<ReturnStmt>(stmt)){

//							setControlFlow();

						
							for(int i=0; i<code_blocks.size(); i++)
									code_blocks[i]->setDepBlocks();
							for(int i=0; i<code_blocks.size(); i++)
									code_blocks[i]->propagateDep();
							for(int i=0; i<code_blocks.size(); i++)
									code_blocks[i]->insert_code = code_blocks[i]->getBlockStr();
							for(int i=0; i<code_blocks.size(); i++)
									code_blocks[i]->setFinalcBlock();
							for(int i=0; i<code_blocks.size(); i++)
									code_blocks[i]->finalizeDefs();
							SourceLocation insertPlace = processedCodeBeginPlace;
							SourceRange sr(insertPlace, ret_stmt->getLocStart().getLocWithOffset(-1));
							rewriter.RemoveText(sr);

							//Generate queryinfo.txt, for ILP solver

							//End Generate queryinfo.txt
							vector<codeBlock*> tempQ;
							for(int i=0; i<code_blocks.size(); i++){	
									codeBlock* c_block = code_blocks[i];
									if(c_block->processed == false){
											tempQ.push_back(c_block);
											for(int j=0; j<c_block->cond_blocks.size(); j++){
													addSets(c_block->uses, c_block->cond_blocks[j]->uses);
											}
									}
							}

#ifdef SORT_QUERIES
							vector<codeBlock*> tempQ2;
							for(int i=0; i<tempQ.size(); i++){
									tempQ2.push_back(tempQ[i]);
							}
							map<codeBlock*, set<codeBlock*> > map_parent;
							vector<codeBlock*> query_seq;
							vector<codeBlock*> temp_query_seq;
							for(int i=0; i<tempQ.size(); i++){
									if(tempQ[i]->type == QUERY){
											set<codeBlock*> new_set;
											temp_query_seq.push_back(tempQ[i]);
#ifdef DEBUG_CFG
											cout<<"query_code block "<<tempQ[i]->index<<"("<<tempQ[i]->label<<"): ";
#endif
											for(int j=0; j<i; j++){
													if(tempQ[i]->isDefinedBy(tempQ[j])){
															if(tempQ[j]->type == QUERY)
																	new_set.insert(tempQ[j]);
															else{
																	queue<codeBlock*> que;
																	que.push(tempQ[j]);
																	while(!que.empty()){
																			codeBlock* pre = que.front();
																			que.pop();
																			for(int k=0; k<tempQ.size(); k++){
																					if(tempQ[k] == pre)break;
																					if(pre->isDefinedBy(tempQ[k])){
																							que.push(tempQ[k]);
																							if(tempQ[k]->type == QUERY)
																									new_set.insert(tempQ[k]);
																					}
																			}	
																	}	
															}
													}
											}
											map_parent[tempQ[i]] = new_set;
#ifdef DEBUG_CFG
											cout<<"\t\tparents include :";
											for(set<codeBlock*>::iterator it = new_set.begin(); it != new_set.end(); it++)
														cout<<(*it)->index<<", ";
											cout<<endl;
#endif
									}
							}
							for(int i=0; i<tempQ.size(); i++){
									cout<<"----q"<<i<<"--------"<<endl;
									cout<<tempQ[i]->insert_code<<endl;
							}
#ifdef GENERATE_ILP_INPUT
							ofstream outfile("queryinfo.txt");
							int query_cnt = 0;
							map<codeBlock*, int> query_index;
							for(int i=0; i<tempQ.size(); i++){
									if(tempQ[i]->type == QUERY){
										query_index[tempQ[i]] = query_cnt;
										query_cnt++;
									}
							}
							outfile<<query_cnt<<endl;
							for(int i=0; i<tempQ.size(); i++){
									if(tempQ[i]->type == QUERY)
										outfile<<tempQ[i]->conflict_index<<" ";
							}
							outfile<<endl;
							for(int i=0; i<tempQ.size(); i++){
									if(tempQ[i]->type != QUERY)continue;
									for(set<codeBlock*>::iterator it = map_parent[tempQ[i]].begin(); it != map_parent[tempQ[i]].end(); it++){
											for(int k=0; k<i; k++)
													if(tempQ[k]->type == QUERY && tempQ[k] == *it)
															outfile<<query_index[tempQ[k]]<<" "<<query_index[tempQ[i]]<<endl;
									}
							}
#endif
#ifdef GENERATE_LARGE_ILP_INPUT
						ofstream outfile("queryinfo.txt");
						outfile<<tempQ.size()<<endl;
						set<condBlock*> cond_block_set;
						for(int i=0; i<tempQ.size(); i++){
								for(int j=0; j<tempQ[i]->cond_blocks.size(); j++)
									cond_block_set.insert(tempQ[i]->cond_blocks[j]);
						}
						outfile<<"#statements:"<<(cond_block_set.size()+tempQ.size())<<endl;
						for(int i=0; i<tempQ.size(); i++){
								if(tempQ[i]->type == QUERY)
									outfile<<tempQ[i]->conflict_index<<" ";
								else
									outfile<<"1 ";
						}
						outfile<<endl;
						for(int i=0; i<tempQ.size(); i++){
							set<codeBlock*> p_set;
							for(set<const VarDecl*>::iterator it = tempQ[i]->uses.begin(); it != tempQ[i]->uses.end(); it++){
									if(tempQ[i]->def_stmts.find(*it) != tempQ[i]->def_stmts.end()){
											for(set<codeBlock*>::iterator cit = tempQ[i]->def_stmts[*it].begin(); cit != tempQ[i]->def_stmts[*it].end(); cit++)
												p_set.insert((*cit)->finalcBlock);
									}
							}
							for(set<codeBlock*>::iterator it = p_set.begin(); it != p_set.end(); it++){
									for(int k=0; k<i; k++)
											if(tempQ[k] == *it)
												outfile<<k<<" "<<i<<endl;
							}
						}
						for(int i=0; i<tempQ.size(); i++){
							for(set<const VarDecl*>::iterator it = tempQ[i]->uses.begin(); it != tempQ[i]->uses.end(); it++){
								if(tempQ[i]->def_stmts.find(*it) != tempQ[i]->def_stmts.end()){
										int j = -1;
										for(set<codeBlock*>::iterator cit = tempQ[i]->def_stmts[*it].begin(); cit != tempQ[i]->def_stmts[*it].end(); cit++){
												j=-1;
												for(int x=0; x<tempQ.size(); x++)
													if((*cit)->finalcBlock == tempQ[x])
														j=x;
												if(j>=i)continue;
												if(j==-1){
														cout<<"block "<<tempQ[i]->index<<" parent "<<(*cit)->index<<endl;
												}
												assert(j!=-1);
												//i: use j's define, now find k
												for(int x=0; x<tempQ.size(); x++){
														if(x==i || x==j)continue;
														if(tempQ[x]->defs.find(*it) != tempQ[x]->defs.end()){
																outfile<<j<<" "<<i<<" "<<x<<endl;
														}
												}
										}
								}else{
										for(int j=i+1; j<tempQ.size(); j++){
												if(tempQ[j]->def_stmts.find(*it) != tempQ[j]->def_stmts.end())
															outfile<<i<<" "<<j<<endl; 
										}
								}
							}
						}
#endif
							//Code to sort queries; should be ILP
							sort(temp_query_seq.begin(), temp_query_seq.end(), qBlockCmp1);	
							//End sort
							set<codeBlock*> processed_blocks;
							while(processed_blocks.size() < temp_query_seq.size()){
									for(int i=0; i<temp_query_seq.size(); i++){
											codeBlock* cur_block = temp_query_seq[i];
											if(processed_blocks.find(cur_block) != processed_blocks.end())
														continue;
											//if empty_pqrents or parents are processed
#ifdef DEBUG_CFG
//											cout<<"---process block "<<cur_block->index<<"("<<cur_block->label<<"), parents.size = "<<map_parent[cur_block].size()<<endl;
#endif
											bool empty_parents = (map_parent[cur_block].size()==0);
											if(empty_parents == 0){
													bool no_parents = true;
													for(set<codeBlock*>::iterator cit = map_parent[cur_block].begin(); cit != map_parent[cur_block].end(); cit++){
															if(processed_blocks.find(*cit) == processed_blocks.end()){
																	no_parents = false;
																	break;
															}
													}
													empty_parents = no_parents;
											}
											if(empty_parents){
#ifdef DEBUG_CFG
//													cout<<"\t\t\tinsert "<<cur_block->index<<", c_index = "<<cur_block->conflict_index<<endl;
#endif
													processed_blocks.insert(cur_block);
													query_seq.push_back(cur_block);
													break;
											}
									}
							}
#ifdef DEBUG_CFG
							cout<<"After sort, query sequence: +++ "<<endl;
							for(int i=0; i<query_seq.size(); i++)
									cout<<"\t block "<<query_seq[i]->index<<endl;
#endif

							tempQ.clear();
							processed_blocks.clear();
							int q_index = 0;
							while(processed_blocks.size() < tempQ2.size()){
									if(q_index < query_seq.size()){
#ifdef DEBUG_CFG
											cout<<"Looking at block "<<query_seq[q_index]->index<<endl;
#endif
											codeBlock* cur_qblock = query_seq[q_index];
											q_index++;
											for(int i=0; i<tempQ2.size(); i++){
													if(tempQ2[i]->type == QUERY 
															|| cur_qblock->isDefinedBy(tempQ2[i]) == false 
															|| processed_blocks.find(tempQ2[i]) != processed_blocks.end())continue;
													if(tempQ2[i] == cur_qblock)break;
													//recursively insert parents into tempQ
													codeBlock* cb = tempQ2[i];
													queue<codeBlock*> que;
													que.push(cb);
													while(!que.empty()){
															cb = que.front();
															que.pop();
#ifdef DEBUG_CFG
															cout<<"\t\tpop out block "<<cb->index<<", parents: ";
#endif
															//no parents or processed
															bool no_parents = true;
															for(int k=0; k<i; k++){
																	if(cb->isDefinedBy(tempQ2[k]) && processed_blocks.find(tempQ2[k]) == processed_blocks.end()){
																			que.push(tempQ2[k]);
																			cout<<tempQ2[k]->index<<", ";
																			no_parents = false;
																	}
															}
															cout<<endl;
															if(no_parents){
#ifdef DEBUG_CFG
																	cout<<"\t\tinsert block"<<cb->index<<" into queue"<<endl;
#endif
																	processed_blocks.insert(cb);
																	tempQ.push_back(cb);
															}else{
																	que.push(cb);
															}
													}
											}
											for(int i=0; i<tempQ2.size(); i++){
													if(tempQ2[i] == cur_qblock)break;
													if(cur_qblock->isDefinedBy(tempQ2[i])){
															if(processed_blocks.find(tempQ2[i]) == processed_blocks.end())
																cout<<"\t\t\t--- block "<<tempQ2[i]->index<<" is not processed"<<endl;
															assert(processed_blocks.find(tempQ2[i]) != processed_blocks.end());
													}
											}	
											tempQ.push_back(cur_qblock);
											processed_blocks.insert(cur_qblock);
											for(int i=0; i<tempQ2.size(); i++){
													if(tempQ2[i]->type == QUERY || tempQ2[i]->isDefinedBy(cur_qblock) == false)continue;
													bool no_parents = true;
													for(int k=0; k<i; k++){
															if(tempQ2[i]->isDefinedBy(tempQ2[k]) && processed_blocks.find(tempQ2[k]) == processed_blocks.end())
																		no_parents = false; 
													}
													if(no_parents){
															tempQ.push_back(tempQ2[i]);
															processed_blocks.insert(tempQ2[i]);
													}
											}
									}else{
											for(int i=0; i<tempQ2.size(); i++){
													if(processed_blocks.find(tempQ2[i]) != processed_blocks.end())continue;
													codeBlock* cb = tempQ2[i];
													queue<codeBlock*> que;
													que.push(cb);
													while(!que.empty()){
															cb = que.front();
															que.pop();
#ifdef DEBUG_CFG
															cout<<"\t\tpop out block "<<cb->index<<", parents: ";
#endif
															//no parents or processed
															bool no_parents = true;
															for(int k=0; k<i; k++){
																	if(cb->isDefinedBy(tempQ2[k]) && processed_blocks.find(tempQ2[k]) == processed_blocks.end()){
																			que.push(tempQ2[k]);
																			cout<<tempQ2[k]->index<<", ";
																			no_parents = false;
																	}
															}
															cout<<endl;
															if(no_parents){
#ifdef DEBUG_CFG
																	cout<<"\t\tinsert block"<<cb->index<<" into queue"<<endl;
#endif
																	processed_blocks.insert(cb);
																	tempQ.push_back(cb);
															}else{
																	que.push(cb);
															}
													}

											}
									}
							}
							
#endif

//#ifdef REORDER_BUFFER
#ifdef DEBUG_DEPRECATED
							for(int i=0; i<code_blocks.size(); i++){
									if(code_blocks[i]->processed == true)
											continue;
									for(map<const VarDecl*, set<codeBlock*> >::iterator it  = code_blocks[i]->def_stmts.begin(); it != code_blocks[i]->def_stmts.end(); it++){
											cout<<"Var "<<it->first->getNameAsString()<<" in block "<<code_blocks[i]->label<<endl;
											cout<<"\t\tdefined by ";
											for(set<codeBlock*>::iterator cit = it->second.begin(); cit != it->second.end(); cit++)
													cout<<(*cit)->label<<"("<<(*cit)->finalcBlock->label<<"), ";
											cout<<endl;
									}
							}

							for(int i=0; i<code_blocks.size(); i++){
									cout<<"\tcode "<<code_blocks[i]->pre_label<<"("<<code_blocks[i]->processed<<")'s finalized code block is "<<code_blocks[i]->finalcBlock->pre_label<<" ("<<code_blocks[i]->finalcBlock->processed<<endl;

							}

							cout<<endl<<"#######     #######        ##########"<<endl;
							for(int i=0; i<tempQ.size(); i++){
									cout<<"block "<<tempQ[i]->index<<" content: "<<endl;
									cout<<"          $$$$$ "<<endl;
									cout<<tempQ[i]->insert_code<<endl;
									cout<<"          $$$$$ "<<endl;
									cout<<"  defines: ";
									for(set<const VarDecl*>::iterator it = tempQ[i]->defs.begin(); it != tempQ[i]->defs.end(); it++)
											cout<<(*it)->getNameAsString()<<", ";
									cout<<endl<<" uses: ";
									for(set<const VarDecl*>::iterator it = tempQ[i]->uses.begin(); it != tempQ[i]->uses.end(); it++)
											cout<<(*it)->getNameAsString()<<", ";
									cout<<endl;

									for(map<const VarDecl*, set<codeBlock*> >::iterator it  =  tempQ[i]->def_stmts.begin(); it != tempQ[i]->def_stmts.end(); it++){
											cout<<"Var "<<it->first->getNameAsString()<<" ";
											cout<<"\t\tdefined by ";
											for(set<codeBlock*>::iterator cit = it->second.begin(); cit != it->second.end(); cit++)
													cout<<(*cit)->finalcBlock->index<<", ";
											cout<<endl;
									}

							}

#endif

							//code_blocks.clear();
							vector<reorderBufferUnit> reorderBuffer;
							renameTable renameT;
							int rnb_cnt = 0;
#ifdef SORT_QUERIES
							vector<map<const VarDecl*, int> > last_use;
							for(int i=0; i<tempQ.size(); i++){
									map<const VarDecl*, int> new_map;
									for(set<const VarDecl*>::iterator it = tempQ[i]->defs.begin(); it != tempQ[i]->defs.end(); it++){
											if(new_map.find(*it) != new_map.end())continue;
											for(int j=tempQ.size()-1; j>i; j--){
													if(tempQ[j]->def_stmts[*it].find(tempQ[i]) != tempQ[j]->def_stmts[*it].end()){
															new_map[*it] = j;
													}
											}
									}
									last_use.push_back(new_map);
							}
							for(int i=0; i<tempQ.size(); i++){
									map<const VarDecl*, string> changed_names;

									for(set<const VarDecl*>::iterator it = tempQ[i]->uses.begin(); it != tempQ[i]->uses.end(); it++){
											codeBlock* def_cb = NULL;
											for(set<codeBlock*>::iterator cit = tempQ[i]->def_stmts[*it].begin(); cit != tempQ[i]->def_stmts[*it].end(); cit++){
													if((*cit)->finalcBlock != tempQ[i])
															def_cb = (*cit)->finalcBlock;
											}
											if(def_cb != NULL && renameT.table.find(def_cb) != renameT.table.end()){
													if(renameT.table[def_cb].names.find(*it) != renameT.table[def_cb].names.end()){
#ifdef DEBUG_CFG
																cout<<"block "<<tempQ[i]->index<<" uses "<<(*it)->getNameAsString()<<", rename to "<<renameT.table[def_cb].names[*it]<<endl;
#endif
																changed_names[*it] = renameT.table[def_cb].names[*it];
																//TODO: if the name is defined by multiple statements, insert a new assign into after these defining stmts.
																//tempQ[i]->insert_code = replaceString(tempQ[i]->insert_code, (*it)->getNameAsString(), renameT.table[def_cb].names[*it]); 
													}
											}
									}
									for(set<const VarDecl*>::iterator it = tempQ[i]->defs.begin(); it != tempQ[i]->defs.end(); it++){
											string new_name = (*it)->getNameAsString();
											bool need_rename = false;
											char n_str[10] = {0};
											for(int j=0; j<i; j++){
													if(last_use[j].find(*it) != last_use[j].end() && 
															last_use[j][*it] > i){
																need_rename = true;
																sprintf(n_str, "_%d", rnb_cnt);
																rnb_cnt++;

													}
											}
											if(need_rename){
													new_name.append(n_str);
													if(renameT.table.find(tempQ[i]) == renameT.table.end()){
															cb_renames newrenames;
															newrenames.names[*it] = new_name;	
															renameT.table[tempQ[i]] = newrenames;
													}else{
															renameT.table[tempQ[i]].names[(*it)] = new_name;
													}
#ifdef DEBUG_CFG
													cout<<"block "<<tempQ[i]->index<<" rename var "<<(*it)->getNameAsString()<<" to "<<new_name<<endl;
#endif
													if(changed_names.find(*it) != changed_names.end()){
															//this var is both used and defined in this block (assuming used first).
															//In case they are defined and used in a same function call... 
															//Add an assign: result32 = result29;
															string added_assign = generateAssignment(*it, changed_names[*it], new_name);
															changed_names[*it] = new_name;
															tempQ[i]->insert_code.insert(0, added_assign);
													}else{
															changed_names[*it] = new_name;
													}
													//tempQ[i]->insert_code = replaceString(tempQ[i]->insert_code, (*it)->getNameAsString(), new_name);	
											}
									}
									for(map<const VarDecl*, string>::iterator vit = changed_names.begin(); vit != changed_names.end(); vit++){
											tempQ[i]->insert_code = replaceString(tempQ[i]->insert_code, (vit->first)->getNameAsString(), vit->second);
									}
							}

							for(int i=tempQ.size()-1; i>=0; i--){	
									codeBlock* c_block = tempQ[i];
									rewriter.InsertTextBefore(insertPlace, c_block->insert_code);
							}

#endif /* SORT_QUERIES */


#ifdef REORDER_BUFFER
							for(int i=0; i<tempQ.size(); i++){
									//code_blocks.push_back(tempQ[i]);
									reorderBufferUnit ru;
									map<const VarDecl*, string> changed_names;
									ru.cb = tempQ[i];
									if(tempQ[i]->conflict_index == 0)
											ru.exec_cycles = 1;
									else
											ru.exec_cycles = tempQ[i]->conflict_index;
									ru.cycle_dispatched = i+1;

									//check and determine the dispatch cycle
									for(set<const VarDecl*>::iterator it = tempQ[i]->uses.begin(); it != tempQ[i]->uses.end(); it++){
											for(int j=reorderBuffer.size()-1; j>=0; j--){
													//if(reorderBuffer[j].cb->defs.find(*it) != reorderBuffer[j].cb->defs.end()){
													if(tempQ[i]->isDefinedBy(reorderBuffer[j].cb)){
																ru.cycle_dispatched = max(ru.cycle_dispatched, reorderBuffer[j].cycle_finish);
																//break;
													}	
											}
											codeBlock* def_cb = NULL;
											for(set<codeBlock*>::iterator cit = tempQ[i]->def_stmts[*it].begin(); cit != tempQ[i]->def_stmts[*it].end(); cit++){
													if((*cit)->finalcBlock != tempQ[i])
															def_cb = (*cit)->finalcBlock;
											}
											if(def_cb != NULL && renameT.table.find(def_cb) != renameT.table.end()){
													if(renameT.table[def_cb].names.find(*it) != renameT.table[def_cb].names.end()){
#ifdef DEBUG_CFG
																cout<<"block "<<tempQ[i]->index<<" uses "<<(*it)->getNameAsString()<<", rename to "<<renameT.table[def_cb].names[*it]<<endl;
#endif
																changed_names[*it] = renameT.table[def_cb].names[*it];
																//TODO: if the name is defined by multiple statements, insert a new assign into after these defining stmts.
																//tempQ[i]->insert_code = replaceString(tempQ[i]->insert_code, (*it)->getNameAsString(), renameT.table[def_cb].names[*it]); 
													}
											}
									}

									for(set<const VarDecl*>::iterator it = tempQ[i]->defs.begin(); it != tempQ[i]->defs.end(); it++){
											string new_name = (*it)->getNameAsString();
											bool need_rename = false;
											char n_str[10] = {0};
											for(int j=reorderBuffer.size()-1; j>=0; j--){
													if(reorderBuffer[j].cb->uses.find(*it) != reorderBuffer[j].cb->uses.end() && ru.cycle_dispatched < reorderBuffer[j].cycle_finish){
																//Insert an item into the reorder buffer
																sprintf(n_str, "_%d", rnb_cnt);
																rnb_cnt++;
																need_rename = true;
																//ru.cycle_dispatched = max(ru.cycle_dispatched, reorderBuffer[j].cycle_finish);
																//break;
													}	
											}
											if(need_rename){
													new_name.append(n_str);
													if(renameT.table.find(tempQ[i]) == renameT.table.end()){
															cb_renames newrenames;
															newrenames.names[*it] = new_name;	
															renameT.table[tempQ[i]] = newrenames;
													}else{
															renameT.table[tempQ[i]].names[(*it)] = new_name;
													}
#ifdef DEBUG_CFG
													cout<<"block "<<tempQ[i]->index<<" rename var "<<(*it)->getNameAsString()<<" to "<<new_name<<endl;
#endif
													if(changed_names.find(*it) != changed_names.end()){
															//this var is both used and defined in this block (assuming used first).
															string added_assign = generateAssignment(*it, changed_names[*it], new_name);
															changed_names[*it] = new_name;
															tempQ[i]->insert_code.insert(0, added_assign);
													}else{
															changed_names[*it] = new_name;
													}
													//tempQ[i]->insert_code = replaceString(tempQ[i]->insert_code, (*it)->getNameAsString(), new_name);	
											}
									}
									for(map<const VarDecl*, string>::iterator vit = changed_names.begin(); vit != changed_names.end(); vit++){
												tempQ[i]->insert_code = replaceString(tempQ[i]->insert_code, (vit->first)->getNameAsString(), vit->second);
									}

									ru.cycle_finish = ru.exec_cycles + ru.cycle_dispatched;
#ifdef DEBUG_DEPRECATED
									cout<<"$$ code block "<<tempQ[i]->label<<" ("<<tempQ[i]->index<<") $$:"<<endl;
									cout<<"defs: "<<endl;
									for(set<const VarDecl*>::iterator it = tempQ[i]->defs.begin(); it != tempQ[i]->defs.end(); it++){
											cout<<(*it)->getNameAsString()<<", ";
									}
									cout<<endl;
									cout<<"uses: "<<endl;
									for(set<const VarDecl*>::iterator it = tempQ[i]->uses.begin(); it != tempQ[i]->uses.end(); it++){
											cout<<(*it)->getNameAsString()<<", ";
									}
									cout<<endl;
									cout<<"\tcycle_dispatched = "<<ru.cycle_dispatched<<", cycle_finished = "<<ru.cycle_finish<<endl;
#endif
									reorderBuffer.push_back(ru);
							}
							sort(reorderBuffer.begin(), reorderBuffer.end(), reorderBufCmp);

//Insert code block into the original source code file
							for(int i=reorderBuffer.size()-1; i>=0; i--)
									rewriter.InsertTextBefore(insertPlace, reorderBuffer[i].cb->insert_code);
#endif


//#endif /* REORDER_BUFFER */

#ifdef ONLY_FISSION
							for(int i=code_blocks.size()-1; i>=0; i--){	
									codeBlock* c_block = code_blocks[i];
									if(c_block->processed)
											continue;
									rewriter.InsertTextBefore(insertPlace, c_block->insert_code);
							}
#endif
					}
				}
		}
		void setFirstStmtPlace(Stmt* stmt){
				if(processedCodeBeginPlace.isValid() == false){
						processedCodeBeginPlace = stmt->getLocStart();
				}
		}
/*
		void recursiveSort(int i, bool* processed, vector<queryBlock*>& temp_qblocks){
			for(set<queryBlock*>::iterator it = query_blocks[i]->children.begin();
										it != query_blocks[i]->children.end(); it++){
					if(processed[(*it)->sort_index] == false){
							recursiveSort((*it)->sort_index, processed, temp_qblocks);
					}
			}
#ifdef DEBUG
					cout<<"--- push back "<<query_blocks[i]->query_name<<", children.size = "<<query_blocks[i]->children.size()<<endl;
#endif
					processed[i] = true;
					temp_qblocks.push_back(query_blocks[i]);

		}
*/
};	

class myAnalysisConsumer : public AnalysisConsumer {
    private:
		myVisitor *visitor; // doesn't have to be private
		CompilerInstance *myCI;
public:
		// override the constructor in order to pass CI
		explicit myAnalysisConsumer(CompilerInstance *CI,
								const Preprocessor& pp,
                   				const std::string& outdir,
                   				AnalyzerOptionsRef opts,
                   				ArrayRef<std::string> plugins)
								//CodeInjector *injector)
					: AnalysisConsumer(pp, outdir, opts, plugins),
					myCI(CI),
					visitor(new myVisitor(CI))  // initialize the visitor
				{ }

	bool HandleTopLevelDecl(DeclGroupRef DG) {
//		storeTopLevelDecls(DG);
		// a DeclGroupRef may have multiple Decls, so we iterate through each one
		for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; i++) {
			Decl *D = *i;
			if(FunctionDecl* FD = dyn_cast<FunctionDecl>(D)){	
				string func_name = FD->getNameInfo().getAsString();
				if(strcmp(func_name.c_str(), "execute")==0)
				{
						visitor->in_execute_function = true;
						visitor->TraverseDecl(D); // recursively visit each AST node in Decl "D"
						visitor->in_execute_function = true;
						visitor->myTraverseStmt(FD->getBody());

						AnalysisDeclContext* adCtxt = Mgr->getAnalysisDeclContext(D);
						myLiveVariables *liveV = myLiveVariables::create(*adCtxt, myCI->getASTContext().getSourceManager());
						if(liveV==nullptr){
						}else{
						//liveV->dumpBlockLiveness(myCI->getASTContext().getSourceManager());
						}

						visitor->in_execute_function = true;
						visitor->myTraverseReturnStmt(FD->getBody());

				}
			}
		}

		return true;
	}
	virtual void HandleTranslationUnit(ASTContext &Context) {
			/* we can use ASTContext to get the TranslationUnitDecl, which is
			a single Decl that collectively represents the entire source file */
		visitor->TraverseDecl(Context.getTranslationUnitDecl());
		DeclarationNameTable* declaNameTbl = new DeclarationNameTable(Context);
	}
	~myAnalysisConsumer() {}
//    void AddDiagnosticConsumer(PathDiagnosticConsumer* c){
//	}
/*
	public:
()){
																		explicit myAnalysisConsumer(CompilerInstance *CI)
						: visitor(new myVisitor(CI)) // initialize the visitor
				{ }
	bool HandleTopLevelDecl(DeclGroupRef DG) {
//		storeTopLevelDecls(DG);
		// a DeclGroupRef may have multiple Decls, so we iterate through each one
		for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; i++) {
			Decl *D = *i;
			if(FunctionDecl* FD = dyn_cast<FunctionDecl>(D)){	
				string func_name = FD->getNameInfo().getAsString();
				if(strcmp(func_name.c_str(), "execute")==0)
				{
						visitor->in_execute_function = true;
						visitor->TraverseDecl(D); // recursively visit each AST node in Decl "D"
						visitor->in_execute_function = true;
						visitor->myTraverseStmt(FD->getBody());
				}
			}
		}
		return true;
	}
	~myAnalysisConsumer() {}
*/
};

class myAnalysisAction : public ASTFrontendAction {
	protected:
	std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) {
		//std::unique_ptr<ASTConsumer> p(new myASTConsumer(&CI));
		//return p;
	  	LangOptions& langOp = CI.getLangOpts();
		langOp.CPlusPlus = 1;

		//CI.getPreprocessor().getDiagnostics().setWarningsAsErrors(false);

 		AnalyzerOptionsRef analyzerOpts = CI.getAnalyzerOpts();
 // 		bool hasModelPath = analyzerOpts->Config.count("model-path") > 0;

//		std::unique_ptr<ASTConsumer> p(new myAnalysisConsumer(&CI));
//		return p;
			//hasModelPath ? new ModelInjector(CI) : nullptr);
		return llvm::make_unique<myAnalysisConsumer>(&CI, 
      		CI.getPreprocessor(), CI.getFrontendOpts().OutputFile, analyzerOpts,
      		CI.getFrontendOpts().Plugins);

	}
};

class ClangCheckActionFactory {
public:
  std::unique_ptr<clang::ASTConsumer> newASTConsumer() {
    return llvm::make_unique<clang::ASTConsumer>();
  }
};

static cl::OptionCategory MyToolCategory("My tool options");

int main(int argc, const char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal();
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

 
	readConflictInfo();

  FrontendActionFactory *fea = newFrontendActionFactory<myAnalysisAction>().release(); 
	
		int result = Tool.run(fea);
		
		rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
		// print out the rewritten source code ("rewriter" is a global var.)
		return result;

}

