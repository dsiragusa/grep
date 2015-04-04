%{
	#include <cstdio>
	#include <iostream>
	#include <stack>
	#include "src/Nfa.h"
	#include "src/Dfa.h"
	using namespace std;
	
	int yylex();
	int yyparse();
	void parse_string(char const *);
	void yyerror(const char *p) { cerr << "\nInvalid regular expression\n"; exit(0);}
	
	stack<Nfa *> nfas;
	list<int> bracket;
	stack<State *> startingAnywhere;
	int orCount = 0;
	
	bool isLineStart = false;
	bool wasLineEnd = false;
	bool isLineEnd = false;
	
	void concatenate() {
		if ( ! isLineStart && ! isLineEnd) {
			Nfa *a = nfas.top();	
			nfas.pop();
			
			nfas.top()->concatenate(a);
			startingAnywhere.pop();
			while (orCount > 0) {
				startingAnywhere.pop();
				orCount--;
			}
		}
		else if (isLineStart) {
			isLineStart = false;
			startingAnywhere.pop();						
		}
		else {
			isLineEnd = false;
			wasLineEnd = true;
		}		
	}
	
	void unify() {
		orCount++;
		Nfa *a = nfas.top();
		nfas.pop();
		nfas.top()->unify(a);		
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
                   | ERE_branch ERE_expression		{cout << "CAT"; concatenate();}
                   ;
ERE_expression     : one_char_or_coll_elem_ERE
                   | '^'					{cout << "{ATSTART}"; isLineStart = true;}
                   | '$'					{cout << "{ATEND}"; isLineEnd = true;}
                   | '(' extended_reg_exp ')'	{cout << "PAREN";}
                   | ERE_expression ERE_dupl_symbol
                   ;
one_char_or_coll_elem_ERE  : ORD_CHAR	{cout << "[" <<$1<<"]"; nfas.push(new Nfa($1)); startingAnywhere.push(nfas.top()->getInitial());}
                   | QUOTED_CHAR		{cout << "["<<$1<<"]"; nfas.push(new Nfa($1)); startingAnywhere.push(nfas.top()->getInitial());}
                   | '.'				{cout << "DOT"; nfas.push(new Nfa(State::DOT)); startingAnywhere.push(nfas.top()->getInitial());}
                   | bracket_expression
                   ;
ERE_dupl_symbol    : '*'	{cout << "STAR"; nfas.top()->apply_cardinality(KLEENE_STAR);}
                   | '+'	{cout << "PLUS"; nfas.top()->apply_cardinality(PLUS);}
                   | '?'	{cout << "OPT"; nfas.top()->apply_cardinality(OPTION);}
                   | '{' DUP_COUNT               '}'	{cout << "{"<<$2<<"}"; nfas.top()->apply_cardinality($2, SIMPLE_COUNT);}
                   | '{' DUP_COUNT ','           '}'	{cout << "{"<<$2<<",}"; nfas.top()->apply_cardinality($2, UNBOUNDED);}
                   | '{' DUP_COUNT ',' DUP_COUNT '}'	{cout << "{"<<$2<<","<<$4<<"}"; nfas.top()->apply_cardinality($2, $4);}
                   ;
            
bracket_expression : '[' matching_list    ']'	{cout << "matching";}
               | '[' nonmatching_list ']'	{cout << "non_matching";}
               ;
matching_list  : bracket_list {cout << "{ENDBRACKET}";}
               ;
nonmatching_list : '^' bracket_list
               ;
bracket_list   : follow_list		{cout << "{??}";}
               | follow_list '-'	{cout << "[-]last"; bracket.push_front('-');}
               ;
follow_list    :             expression_term
               | follow_list expression_term
               ;
expression_term : single_expression
               | range_expression
               ;
single_expression : end_range
               | character_class
               /*| equivalence_class*/
               ;
range_expression : start_range end_range
               | start_range '-'	{cout << "[-]"; bracket.push_front('-');}
               ;
start_range    : end_range '-'		{cout << "{RANGE}";}
               ;
end_range      : COLL_ELEM	{cout << "[" << $1 << "]"; bracket.push_front($1);}
               /*| collating_symbol*/
               ;
/*
collating_symbol : Open_dot COLL_ELEM Dot_close
               | Open_dot META_CHAR Dot_close
               ;
equivalence_class : Open_equal COLL_ELEM Equal_close
               ;
*/
character_class : Open_colon class_name Colon_close		{}
               ;

%%

int main(int argc, char** argv) {
	if (argc < 3) {
		cerr << "Usage: grep <regex> <string>" << endl;
		exit(0);
	}
	
	parse_string(argv[1]);
	yyparse();
	cout << endl << nfas.size() << endl;
	
	cout << endl << startingAnywhere.size() << endl;
	
	while ( ! startingAnywhere.empty()) {
		startingAnywhere.top()->setTransition(State::DOT, startingAnywhere.top());
		startingAnywhere.pop();
	}
	
	nfas.top()->print();
	nfas.top()->toDot("nfa.dot");
	nfas.top()->evaluate(argv[2]);
	nfas.top()->eliminate_eps();
	nfas.top()->print();
	Nfa* top = nfas.top();
	
	top->print();
	top->toDot("nfa.dot");
	top->eliminate_eps();
	top->print();
	Dfa* dfa  = new Dfa(top);
	dfa->print();
	dfa->minimise_hopcroft();
	dfa->print();
	dfa->evaluate(argv[2]);
}


