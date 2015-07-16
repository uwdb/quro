#include "QueryBlock.h"

string parseLabel(const char* label_name){
		int len = strlen(label_name);
		int i=0;
		string label;
    if(strstr(label_name, "QUERY_BEGIN")!=NULL){
				label.assign(label_name+11);
		}else if(strstr(label_name, "QUERY_END")!=NULL){
				label.assign(label_name+9);
		}
		return label;
}

void printSourceRange(const SourceLocation& begin, const SourceLocation& end, SourceManager& M){
	if(begin.isValid() && end.isValid())
		cout<<begin.printToString(M)<<" - "<<end.printToString(M)<<endl;
	else
		cout<<"Invalid loc"<<endl;
}

bool condBlock::inCond(SourceLocation loc, SourceManager& M){
		BeforeThanCompare<SourceLocation> Compare(M);
		if(loc.isValid() && cond_begin.isValid() && cond_end.isValid()){
				if(Compare(loc, cond_begin) || Compare(cond_end, loc)){
				}else{
						return true;
				}
		}
		return false;
}
condBlock* inAnyCond(SourceLocation loc, SourceManager& M){
	for(int i=cond_blocks.size()-1; i>=0; i--){
			if(cond_blocks[i]->inCond(loc, M))
					return cond_blocks[i];
	}
	return NULL;
}

bool rangeIsWithin(SourceLocation inner_begin, SourceLocation inner_end,  SourceLocation outer_begin, SourceLocation outer_end, SourceManager& M){
		BeforeThanCompare<SourceLocation> Compare(M);
		if(inner_begin.isValid() && inner_end.isValid() && outer_begin.isValid() && outer_end.isValid()){
				if(Compare(inner_begin, outer_begin) ||
						Compare(outer_end, inner_end) ){
						return false;
				}else{
						return true;
				}
		}
		return false;
}

bool rangeCompletelyBefore(SourceLocation before_begin, SourceLocation before_end,  SourceLocation after_begin, SourceLocation after_end, SourceManager& M){
		BeforeThanCompare<SourceLocation> Compare(M);
		if(before_begin.isValid() && before_end.isValid() && after_begin.isValid() && after_end.isValid()){
				if(Compare(before_begin, after_begin) &&
						Compare(before_end, after_begin) ){
						return true;
				}else{
						return false;
				}
		}
		return false;
}

const VarDecl* crossCompare(set<const VarDecl*>& s1, set<const VarDecl*>& s2){
		for(set<const VarDecl*>::iterator it1 = s1.begin(); it1 != s1.end(); it1++){
				for(set<const VarDecl*>::iterator it2 = s2.begin(); it2 != s2.end(); it2++){
						if(*it1 == *it2){
#ifdef DEBUG
								cout<<"crossCompare: "<<(*it1)->getNameAsString()<<endl;
#endif
								return (*it1);
						}
				}
		}
		return NULL;
}


void addSets(set<const VarDecl*>& dest, set<const VarDecl*>& src){
		for(set<const VarDecl*>::iterator it = src.begin(); it != src.end(); it++){
				dest.insert(*it);
		}
}

void analyzeStmts(Stmt* st, set<const VarDecl*>& uses, set<const VarDecl*>& defs, SourceManager& M){
		if(DeclRefExpr *decl = dyn_cast<DeclRefExpr>(st)){
				SourceRange sr(st->getLocStart(), st->getLocEnd());
				if(sr_uses.find(sr) != sr_uses.end()){
						if(const VarDecl *vardecl = dyn_cast<VarDecl>(decl->getDecl())){
								if(sr_uses.getattr(sr)){
										if(const ArrayType *atype = dyn_cast<ArrayType>(vardecl->getType().getTypePtr())){
												defs.insert(vardecl);
										}else if(const PointerType *ptype = dyn_cast<PointerType>(vardecl->getType().getTypePtr())){
												defs.insert(vardecl);
										}else if(const ReferenceType *rtype = dyn_cast<ReferenceType>(vardecl->getType().getTypePtr())){
												defs.insert(vardecl);
										}
								}
								uses.insert(vardecl);
						}
				}
				if(sr_defs.find(sr) != sr_defs.end()){
						if(const VarDecl *vardecl = dyn_cast<VarDecl>(decl->getDecl())){
								defs.insert(vardecl);
						}
				}
				return ;
		}
		else if(const ArraySubscriptExpr* array = dyn_cast<ArraySubscriptExpr>(st)){
				SourceRange sr(array->getLocStart(), array->getLocEnd());
				SourceRange sub_sr(array->getLHS()->getLocStart(), array->getLHS()->getLocEnd());
				if(sr_uses.find(sr) != sr_uses.end()){
						sr_uses.insert(sub_sr);
				}
				if(sr_defs.find(sr) != sr_defs.end()){
						sr_defs.insert(sub_sr);
				}
		}

		std::queue<const Stmt*> que;
		que.push(st);
		while(!que.empty()){
	  	  const Stmt* s_it = que.front();
		  	que.pop();
				if (const CallExpr *CE = dyn_cast<CallExpr>(s_it)) {
		  			unsigned int nArgs = CE->getNumArgs();
						for(int i=0; i<nArgs; i++){
								const Expr* expr = CE->getArg(i);
								SourceRange sr(expr->getLocStart(), expr->getLocEnd());
								sr_uses.insert(sr, true);
								que.push(expr);
						}
				}
				else if(const MemberExpr* mexpr = dyn_cast<MemberExpr>(s_it)){
						SourceRange sr(mexpr->getBase()->getLocStart(), mexpr->getBase()->getLocEnd());
						sr_uses.insert(sr);
						que.push(mexpr->getBase());
				}
				else if (const BinaryOperator *BO = dyn_cast<BinaryOperator>(s_it)){
		    		//std::cout<<"binary operation: "<<std::endl;
						SourceRange sr_left(BO->getLHS()->getLocStart(), BO->getLHS()->getLocEnd());
						SourceRange sr_right(BO->getRHS()->getLocStart(), BO->getRHS()->getLocEnd());
						if (BO->getOpcode() == BO_Assign) {
								que.push(BO->getRHS());
								sr_uses.insert(sr_right);
						}
						else{
		  					que.push(BO->getLHS());
								que.push(BO->getRHS());
								sr_uses.insert(sr_right);
								sr_uses.insert(sr_left);
						}
				}else if (const DeclRefExpr *decl = dyn_cast<DeclRefExpr>(s_it)){
		  			//std::cout<<"get declrefexpr"<<std::endl;
		  			if(const VarDecl *vardecl = dyn_cast<VarDecl>(decl->getDecl())){
								uses.insert(vardecl);
						}
				}else if(const TypoExpr* pl = dyn_cast<TypoExpr>(s_it)){
						//TODO: there're many place holder type that will be filled in later...
						//SourceRange sr(pl->getLocStart(), pl->getLocEnd());
						//sr_uses.insert(sr);
				}else if(const CastExpr* ce = dyn_cast<CastExpr>(s_it)){
						que.push(ce->getSubExpr());
				}
		}
	
//DEFS analysis
		que.push(st);
		while(!que.empty()){
	  	  const Stmt* s_it = que.front();
		  	que.pop();
				if(const CXXMemberCallExpr* calle = dyn_cast<CXXMemberCallExpr>(s_it)){
						Expr* classH = calle->getImplicitObjectArgument() ;
						CXXMethodDecl* methodD = calle->getMethodDecl();
						SourceRange sr(classH->getLocStart(), classH->getLocEnd());
						if(methodD->isConst()){
							sr_uses.insert(sr);
					  	continue;
						}
						//que.push(classH);
						if(DeclRefExpr *_decl = dyn_cast<DeclRefExpr>(classH)){
							if(const VarDecl *vardecl = dyn_cast<VarDecl>(_decl->getDecl())){
								cout<<vardecl->getNameAsString()<<endl;
								defs.insert(vardecl);
							}
#ifdef DEBUG
								cout<<"callee decl range:";
								printSourceRange(calle->getCalleeDecl()->getLocStart(), calle->getCalleeDecl()->getLocEnd(), M);
#endif
						}
						sr_defs.insert(sr);
				}else if (const CallExpr *CE = dyn_cast<CallExpr>(s_it)) {
						const Expr* classH = (CE->getCallee());
						que.push(classH);
						SourceRange sr(classH->getLocStart(), classH->getLocEnd());
						sr_defs.insert(sr);
				}
				else if (const BinaryOperator *BO = dyn_cast<BinaryOperator>(s_it)){
						if (BO->getOpcode() == BO_Assign) {
              	//if (const DeclRefExpr *DR =
                    //dyn_cast<DeclRefExpr>(BO->getLHS()->IgnoreParens())) {
												//if (const VarDecl *vard = dyn_cast<VarDecl>(DR->getDecl())){
														SourceRange sr_left(BO->getLHS()->getLocStart(), BO->getLHS()->getLocEnd());
														sr_defs.insert(sr_left);
														que.push(BO->getLHS());
												//}
								//}
						}
				}else if(const UnaryOperator *UO = dyn_cast<UnaryOperator>(s_it)){
						que.push(UO->getSubExpr());
						SourceRange sr(UO->getSubExpr()->getLocStart(), UO->getSubExpr()->getLocEnd());
						sr_defs.insert(sr);
						sr_uses.insert(sr);
				}
		}
}

bool inAnyQuery(SourceLocation begin, SourceLocation end, SourceManager& M){
		BeforeThanCompare<SourceLocation> Compare(M);
		if(begin.isValid() == false || end.isValid() == false){
				return false;
		}
		//if(query_blocks.size()==0)
		//		return false;
		//queryBlock* last_qb = query_blocks[query_blocks.size()-1];
		queryBlock* last_qb = NULL;
		int i = code_blocks.size()-1;
		while(i>=0){
				if(code_blocks[i]->type == QUERY){
						last_qb = reinterpret_cast<queryBlock*>(code_blocks[i]);
						break;
				}
				i--;
		}
		if(last_qb == NULL)
				return false;
		if(last_qb->begin.isValid() == false || last_qb->end.isValid() == false)
				return false;
/*		if(Compare(last_qb->begin, begin) &&
				Compare(end, last_qb->end))
				return true;
		return false;*/

		//XXX: Because the last_qb->end and end can be equal, which is considered as within the query block, listing the conditions for returning false instead of true.
		if(Compare(begin, last_qb->begin) 
				|| Compare(last_qb->end, end)
				|| Compare(last_qb->end, begin)
				|| Compare(end, last_qb->begin))
				return false;
		return true;

}


condBlock* ifBlock::isWithin(SourceLocation loc, SourceManager& M){
		BeforeThanCompare<SourceLocation> Compare(M);
		if(loc.isValid() && begin.isValid() && end.isValid()){
				if(Compare(begin, loc) && Compare(loc, end))
						return this;
		}
		if(loc.isValid() && matched_else!=NULL && matched_else->begin.isValid() && matched_else->end.isValid()){
				if(Compare(matched_else->begin, loc) && Compare(loc, matched_else->end))
						return matched_else;
		}
		return NULL;
}

condBlock* elseBlock::isWithin(SourceLocation loc, SourceManager& M){
		BeforeThanCompare<SourceLocation> Compare(M);
		if(loc.isValid() && begin.isValid() && end.isValid()){
				if(Compare(begin, loc) && Compare(loc, end))
						return this;
		}
		if(loc.isValid() && matched_if->begin.isValid() && matched_if->end.isValid()){
				if(Compare(matched_if->begin, loc) && Compare(loc, matched_if->end))
						return matched_if;
		}
		return NULL;
}

condBlock* forBlock::isWithin(SourceLocation loc, SourceManager& M){
		BeforeThanCompare<SourceLocation> Compare(M);
		if(loc.isValid() && begin.isValid() && end.isValid()){
				if(Compare(begin, loc) && Compare(loc, end)){
						return this;
				}
		}
		return NULL;	
}

condBlock* forBlock::isWithin(SourceLocation loc_begin, SourceLocation loc_end, SourceManager& M){
		if(rangeIsWithin(loc_begin, loc_end, begin, end, M)){
				return this;
		}
		return NULL;
}

condBlock* whileBlock::isWithin(SourceLocation loc, SourceManager& M){
		BeforeThanCompare<SourceLocation> Compare(M);
		if(loc.isValid() && begin.isValid() && end.isValid()){
				if(Compare(begin, loc) && Compare(loc, end)){
						return this;
				}
		}
		return NULL;	
}

condBlock* whileBlock::isWithin(SourceLocation loc_begin, SourceLocation loc_end, SourceManager& M){
		if(rangeIsWithin(loc_begin, loc_end, begin, end, M)){
				return this;
		}
		return NULL;
}

condBlock* elseBlock::isWithin(SourceLocation loc_begin, SourceLocation loc_end, SourceManager& M){
		if(rangeIsWithin(loc_begin, loc_end, begin, end, M)){
				return this;
		}
		return NULL;
}

condBlock* ifBlock::isWithin(SourceLocation loc_begin, SourceLocation loc_end, SourceManager& M){
		if(rangeIsWithin(loc_begin, loc_end, begin, end, M)){
				return this;
		}
		if(matched_else!=NULL && rangeIsWithin(loc_begin, loc_end, matched_else->begin, matched_else->end, M)){
				return matched_else;
		}
		return NULL;
}

bool condBlock::checkDep(set<const VarDecl*>& defs, set<const VarDecl*>& uses){
		for(set<const VarDecl*>::iterator it = defs.begin(); it != defs.end(); it++){
				if(uses.find(*it) != uses.end())
						return true;
		}
		return false;
}

bool ifBlock::checkQueryDep(queryBlock* q1, queryBlock* q2){
		//Two query blocks not breakable if q1 modifies the if condition
		bool defuse = checkDep(q1->defs, uses);
		return defuse;
}

bool elseBlock::checkQueryDep(queryBlock* q1, queryBlock* q2){
		//Two query blocks not breakable if q1 modifies the if condition
		if(matched_if == NULL)
					return true;
		bool defuse = checkDep(q1->defs, matched_if->uses);
		return defuse;
}

bool forBlock::checkQueryDep(queryBlock* q1, queryBlock* q2){
		bool cross_loop = false;
		//in for loop: if q2 defines sth. that q1 uses, there's also dependency
		//q2 may also defines a[i+1], and q1 uses a[i]
		//TODO: how to check array index??
		cross_loop = checkDep(q1->defs, q2->uses);
		bool defuse = checkDep(q1->defs, uses);
		return (cross_loop  & defuse);
}


bool whileBlock::checkQueryDep(queryBlock* q1, queryBlock* q2){
		bool cross_loop = false;
		cross_loop = checkDep(q1->defs, q2->uses);
		bool defuse = checkDep(q1->defs, uses);
		return (cross_loop  & defuse);
}

forBlock::forBlock(){
	type = FOR;
}

whileBlock::whileBlock(){
	type = WHILE;
}

ifBlock::ifBlock(){
	type = IF;
}

elseBlock::elseBlock(){
	type = ELSE;
}

SourceLocation condBlock::instack_getStart(){
	if(type == ELSE){
			elseBlock* pb = reinterpret_cast<elseBlock*>(this);
			return pb->matched_if->begin;
	}
	return begin;
}

SourceLocation condBlock::instack_getEnd(){
	if(type == IF){
			ifBlock* pb = reinterpret_cast<ifBlock*>(this);
			if(pb->matched_else==NULL)
					return end;
			else
				return	pb->matched_else->end;
	}
	return end;
}

codeBlock::codeBlock(){
	processed = false;
	conflict_index = 0;
	sort_index = 0;
	sort_base = 0;
}

queryBlock::queryBlock(){
	type = QUERY;
}

nonqueryBlock::nonqueryBlock(){
	type = NONQUERY;
}

#ifdef EXACT_USEDEF
void setControlFlow(){
	vector<condBlock*> all_cond_blocks;
	for(int i=0; i<code_blocks.size()-1; i++){
			int j = i+1;
			code_blocks[i]->successor.insert(code_blocks[j]);
			code_blocks[j]->predecessor.insert(code_blocks[i]);
			for(j=0; j<code_blocks[i]->cond_blocks.size(); j++){
					bool exist = false;
					for(int k=0; k<all_cond_blocks.size(); k++)
							if(all_cond_blocks[k] == code_blocks[i]->cond_blocks[j])
									exist = true;
					if(exist == false)
							all_cond_blocks.push_back(code_blocks[i]->cond_blocks[j]);
			}
	}
	bool nochange = false;
	while(nochange == false){
			nochange = true;
			for(int i=0; i<all_cond_blocks.size(); i++){
					condBlock* condb = all_cond_blocks[i];
					if(condb->type == IF){
							ifBlock* ifb = reinterpret_cast<ifBlock*>(condb);
							if(ifb->matched_else != NULL){
									elseBlock* efb = reinterpret_cast<elseBlock*>(ifb->matched_else);
									codeBlock* pre = condb->stmt_list[condb->stmt_list.size()-1];
									codeBlock* suc = efb->stmt_list[0];
									if(pre->successor.find(suc) != pre->successor.end()){
											nochange = false;
											pre->successor.erase(suc);
											suc->predecessor.erase(pre);
									}
									pre = condb->stmt_list[0];
									suc = efb->stmt_list[0];
									for(set<codeBlock*>::iterator it = pre->predecessor.begin(); it != pre->predecessor.end(); it++){
											if((*it) != suc && suc->predecessor.find(*it) == suc->predecessor.end()){
												nochange = false;
												suc->predecessor.insert(*it);
												(*it)->successor.insert(suc);
											}
									}
									pre = condb->stmt_list[condb->stmt_list.size()-1];
									suc = efb->stmt_list[efb->stmt_list.size()-1];
									for(set<codeBlock*>::iterator it = suc->successor.begin(); it != suc->successor.end(); it++){
											if((*it) != pre && pre->successor.find(*it) == pre->successor.end()){
												pre->successor.insert(*it);
												(*it)->predecessor.insert(pre);
												nochange = false;
											}
									}
					}else{
							codeBlock* pre = condb->stmt_list[0];
							codeBlock* suc = condb->stmt_list[condb->stmt_list.size()-1];
							for(set<codeBlock*>::iterator it = suc->successor.begin(); it != suc->successor.end(); it++){
										for(set<codeBlock*>::iterator it1 = pre->predecessor.begin(); it1!= pre->predecessor.end(); it1++){
												if((*it) != (*it1) && (*it1)->successor.find(*it) == (*it1)->successor.end()){
														nochange = false;
														(*it1)->successor.insert(*it);
												} 
										}
							}	 
					}
				}/*else if(condb->type == WHILE || condb->type == FOR){
						codeBlock* pre = condb->stmt_list[condb->stmt_list.size()-1];
						codeBlock* suc = condb->stmt_list[0];
						if(pre != suc && pre->successor.find(suc) == pre->successor.end()){
									nochange = false;
									pre->successor.insert(suc);
									suc->predecessor.insert(pre);
						}
				}*/

		}
	}

#ifdef DEBUG
	cout<<"%%%%%%%% setControlFlow %%%%%%%%%"<<endl<<endl;
	for(int i=0; i<code_blocks.size(); i++){
			cout<<"######## "<<code_blocks[i]->label<<" ####### 's successor:"<<endl;
			for(set<codeBlock*>::iterator it = code_blocks[i]->successor.begin(); it != code_blocks[i]->successor.end(); it++){
					cout<<"\t $ "<<(*it)->label<<endl;
			}
			cout<<" ------------------- "<<endl;
	}
#endif

	for(int i=0; i<code_blocks.size(); i++){
			codeBlock* cb = code_blocks[i];
			for(set<const VarDecl*>::iterator it = cb->uses.begin(); it != cb->uses.end(); it++){
					set<codeBlock*> scb;
					cb->real_uses[*it] = scb;
			}
			for(set<const VarDecl*>::iterator itv = cb->uses.begin(); itv != cb->uses.end(); itv++){
					set<codeBlock*> processed_blocks;
					processed_blocks.insert(cb);
					queue<codeBlock*> que;
					
					for(set<codeBlock*>::iterator it = cb->predecessor.begin(); it != cb->predecessor.end(); it++){
							que.push(*it);
					}
					while(!que.empty()){
							codeBlock* pre = que.front();
							que.pop();
							processed_blocks.insert(pre);
							if(pre->defs.find(*itv) != pre->defs.end()){
									cb->real_uses[*itv].insert(pre);
							}else{
									for(set<codeBlock*>::iterator preit = pre->predecessor.begin(); preit != pre->predecessor.end(); preit++)
											if(processed_blocks.find(*preit) == processed_blocks.end()){
													que.push(*preit);
											}
							}
					}
			}
#ifdef DEBUG
		cout<<endl<<endl<<"============code block "<<cb->label<<"============"<<endl;
		for(set<const VarDecl*>::iterator it = cb->uses.begin(); it != cb->uses.end(); it++){
			set<codeBlock*> scb = cb->real_uses[*it];
			cout<<"var "<<(*it)->getNameAsString()<<" is defined by: "<<endl;
			for(set<codeBlock*>::iterator itv = scb.begin(); itv != scb.end(); itv++){
					cout<<"\t <"<<(*itv)->label<<">"<<endl;
			}
		}
#endif
	}
	
}
#endif

string codeBlock::getBlockStr(){
	bool contain_query = false;
	if(type == QUERY)
			contain_query = true;
	char label_begin[100];
	char label_end[100];
	sprintf(label_begin, "\nQUERY_BEGIN_%s:\n", label.c_str());
	sprintf(label_end, "\nQUERY_END_%s:\n\n", label.c_str());
	char code[65536] = {0};
//	string source_code = recursiveGenerate(cond_blocks.size()-1, contain_query);
	string pre_label = label;
	string source_code = recursiveGenerate(cond_blocks.size()-1);
#ifdef DEBUG
	cout<<"%%%%%%%%% "<<label<<" ("<<pre_label<<") %%%%%%%%%"<<endl;
	cout<<source_code<<endl;
	cout<<"%%%%%%%% end "<<label<<" %%%%%%%%"<<endl;
#endif
	if(contain_query == false)
			return source_code;
	sprintf(code, "%s%s%s", label_begin, source_code.c_str(), label_end);
	string code_str(code);
	return code_str;
}

void codeBlock::setDepBlocks(){
	set<codeBlock*> outer_while_contains;
	for(int i=cond_blocks.size()-1; i>=0; i--){
		//Most outer while/for loop: add all blocks inside the loop into outer_while_contains set
		if(cond_blocks[i]->type == WHILE || cond_blocks[i]->type == FOR){
				for(int j=0; j<code_blocks.size(); j++){
						for(int k=0; k<code_blocks[j]->cond_blocks.size(); k++){
								if(code_blocks[j]->cond_blocks[k] == cond_blocks[i]){
										outer_while_contains.insert(code_blocks[j]);
								}
						}
				}
		}
		//cond 1: if this is in a for/while loop and affect the condition: put all codeblocks into its set
		if(crossCompare(defs, cond_blocks[i]->uses)){
#ifdef DEBUG
			cout<<"code block "<<label<<"conflict with if/while/for conditions"<<endl;
#endif
			if(cond_blocks[i]->type == WHILE || cond_blocks[i]->type == FOR){
					for(int j=0; j<code_blocks.size(); j++){
							for(int k=0; k<code_blocks[j]->cond_blocks.size(); k++){
									if(code_blocks[j]->cond_blocks[k] == cond_blocks[i]){
#ifdef DEBUG
								cout<<"Due to for/while loop conditions, ##"<<code_blocks[j]->label<<"## depend on ##"<<label<<"##"<<endl;
#endif
											code_blocks[j]->dep_blocks.insert(this);
											dep_blocks.insert(code_blocks[j]);
									}
							}
					}
			}else{
					for(int j=0; j<code_blocks.size(); j++){
							for(int k=0; k<code_blocks[j]->cond_blocks.size(); k++){
									if(code_blocks[j]->cond_blocks[k] == cond_blocks[i]){
											//if block but inside the big while loop; all blocks inside the if block should be sticked together
											if(code_blocks[j]->cond_blocks[k] == cond_blocks[i] 
													|| outer_while_contains.find(code_blocks[j]) != outer_while_contains.end()){
													code_blocks[j]->dep_blocks.insert(this);
													dep_blocks.insert(code_blocks[j]);
											}
									}
							}
					}
			}
		}
		//cond2: data dependencies
		for(int j=index+1; j<code_blocks.size(); j++){
			if(cond_blocks[i]->type == WHILE || cond_blocks[i]->type == FOR){
					for(int k=0; k<code_blocks[j]->cond_blocks.size(); k++){
							if(code_blocks[j]->cond_blocks[k] == cond_blocks[i] &&
									(crossCompare(code_blocks[j]->defs, uses) || 
									 crossCompare(defs, code_blocks[j]->uses))){
									const VarDecl* v1 = crossCompare(code_blocks[j]->defs, uses);
									const VarDecl* v2 = crossCompare(defs, code_blocks[j]->uses);
									if(v1 != NULL){
											if(const ArrayType *atype = dyn_cast<ArrayType>(v1->getType().getTypePtr())){}
											else{
#ifdef DEBUG
								cout<<"Due to data dependency, ##"<<code_blocks[j]->label<<"## depend on ##"<<label<<"##"<<endl;
#endif
												code_blocks[j]->dep_blocks.insert(this);
												dep_blocks.insert(code_blocks[j]);
											}
									}
									if(v2 != NULL){
											if(const ArrayType *atype = dyn_cast<ArrayType>(v2->getType().getTypePtr())){}
											else{
#ifdef DEBUG
								cout<<"Due to data dependency, ##"<<code_blocks[j]->label<<"## depend on ##"<<label<<"##"<<endl;
#endif
												code_blocks[j]->dep_blocks.insert(this);
												dep_blocks.insert(code_blocks[j]);
											}
									}
							}
							else if(code_blocks[j]->cond_blocks[k] == cond_blocks[i]){
									for(int l=0; l<k; l++){
											if(crossCompare(defs, code_blocks[j]->cond_blocks[l]->uses)){
#ifdef DEBUG
								cout<<"Due to data dependency to consequent for/while/if block condition, ##"<<code_blocks[j]->label<<"## depend on ##"<<label<<"##"<<endl;
#endif
													code_blocks[j]->dep_blocks.insert(this);
													dep_blocks.insert(code_blocks[j]);

											}
									}
							}
					}
			}
/*
			else if(cond_blocks[i]->type == IF || cond_blocks[i]->type == ELSE){
					for(int k=0; k<code_blocks[j]->cond_blocks.size(); k++){
							if(code_blocks[j]->cond_blocks[k] == cond_blocks[i] &&
									 crossCompare(defs, code_blocks[j]->uses)){
#ifdef DEBUG
								cout<<"Due to data dependency, ##"<<code_blocks[j]->label<<"## depend on ##"<<label<<"##"<<endl;
#endif
											code_blocks[j]->dep_blocks.insert(this);
											dep_blocks.insert(code_blocks[j]);
							}
					}
			}
*/
		}
	}

}


void codeBlock::propagateDep(){
	//Dep property propagate:
	//If A has dep_block B and B has dep_block C, then A has dep_block C
	queue<codeBlock*> que;
	for(set<codeBlock*>::iterator it = dep_blocks.begin(); it != dep_blocks.end(); it++)
			que.push(*it);
	while(!que.empty()){
			codeBlock* cb = que.front();
			que.pop();
			for(set<codeBlock*>::iterator it1 = cb->dep_blocks.begin(); it1 != cb->dep_blocks.end(); it1++){
					if((*it1)!= this && dep_blocks.find(*it1) == dep_blocks.end()){
#ifdef DEBUG
							cout<<"Propagation: add ##"<<(*it1)->label<<"## into ##"<<label<<"##'s deplist"<<endl;
#endif
							dep_blocks.insert(*it1);
							que.push(*it1);
					}
			}
	}
}
string codeBlock::recursiveGenerate(int i){
	if(i<0){
		source.append("\n");
		return source;
	}
	char code[65536] = {0};
	string dep_blocks_code("");
	string s = recursiveGenerate(i-1);
	for(int j=index+1; j<code_blocks.size(); j++){
		if(dep_blocks.find(code_blocks[j]) == dep_blocks.end() || code_blocks[j]->processed == true)
					continue;
		for(int k=0; k<code_blocks[j]->cond_blocks.size(); k++){
				if(code_blocks[j]->cond_blocks[k] == cond_blocks[i]){
						dep_blocks_code.append(code_blocks[j]->recursiveGenerate(k-1));
#ifdef DEBUG
						cout<<"block ##"<<label<<"## appends block ##"<<code_blocks[j]->label<<"## into its dep_list, k = "<<k<<endl;
#endif
						if(type == NONQUERY && code_blocks[j]->type == QUERY)
									label = code_blocks[j]->label;
//Add code_blocks[j]'s use/def set to this use/def set
						addSets(defs, code_blocks[j]->defs);
						addSets(uses, code_blocks[j]->uses);
						addSets(code_blocks[j]->defs, defs);
						addSets(code_blocks[j]->uses, uses);

						conflict_index = max(conflict_index, code_blocks[j]->conflict_index);
						code_blocks[j]->processed = true;
				}
		}
	}

	if(cond_blocks[i]->type == IF){
			sprintf(code, "if(%s){\n\t%s\n%s\n}\n", cond_blocks[i]->cond_str.c_str(), s.c_str(), dep_blocks_code.c_str());
	}else if(cond_blocks[i]->type == ELSE){
			sprintf(code, "if(!(%s)){\n\t%s\n%s\n}\n", cond_blocks[i]->cond_str.c_str(), s.c_str(), dep_blocks_code.c_str());
	}else if(cond_blocks[i]->type == WHILE){
			sprintf(code, "while(%s){\n\t%s\n%s\n}\n", cond_blocks[i]->cond_str.c_str(), s.c_str(), dep_blocks_code.c_str());
	}else if(cond_blocks[i]->type == FOR){
			forBlock* for_block = reinterpret_cast<forBlock*>(cond_blocks[i]);
			sprintf(code, "for(%s; %s; %s){\n\t%s\n%s\n}\n", for_block->init_str.c_str(), for_block->cond_str.c_str(), for_block->inc_str.c_str(), s.c_str(), dep_blocks_code.c_str());
	}

	string str(code);
	return str;
}

int codeBlock::crossCompareAllConds(condBlock* most_inner_cond, int c_block){
	int i=0;
	while(i<cond_blocks.size()){
		if(cond_blocks[i] == most_inner_cond){
				for(int j=i-1; j>=0; j--){
						//Any cond block uses cross with c_block's defs: indicate a data dependency
						if(crossCompare(code_blocks[c_block]->defs, cond_blocks[j]->uses)){
								return i;
						}
				}
				return -1;
		}else
			i++;
	}
	return -1;
}


const char* getStmtType(Stmt* st){
	if(const AttributedStmt* ast = dyn_cast<AttributedStmt>(st))
			return "AttributeStmt";
	else if(const BreakStmt* bst = dyn_cast<BreakStmt>(st))
			return "BreakStmt";
	else if(const CompoundStmt* cst = dyn_cast<CompoundStmt>(st))
			return "CompoundStmt";
	else if(const ContinueStmt* cst = dyn_cast<ContinueStmt>(st))
			return "ContinueStmt";
	else if(const DeclStmt* dst = dyn_cast<DeclStmt>(st))
			return "DeclStmt";
	else if(const Expr* expr = dyn_cast<Expr>(st)){
			if(const BinaryOperator *bop = dyn_cast<BinaryOperator>(expr))
					return "BinaryOperator";
			else if(const BlockExpr *bop = dyn_cast<BlockExpr>(expr))
					return "BlockExpr";
			return "EXPR ";
	}
	else if(const DoStmt* expr = dyn_cast<DoStmt>(st))
			return "DoStmt";
	else if(const ForStmt* fst = dyn_cast<ForStmt>(st))
			return "ForStmt";
	else if(const GotoStmt* gst = dyn_cast<GotoStmt>(st))
			return "GotoStmt";
	else if(const LabelStmt* gst = dyn_cast<LabelStmt>(st))
			return "labelStmt";
	else if(const NullStmt* nst = dyn_cast<NullStmt>(st))
			return "NullStmt";
	else if(const WhileStmt* wst = dyn_cast<WhileStmt>(st))
			return "WhileStmt";
	else if(const ReturnStmt* rst = dyn_cast<ReturnStmt>(st))
			return "ReturnStmt";
	else if(const IfStmt* rst = dyn_cast<IfStmt>(st))
			return "IfStmt";
	else
			return "DONTKNOW";
	
}


void splitByBlank(vector<string>& vec, string str){
	char s[1000] = {0};
	int i = 0;
	int j = 0;
	while(i < str.length()){
		if(str[i]==' ' || str[i] == '\t' ||str[i]=='\n' || str[i] == ','){
				string s1(s);
				vec.push_back(s1);
				memset(s, 0, sizeof(char)*1000);
				j = 0;
				i++;
		}else{
				s[j] = str[i];
				i++, j++;
		}
	}
	string s1(s);
	vec.push_back(s1);
}

int getConflictIndex(int row_num, OPERATION op){
	int pri_index = 4000000/row_num;
	if(op == UPDATE)
		pri_index += 10000;
	if(op == INSERT)
		pri_index = 10;
	return pri_index;
}

void readConflictInfo(){
	ifstream infile;
	infile.open("dbname.txt");
	char dbname[100];
	infile>>dbname;
#ifdef DEBUG
	cout<<"Database name:"<<dbname<<endl;
#endif
	infile.close();

	char tbl_descp_name[100];
	sprintf(tbl_descp_name, "%s_freq.txt", dbname);
	infile.open(tbl_descp_name);
	char line[500];	
	string line_str;
	vector<string> split_str;
	while(!infile.eof()){
		infile.getline(line, 500);
		TABLE_DESC tbl;
		line_str.assign(line);
		split_str.clear();
		splitByBlank(split_str, line_str);
		if(split_str.size()!=2)
				break;
		tbl.name = split_str[0];
		sscanf(split_str[1].c_str(), "%d", &tbl.num_rows);
		tables[tbl.name] = tbl;
#ifdef DEBUG
		cout<<"TABLE "<<tbl.name<<"'s num_rows = "<<tbl.num_rows<<endl;
#endif
	}
}

bool qBlockCmp1(codeBlock* q1, codeBlock* q2){
		return q1->conflict_index < q2->conflict_index;
}
bool qBlockCmp(codeBlock* q1, codeBlock* q2){
		return q1->sort_index < q2->sort_index;
}
bool reorderBufCmp(reorderBufferUnit r1, reorderBufferUnit r2){
		return r1.cycle_finish < r2.cycle_finish;
}
