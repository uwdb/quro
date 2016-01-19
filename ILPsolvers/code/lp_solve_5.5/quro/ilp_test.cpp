#include "stdlib.h"
#include "lp_lib.h"
#include <fstream>
#include <iostream>

#define MAXV 10000
using namespace std;

int nQueries = 5;
int c_index[200] = {0};
int dep_cond[1000][2] = {0};
int rd_cond[1000][3] = {0};
int num_dep_cond = 0;
int num_rd_cond = 0;
int get_indicator_index(int q1, int q2){
	if(q1 > q2){
		int tmp = q2;
		q2 = q1;
		q1 = tmp;
	}
	int x = (nQueries+q2)*(nQueries-q2-1)/2 + (q2 - q1) - 1;
	return nQueries + x;	
}

int cnt_num_blank(char* line){
	int i = 0;
	int cnt = 0;
	while(i<strlen(line)){
		if(line[i]==' ')
			cnt++;
		i++;
	}
	return cnt;
}
int demo()
{
  lprec *lp;
  int Ncol, *colno = NULL, j, ret = 0;
  REAL *row = NULL;


	ifstream infile("queryinfo.txt");
	infile>>nQueries;
	for(int i=0; i<nQueries; i++)
		infile>>c_index[i];
	char line[300];
	while(!infile.eof()){
		infile.getline(line, 300);
		if(cnt_num_blank(line)==1){
				sscanf(line, "%d %d", &dep_cond[num_dep_cond][0], &dep_cond[num_dep_cond][1]);
				num_dep_cond++;
		}else if(cnt_num_blank(line)==2){
				sscanf(line, "%d %d %d", &rd_cond[num_rd_cond][0], &rd_cond[num_rd_cond][1], &rd_cond[num_rd_cond][2]);
				num_rd_cond++;
		}
	}
	cout<<"read file complete"<<endl;
  /* We will build the model row by row
     So we start with creating a model with 0 rows and 2 columns */
  Ncol = nQueries * nQueries + num_rd_cond; /* there are two variables in the model */
  lp = make_lp(0, Ncol);
  if(lp == NULL)
    ret = 1; /* couldn't construct a new model... */

	set_verbose(lp, CRITICAL);

  if(ret == 0) {
    //set_col_name(lp, 1, "x");
    //set_col_name(lp, 2, "y");

		char _name[100] = {0};
		for(int i=0; i<nQueries; i++){
				sprintf(_name, "q%d", i+1);
				set_col_name(lp, i+1, _name);
		}
		for(int i=0; i<nQueries; i++){
			for(int j=i+1; j<nQueries; j++){
				int x = get_indicator_index(i, j);
				sprintf(_name, "helper%d_%d", i+1, j+1);
				set_col_name(lp, x+1, _name);
			}
		}

		for(int i=0; i<num_rd_cond; i++){
			sprintf(_name, "or%d_%d_%d", rd_cond[i][0], rd_cond[i][1], rd_cond[i][2]);
			set_col_name(lp, nQueries*nQueries+i+1, _name);
		}
    /* create space large enough for one row */
    colno = (int *) malloc(Ncol * sizeof(*colno));
    row = (REAL *) malloc(Ncol * sizeof(*row));
    if((colno == NULL) || (row == NULL))
      ret = 2;
  }
	
	memset(colno, 0, sizeof(*colno)*Ncol);
	memset(row, 0, sizeof(*row)*Ncol);
	for(int i=0; i<Ncol; i++){
		set_int(lp, i+1, TRUE);
		set_lowbo(lp, i+1, 0);
	}
	cout<<"set variable name finish"<<endl;
	set_add_rowmode(lp, TRUE);

	for(int i=0; i<nQueries; i++){
		for(int j=i+1; j<nQueries; j++){
				int x = get_indicator_index(i, j);
				for(int k=0; k < Ncol; k++){
						colno[k] = k+1;
						if(k == x){
								row[k] = 1;
						}else
								row[k] = 0;
				}
				if(!add_constraintex(lp, Ncol, row, colno, LE, 1))
					ret = 3;

				printf("helper%d_%d <= 1 (index = %d)\n", i, j, x);

				for(int k=0; k<Ncol; k++){
						colno[k] = k+1;
						if(k == x)
								row[k] = MAXV;
						else row[k] = 0;
				}
				row[i] = 1;
				row[j] = 0-1;
				if(!add_constraintex(lp, Ncol, row, colno, GE, 1))
					ret = 3;
				row[x] = 0-MAXV;
				row[i] = 0-1;
				row[j] = 1;
				if(!add_constraintex(lp, Ncol, row, colno, GE, 1-MAXV))
					ret = 3;
		}
	}
	if(ret != 0)
		return ret;
	memset(row, 0, sizeof(*row)*Ncol);
	memset(colno, 0, sizeof(*colno)*Ncol);
	for(int i=0; i<num_dep_cond; i++){
		int q1, q2;
		//read q1, q2: q1 before q2
		q1 = dep_cond[i][0];
		q2 = dep_cond[i][1];
		if(ret == 0 && q1 != q2){
/*
				j = 0;
				colno[j] = q1+1;
				row[j++] = 1;
				colno[j] = q2+1;
				row[j++] = 0-1;
*/
				for(int j=0; j<nQueries; j++){
						colno[j] = j+1;
						if(j == q1)
								row[j] = 0-1;
						else if(j == q2)
								row[j] = 1;
						else
								row[j] = 0;
				}
				for(int k=0; k<nQueries; k++)
					cout<<colno[k]<<" ";
				for(int k=0; k<nQueries; k++)
					cout<<row[k]<<" ";
				cout<<endl;
				cout<<"add condition: "<<q2<<" - "<<q1<<" > 0"<<endl;
				if(!add_constraintex(lp, nQueries, row, colno, GE, 1))
					ret = 3;
			//	if(!add_constraint(lp, row, LE, 1))
			//		ret = 3;
		}
	}
	if(ret != 0)
		return ret;
	for(int i=0; i<nQueries; i++){
		if(ret == 0){
			j = 0;
			//colno[j] = i+1;
			//row[j++] = 1;
			for(int j=0; j<nQueries; j++){
					colno[j] = j+1;
					if(j == i)
							row[j] = 1;
					else
							row[j] = 0;
			}
			//cout<<"constraint for q"<<i<<endl;
			//for(int k=0; k<nQueries; k++)
			//		cout<<colno[k]<<" ";
			//for(int k=0; k<nQueries; k++)
			//		cout<<row[k]<<" ";
			//cout<<endl;
			if(!add_constraintex(lp, nQueries, row, colno, LE, nQueries))
					ret = 3;
		}
	}

	memset(row, 0, sizeof(*row)*Ncol);
	memset(colno, 0, sizeof(*colno)*Ncol);

	if(ret != 0)
		return ret;
	for(int i=0; i<num_rd_cond; i++){
			for(int k=0; k < Ncol; k++){
				colno[k] = k+1;
				if(k == nQueries*nQueries+i)
						row[k] = 1;
				else
						row[k] = 0;
			}
			if(!add_constraintex(lp, Ncol, row, colno, LE, 1))
				ret = 3;
			for(int k=0; k < Ncol; k++){
				if(k == nQueries*nQueries+i)
					row[k] = MAXV;
				else
					row[k] = 0;
			}
			cout<<"add constraint: q"<<rd_cond[i][0]<<" - q"<<rd_cond[i][1]<<", q"<<rd_cond[i][2]<<endl;
			row[rd_cond[i][0]] = 1;
			row[rd_cond[i][2]] = 0-1;
			if(!add_constraintex(lp, Ncol, row, colno, GE, 1)){
					ret = 3;
					cout<<"SET RET TO 3"<<endl;
					return ret;
			}
			cout<<"\tadd 1 success"<<endl;
			row[nQueries*nQueries+i] = 0-MAXV;
			row[rd_cond[i][0]] = 0;
			row[rd_cond[i][1]] = 0-1;
			row[rd_cond[i][2]] = 1;
			if(!add_constraintex(lp, Ncol, row, colno, GE, 1-MAXV)){
					ret = 3;
					cout<<"SET RET TO 3"<<endl;
					return ret;
			}
	}
	cout<<"ret = "<<ret<<endl;
	
	if(ret == 0){
			set_add_rowmode(lp, FALSE);
			j = 0;
			cout<<"set goal: maximize (";
			for(int i=0; i<nQueries; i++){
					colno[j] = i+1;
					row[j++] = c_index[i];
					cout<<c_index[i]<<"*q"<<i;
					if(i != nQueries - 1)
						cout<<" + ";
			}
			cout<<endl;
			if(!set_obj_fnex(lp, j, row, colno))
					ret = 4;
	}


  if(ret == 0) {
    /* set the object direction to maximize */
    set_maxim(lp);

    /* just out of curioucity, now show the model in lp format on screen */
    /* this only works if this is a console application. If not, use write_lp and a filename */
    write_LP(lp, stdout);
    /* write_lp(lp, "model.lp"); */

    /* I only want to see important messages on screen while solving */
    set_verbose(lp, IMPORTANT);

    /* Now let lpsolve calculate a solution */
    ret = solve(lp);
    if(ret == OPTIMAL)
      ret = 0;
    else
      ret = 5;
  }

  if(ret == 0) {
    /* a solution is calculated, now lets get some results */

    /* objective value */
    printf("Objective value: %f\n", get_objective(lp));

    /* variable values */
    get_variables(lp, row);
    for(j = 0; j < Ncol; j++)
      printf("%s: %f\n", get_col_name(lp, j + 1), row[j]);

    /* we are done now */
  }

  /* free allocated memory */
//  if(row != NULL)
//    free(row);
//  if(colno != NULL)
//    free(colno);
//
//  if(lp != NULL) {
//    /* clean up such that all used memory by lpsolve is freed */
//    delete_lp(lp);
//  }

  return(ret);
}

int main()
{
  demo();
}
