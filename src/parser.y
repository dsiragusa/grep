%{
	#include <cstring>
	#include <iostream>
	#include <fstream>
	#include <stack>
	#include "src/Nfa.h"
	#include "src/Dfa.h"
	#include "src/Tree.h"
	#include "src/BracketExprManager.h"
	
	using namespace std;
	
	int yylex();
	int yyparse();
	void parse_string(char const *);
	void yyerror(const char *p) { cerr << "\nInvalid regular expression\n"; exit(0);}
	
	BracketExprManager bracket = BracketExprManager();
	stack<Nfa *> nfas;
	stack<Tree *> startPoints;
	stack<Tree *> endPoints;
	bool isLineEnd = false;	

	void concatenate() {
		if (isLineEnd) {
			isLineEnd = false;
			endPoints.top()->setSkip();
		}		
		else {
			Nfa *a = nfas.top();	
			nfas.pop();
			nfas.top()->concatenate(a);
						
			delete startPoints.top();
			startPoints.pop();
			
			Tree *t = endPoints.top();
			endPoints.pop();
			delete endPoints.top();
			endPoints.pop();
			endPoints.push(t);
		}
	}
	
	void unifyTrees(stack<Tree *> *treeStack) {
		Tree *r = treeStack->top();
		treeStack->pop();
		Tree *l = treeStack->top();
		treeStack->pop();
		Tree *node = new Tree(l, r);
		treeStack->push(node);
	}
	
	void unify() {
		Nfa *a = nfas.top();
		nfas.pop();
		nfas.top()->unify(a);
		
		unifyTrees(&startPoints);
		unifyTrees(&endPoints);
	}
	
	void pushNfa(Nfa *nfa) {
		nfas.push(nfa);
		Tree *leaf = new Tree(nfa->getInitial());
		startPoints.push(leaf);
		
		leaf = new Tree(*nfa->getFinals().begin());
		endPoints.push(leaf);
	}
				
	void parseFile(istream& input, Dfa *dfa, bool multipleFiles, char const *fileName) {
		string line;
		while (getline(input, line)) {
			if (dfa->evaluate(line)) {
				if (multipleFiles)
					cout << fileName << ":";
				cout << line << endl;
			}
		}	
	}
	
	void applyRules(stack<Tree *> *toApply) {
		while ( ! toApply->empty()) {
			Tree *t = toApply->top();
			t->applyRules();
			delete t;
			toApply->pop();
		}
	}
	
%}

%union {
	int num;
	char *s;
	char c;
}

%token<c>  ORD_CHAR QUOTED_CHAR <num>DUP_COUNT
%token<c>    COLL_ELEM /*META_CHAR*/
%token    /*Open_equal Equal_close Open_dot Dot_close*/ Open_colon Colon_close
%token<s>    class_name
%type<num>	ERE_dupl_symbol

%start  extended_reg_exp
%%


extended_reg_exp   :                      ERE_branch
                   | extended_reg_exp '|' ERE_branch	{unify();}
                   ;
ERE_branch         :            ERE_expression
                   | '^'		ERE_expression		{startPoints.top()->setSkip();}
                   | ERE_branch ERE_expression		{concatenate();}
                   ;
ERE_expression     : one_char_or_coll_elem_ERE	
                   | '$'					{isLineEnd = true;}
                   | '(' extended_reg_exp ')'	
                   | ERE_expression ERE_dupl_symbol
                   ;
one_char_or_coll_elem_ERE  : ORD_CHAR	{pushNfa(new Nfa($1));}
                   | QUOTED_CHAR		{pushNfa(new Nfa($1));}
                   | '.'				{pushNfa(new Nfa(State::DOT));}
                   | bracket_expression {bracket.clear();}
                   ;
ERE_dupl_symbol    : '*'	{nfas.top()->applyCardinality(KLEENE_STAR);}
                   | '+'	{nfas.top()->applyCardinality(PLUS);}
                   | '?'	{nfas.top()->applyCardinality(OPTION);}
                   | '{' DUP_COUNT               '}'	{nfas.top()->applyCardinality($2, SIMPLE_COUNT);}
                   | '{' DUP_COUNT ','           '}'	{nfas.top()->applyCardinality($2, UNBOUNDED);}
                   | '{' DUP_COUNT ',' DUP_COUNT '}'	{nfas.top()->applyCardinality($2, $4);}
                   ;
bracket_expression : '[' matching_list    ']'	{pushNfa(new Nfa(bracket.getBracket(), true));}
                   | '[' nonmatching_list ']'	{pushNfa(new Nfa(bracket.getBracket(), false));}
                   ;
matching_list  : bracket_list
               ;
nonmatching_list : '^' bracket_list
               ;
bracket_list    :              expression_term
                | bracket_list expression_term
                ;
expression_term : single_expression
                | range_expression
                ;
single_expression : end_range
                  | character_class
               /*| equivalence_class*/
                  ;
range_expression : start_range end_range {bracket.expandRange();}
                 | start_range '-'	{bracket.setCollElem('-'); bracket.expandRange();}
                 ;
start_range    : end_range '-'		
               ;
end_range      : COLL_ELEM	{bracket.setCollElem($1);}
               /*| collating_symbol*/
               ;
/*
collating_symbol : Open_dot COLL_ELEM Dot_close
               | Open_dot META_CHAR Dot_close
               ;
equivalence_class : Open_equal COLL_ELEM Equal_close
               ;
*/
character_class : Open_colon class_name Colon_close		{bracket.applyClass($2); free($2);}
                ;

%%

int main(int argc, char** argv) {
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " [pattern] [file...]" << endl;
		exit(0);
	}
	
	
	if (strlen(argv[1]) > 0) {
		parse_string(argv[1]);
		yyparse();
		
		applyRules(&startPoints);
		applyRules(&endPoints);
	}
	else {
		Nfa *nfa = new Nfa(State::DOT);
		nfa->getInitial()->setTransition(State::DOT, nfa->getInitial());

		State* final = *nfa->getFinals().begin();
		final->setTransition(State::DOT, final);
		nfas.push(nfa);
	}
	
	Nfa* thompson = nfas.top();
	thompson->toDot("dot/thompson.dot");
	
	Nfa *noEps = new Nfa(thompson);
	delete thompson;
	noEps->eliminateEps();
	noEps->toDot("dot/noEps.dot");
	
	Dfa* dfa = new Dfa(noEps);
	delete noEps;
	dfa->toDot("dot/dfa.dot");
	dfa->minimize();
	dfa->toDot("dot/brzo.dot");

	if (argc == 2)
		parseFile(cin, dfa, false, "");
	else {
		int nextFile = 2;
		bool multipleFiles = (argc - nextFile > 1);
		while (nextFile < argc) {
			ifstream fin(argv[nextFile]);
			parseFile(fin, dfa, multipleFiles, argv[nextFile]);
			fin.close();
			++nextFile;
		}
	}
	
	delete dfa;
}


