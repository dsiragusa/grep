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
			cout << "LINEEND set skip on start candidate: "; endPoints.top()->print();
			isLineEnd = false;
			endPoints.top()->setSkip();
		}		
		else {
			Nfa *a = nfas.top();	
			nfas.pop();
			cout << "removing start candidate: "; startPoints.top()->print();
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
		
		cout << "new start candidate: " << " " << nfa->getInitial()->getId() << endl;
		
		leaf = new Tree(*nfa->getFinals().begin());
		endPoints.push(leaf);
	}
			
	void printBracket() {
		cout << "\n Bracket:";
		for (char el : bracket.getBracket()) {
			cout << " " << el;
		}
		cout << endl;
	}
	
	void parseFile(istream& input, Nfa *thompson, Nfa *noEps, Dfa *dfa, bool multipleFiles, char const *fileName) {
		string line;
		while (getline(input, line)) {
			if (multipleFiles)
				cout << fileName << ":";
			cout << "Thompson NFA:" << endl;
			thompson->evaluate(line);
			cout << "No-EPS NFA:" << endl;
			noEps->evaluate(line);
			cout << "DFA:" << endl;
			dfa->evaluate(line);
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
                   | extended_reg_exp '|' ERE_branch	{cout << "{OR}"; unify();}
                   ;
ERE_branch         :            ERE_expression
                   | '^'		ERE_expression		{cout << "{ATSTART}"; cout << "LINESTART set skip on start candidate: "; startPoints.top()->print(); startPoints.top()->setSkip();}
                   | ERE_branch ERE_expression		{cout << "CAT"; concatenate();}
                   ;
ERE_expression     : one_char_or_coll_elem_ERE	{cout << "PUSH";}
                   | '$'					{cout << "{ATEND}"; isLineEnd = true;}
                   | '(' extended_reg_exp ')'	{cout << "PAREN";}
                   | ERE_expression ERE_dupl_symbol
                   ;
one_char_or_coll_elem_ERE  : ORD_CHAR	{cout << "[" <<$1<<"]"; pushNfa(new Nfa($1));}
                   | QUOTED_CHAR		{cout << "["<<$1<<"]"; pushNfa(new Nfa($1));}
                   | '.'				{cout << "DOT"; pushNfa(new Nfa(State::DOT));}
                   | bracket_expression {bracket.clear();}
                   ;
ERE_dupl_symbol    : '*'	{cout << "STAR"; nfas.top()->applyCardinality(KLEENE_STAR);}
                   | '+'	{cout << "PLUS"; nfas.top()->applyCardinality(PLUS);}
                   | '?'	{cout << "OPT"; nfas.top()->applyCardinality(OPTION);}
                   | '{' DUP_COUNT               '}'	{cout << "{"<<$2<<"}"; nfas.top()->applyCardinality($2, SIMPLE_COUNT);}
                   | '{' DUP_COUNT ','           '}'	{cout << "{"<<$2<<",}"; nfas.top()->applyCardinality($2, UNBOUNDED);}
                   | '{' DUP_COUNT ',' DUP_COUNT '}'	{cout << "{"<<$2<<","<<$4<<"}"; nfas.top()->applyCardinality($2, $4);}
                   ;
bracket_expression : '[' matching_list    ']'	{cout << "matching"; printBracket(); pushNfa(new Nfa(bracket.getBracket(), true));}
                   | '[' nonmatching_list ']'	{cout << "non_matching"; printBracket(); pushNfa(new Nfa(bracket.getBracket(), false));}
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
range_expression : start_range end_range {cout << "{ENDRANGE}"; bracket.expandRange();}
                 | start_range '-'	{cout << "[-]"; bracket.setCollElem('-'); cout << "{ENDRANGE}"; bracket.expandRange();}
                 ;
start_range    : end_range '-'		{cout << "{RANGE}";}
               ;
end_range      : COLL_ELEM	{cout << "[" << $1 << "]"; bracket.setCollElem($1);}
               /*| collating_symbol*/
               ;
/*
collating_symbol : Open_dot COLL_ELEM Dot_close
               | Open_dot META_CHAR Dot_close
               ;
equivalence_class : Open_equal COLL_ELEM Equal_close
               ;
*/
character_class : Open_colon class_name Colon_close		{cout << "CLASS: " << $2 << " "; bracket.applyClass($2); free($2);}
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
	noEps->eliminateEps();
	noEps->toDot("dot/noEps.dot");
	
	Dfa* dfa = new Dfa(noEps);
	dfa->toDot("dot/dfa.dot");
	dfa->minimize();
	dfa->toDot("dot/brzo.dot");

/*	
	dfa = new Dfa(noEps);
	dfa->minimise_hopcroft();
	dfa->toDot("dot/hopcroft.dot");
	dfa->print();
	*/
	if (argc == 2)
		parseFile(cin, thompson, noEps, dfa, false, "");
	else {
		int nextFile = 2;
		bool multipleFiles = (argc - nextFile > 1);
		while (nextFile < argc) {
			ifstream fin(argv[nextFile]);
			parseFile(fin, thompson, noEps, dfa, multipleFiles, argv[nextFile]);
			fin.close();
			++nextFile;
		}
	}
	
	delete thompson;
	delete noEps;
	delete dfa;
}


