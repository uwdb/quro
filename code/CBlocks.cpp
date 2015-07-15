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
vector<queryBlock*> query_blocks;
vector<condBlock*> cond_blocks;

map<Stmt*, int> stmt_to_index;

stack<condBlock*> stack_cond_blocks;
vector<codeBlock*> code_blocks;

map<string, TABLE_DESC> tables;

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
									sprintf(c_index_str, "//conflict index = %d\n", this_conflict_index);
									q_block->source.assign(c_index_str);
									//q_block->source.append(rewriter.getRewrittenText(sprintf_sr));
									//rewriter.RemoveText(sprintf_sr);
									//q_block->source.append("\n");
									q_block->source.append(rewriter.getRewrittenText(qblock_sr));
							}
							else if(strcmp(funcName.c_str(), "sprintf")==0){
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
					else if(ReturnStmt* ret_stmt = dyn_cast<ReturnStmt>(st)){
							second_pass = false;
							stmt_processed = true;
							in_execute_function = false;
							third_pass = true;
							
							for(int i=0; i<code_blocks.size(); i++)
									code_blocks[i]->setDepBlocks();
							for(int i=0; i<code_blocks.size(); i++)
									code_blocks[i]->propagateDep();
							for(int i=0; i<code_blocks.size(); i++)
									code_blocks[i]->insert_code = code_blocks[i]->getBlockStr();
							
							SourceLocation insertPlace = processedCodeBeginPlace;
							SourceRange sr(insertPlace, ret_stmt->getLocStart().getLocWithOffset(-1));
							rewriter.RemoveText(sr);
							for(int i=code_blocks.size()-1; i>=0; i--){	
									codeBlock* c_block = code_blocks[i];
									if(c_block->processed)
											continue;
									rewriter.InsertTextBefore(insertPlace, c_block->insert_code);
							}
					}
					if(stmt_processed == false && processedCodeBeginPlace.isValid()){
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
			return ;
		}
		void setFirstStmtPlace(Stmt* stmt){
				if(processedCodeBeginPlace.isValid() == false){
						processedCodeBeginPlace = stmt->getLocStart();
				}
		}
};	

class myAnalysisConsumer : public ASTConsumer {
    private:
		myVisitor *visitor; // doesn't have to be private
		CompilerInstance *myCI;
	public:
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
	}/*
	virtual void HandleTranslationUnit(ASTContext &Context) {
		visitor->TraverseDecl(Context.getTranslationUnitDecl());
		DeclarationNameTable* declaNameTbl = new DeclarationNameTable(Context);
	}*/
	~myAnalysisConsumer() {}
//    void AddDiagnosticConsumer(PathDiagnosticConsumer* c){
//	}
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

		std::unique_ptr<ASTConsumer> p(new myAnalysisConsumer(&CI));
		return p;
			//hasModelPath ? new ModelInjector(CI) : nullptr);
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

