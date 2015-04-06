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
			nfas.top()->concatenate(a);
						
			cout << "removing start candidate: "; startPoints.top()->print();
			startPoints.pop();
			
			Tree *t = endPoints.top();
			endPoints.pop();
			endPoints.pop();
			endPoints.push(t);
		}
	}
	
	void unify() {
		Nfa *a = nfas.top();
		nfas.pop();
		nfas.top()->unify(a);
		
		Tree *r = startPoints.top();
		startPoints.pop();
		Tree *l = startPoints.top();
		startPoints.pop();
		Tree *node = new Tree(l, r);
		startPoints.push(node);
		
		r = endPoints.top();
		endPoints.pop();
		l = endPoints.top();
		endPoints.pop();
		node = new Tree(l, r);
		endPoints.push(node);
	}
	
	void pushNfa(Nfa *nfa) {
		//Nfa *nfa = new Nfa(symbol);
		nfas.push(nfa);
		Tree *leaf = new Tree(nfa->getInitial());
		startPoints.push(leaf);
		
		//char tmp = symbol;
		cout << "new start candidate: " << " " << nfa->getInitial()->getId() << endl;
		
		leaf = new Tree(nfa->getFinal());
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
ERE_dupl_symbol    : '*'	{cout << "STAR"; nfas.top()->apply_cardinality(KLEENE_STAR);}
                   | '+'	{cout << "PLUS"; nfas.top()->apply_cardinality(PLUS);}
                   | '?'	{cout << "OPT"; nfas.top()->apply_cardinality(OPTION);}
                   | '{' DUP_COUNT               '}'	{cout << "{"<<$2<<"}"; nfas.top()->apply_cardinality($2, SIMPLE_COUNT);}
                   | '{' DUP_COUNT ','           '}'	{cout << "{"<<$2<<",}"; nfas.top()->apply_cardinality($2, UNBOUNDED);}
                   | '{' DUP_COUNT ',' DUP_COUNT '}'	{cout << "{"<<$2<<","<<$4<<"}"; nfas.top()->apply_cardinality($2, $4);}
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
character_class : Open_colon class_name Colon_close		{cout << "CLASS: " << $2 << " "; bracket.applyClass($2);}
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
		cout << endl << nfas.size() << endl;
	
		cout << endl << startPoints.size() << endl;
		cout << endl << endPoints.size() << endl;
	
		while ( ! startPoints.empty()) {
			Tree *t = startPoints.top();
			t->applyStartRules();
			startPoints.pop();
		}
	
		while ( ! endPoints.empty()) {
			Tree *t = endPoints.top();
			t->applyEndRules();
			endPoints.pop();
		}
	}
	else {
		Nfa *nfa = new Nfa(State::DOT);
		nfa->getInitial()->setTransition(State::DOT, nfa->getInitial());
		nfa->getFinal()->setTransition(State::DOT, nfa->getFinal());
		nfas.push(nfa);
	}
	
	Nfa* thompson = nfas.top();
	thompson->toDot("thompson.dot");
	
	Nfa *noEps = new Nfa(thompson);
	noEps->eliminate_eps();
	noEps->toDot("noEps.dot");
	
	Dfa* dfa = new Dfa(noEps);
	dfa->toDot("dfa.dot");
	dfa->minimise_hopcroft();
	dfa->toDot("hopcroft.dot");
		
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
}


