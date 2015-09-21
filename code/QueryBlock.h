#ifndef QUERY_BLOCK_H
#define QUERY_BLOCK_H
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
#include "clang/AST/DeclCXX.h"

//#define DEBUG
//#define DEBUG_CFG
//#define ONLY_FISSION
#define SORT_QUERIES
#define GENERATE_ILP_INPUT
//#define GENERATE_LARGE_ILP_INPUT

#include <vector>
#include <set>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <fstream>

using namespace std;
using namespace clang;

class queryBlock;
enum CONDTYPE{IF, ELSEIF, ELSE, FOR, WHILE};
enum CODETYPE{QUERY, NONQUERY};
enum OPERATION{INSERT, DELETE, UPDATE, SELECT};

class defuseSets{
	public:
		set<const VarDecl*> defs;
		set<const VarDecl*> uses;
};
template<class T>
class CmpUnorderedSet {
	vector<T> list;
	vector<bool> attr;
public:
	T* end(){
		return NULL;
	}
	T* find(T t){
		for(int i=0; i<list.size(); i++){
				if(list[i] == t)
						return &(list[i]);
		}
		return NULL;
	}
	void insert(T t){
		if(find(t) == end()){
			list.push_back(t);
			attr.push_back(false);
		}
	}
	void insert(T t, bool _attr){
		if(find(t) == end()){
			list.push_back(t);
			attr.push_back(_attr);
		}
	}
	bool getattr(T t){
		for(int i=0; i<list.size(); i++){
				if(list[i] == t)
						return attr[i];
		}
		return false;
	}
};

extern CmpUnorderedSet<SourceRange> sr_uses;
extern CmpUnorderedSet<SourceRange> sr_defs;

class codeBlock;

class condBlock{
public:
		SourceLocation begin;
		SourceLocation end;
		SourceLocation cond_begin;
		SourceLocation cond_end;
		SourceLocation instack_getStart();
		SourceLocation instack_getEnd();
		Stmt* stmt;
		Stmt* body;
		Stmt* cond;
		string cond_str;
		CONDTYPE type;
		const VarDecl* isDefinedBy(codeBlock* cb);
		set<const VarDecl*> uses;
		map<const VarDecl*, set<codeBlock*> > def_stmts;
		vector<queryBlock*> queries;
		vector<codeBlock*> stmt_list;
		bool checkDep(set<const VarDecl*>& defs, set<const VarDecl*>& uses);
		bool inCond(SourceLocation loc, SourceManager& M);
		virtual condBlock* isWithin(SourceLocation loc, SourceManager& M) = 0;
		virtual bool checkQueryDep(queryBlock* q1, queryBlock* q2) = 0;
		virtual condBlock* isWithin(SourceLocation loc_begin, SourceLocation loc_end, SourceManager& M) = 0;
};


class codeBlock{
public:
	SourceLocation begin;
	SourceLocation end;
	Stmt* stmt;
	vector<condBlock*> cond_blocks;
	set<codeBlock*> dep_blocks;
	set<const VarDecl*> defs;
	set<const VarDecl*> uses;
	map<const VarDecl*, set<codeBlock*>> def_stmts;
	codeBlock* finalcBlock;
	string pre_label;
#ifdef EXACT_USEDEF
	map<const VarDecl*, set<codeBlock*>> real_uses;
	set<codeBlock*> predecessor;
	set<codeBlock*> successor;
	map<const VarDecl*, set<codeBlock*> > children;
#endif
	string source;
	CODETYPE type;
	bool processed;
	string insert_code;

	int index;
	string label;
	codeBlock();	
	int crossCompareAllConds(condBlock* most_inner_cond, int c_block);
	const VarDecl* isDefinedBy(codeBlock* cb);
	string getBlockStr();
	void setDepBlocks();
	void setFinalcBlock();
	void propagateDep();
	void finalizeDefs();
	int conflict_index;
	int sort_index;
	int sort_base;


	string recursiveGenerate(int i);
};

class nonqueryBlock : public codeBlock{
public:
	nonqueryBlock();
}; 

class queryBlock : public codeBlock{
public:
	queryBlock();
};
class elseBlock;

class forBlock : public condBlock{
public:
		forBlock();
		string inc_str;
		string init_str;
		condBlock* isWithin(SourceLocation loc, SourceManager& M);
		condBlock* isWithin(SourceLocation loc_begin, SourceLocation loc_end, SourceManager& M);
		bool checkQueryDep(queryBlock* q1, queryBlock* q2);
};

class whileBlock : public condBlock{
public:
		whileBlock();
		condBlock* isWithin(SourceLocation loc, SourceManager& M);
		condBlock* isWithin(SourceLocation loc_begin, SourceLocation loc_end, SourceManager& M);
		bool checkQueryDep(queryBlock* q1, queryBlock* q2);
};
class ifBlock : public condBlock{
public:
		ifBlock();
		elseBlock* matched_else;
		condBlock* isWithin(SourceLocation loc, SourceManager& M);
		condBlock* isWithin(SourceLocation loc_begin, SourceLocation loc_end, SourceManager& M);
		bool checkQueryDep(queryBlock* q1, queryBlock* q2);
};

class elseBlock : public condBlock{
public:
		elseBlock();
		ifBlock* matched_if;
		condBlock* isWithin(SourceLocation loc, SourceManager& M);
		condBlock* isWithin(SourceLocation loc_begin, SourceLocation loc_end, SourceManager& M);
		bool checkQueryDep(queryBlock* q1, queryBlock* q2);
};

extern stack<condBlock*> stack_cond_blocks;

string parseLabel(const char* label_name);
void printSourceRange(const SourceLocation& begin, const SourceLocation& end, SourceManager& M);

bool rangeIsWithin(SourceLocation inner_begin, SourceLocation inner_end,  SourceLocation outer_begin, SourceLocation outer_end, SourceManager& M);
bool rangeIsWithin(SourceLocation inner, SourceLocation outer_begin, SourceLocation outer_end, SourceManager& M);

void addSets(set<const VarDecl*>& dest, set<const VarDecl*>& src);
void addSets(map<const VarDecl*, set<codeBlock*> >& dest, map<const VarDecl*, set<codeBlock*> >& src);

const VarDecl* crossCompare(set<const VarDecl*>& s1, set<const VarDecl*>& s2);
bool rangeCompletelyBefore(SourceLocation before_begin, SourceLocation before_end,  SourceLocation after_begin, SourceLocation after_end, SourceManager& M);
void analyzeStmts(Stmt* st, set<const VarDecl*>& uses, set<const VarDecl*>& defs, SourceManager& M);
const char* getStmtType(Stmt* st);
#ifdef EXACT_USEDEF
void setControlFlow();
#endif

condBlock* inAnyCond(SourceLocation loc, SourceManager& M);
bool inAnyQuery(SourceLocation begin, SourceLocation end, SourceManager& M);

extern vector<queryBlock*> query_blocks;
extern vector<codeBlock*> code_blocks;
extern vector<condBlock*> cond_blocks;

bool qBlockCmp1(codeBlock* q1, codeBlock* q2);
bool qBlockCmp2(codeBlock* q1, codeBlock* q2);

struct TABLE_DESC{
	string name;
//	vector<COLUMN_DESC> columns;
	int num_rows;
};

struct reorderBufferUnit{
	codeBlock* cb;
	int cycle_dispatched;
	int cycle_finish;
	int exec_cycles;
};
struct cb_renames{
	map<const VarDecl*, string> names;
};
struct renameTable{
	map<codeBlock*, cb_renames> table;
};
string replaceString(string src, string word, string new_word);
string generateAssignment(const VarDecl* var, string before, string end);

bool reorderBufCmp(reorderBufferUnit r1, reorderBufferUnit r2);

extern vector<queryBlock*> query_blocks;
void splitByBlank(vector<string>& vec, string str);
int getConflictIndex(int row_num, OPERATION op);
void setDependencies();
extern map<string, TABLE_DESC> tables;
void readConflictInfo();

#endif
