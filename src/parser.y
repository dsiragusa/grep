%{
	#include <cstdio>
	#include <iostream>
	#include <stack>
	#include "src/Nfa.h"
	#include "src/Dfa.h"
	#include "src/Tree.h"
	
	using namespace std;
	
	int yylex();
	int yyparse();
	void parse_string(char const *);
	void yyerror(const char *p) { cerr << "\nInvalid regular expression\n"; exit(0);}

	typedef void(*fun_ptr)();
	
	forward_list<int> bracket;
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
	
	void applyRange() {
		int end = bracket.front();
		bracket.pop_front();
		int start = bracket.front();
		
		if (end < start)
			yyerror("");
			
		for (int i = start + 1; i <= end; i++) {
			bracket.push_front(i);
		}
	}
	
	void applyLower() {
		bracket.push_front('a');
		bracket.push_front('z');
		applyRange();
	}
	
	void applyUpper() {
		bracket.push_front('A');
		bracket.push_front('Z');
		applyRange();		
	}
	
	void applyAlpha() {
		applyLower();
		applyUpper();
	}
	
	void applyDigit() {
		bracket.push_front('0');
		bracket.push_front('9');
		applyRange();
	}
	
	void applyAlnum() {
		applyDigit();
		applyAlpha();
	}
	
	void applyAscii() {
		bracket.push_front(0x0);
		bracket.push_front(0x7f);
		applyRange();
	}
	
	void applyBlank() {
		bracket.push_front(' ');
		bracket.push_front('\t');
	}
	
	void applyCntrl() {
		bracket.push_front(0x0);
		bracket.push_front(0x1f);
		applyRange();
		bracket.push_front(0x7f);		
	}
	
	void applyGraph() {
		bracket.push_front(0x21);
		bracket.push_front(0x7e);
		applyRange();
	}
	
	void applyPrint() {
		bracket.push_front(0x20);
		applyGraph();
	}
	
	void applyPunct() {
 		bracket.push_front(0x21);
 		bracket.push_front(0x2f);
		applyRange();
 		bracket.push_front(0x3a);
 		bracket.push_front(0x40);
		applyRange();
 		bracket.push_front(0x5b);
 		bracket.push_front(0x60);
		applyRange();
 		bracket.push_front(0x7b);
 		bracket.push_front(0x7e);
		applyRange();
	}
	
	void applySpace() {
		applyBlank();
 		bracket.push_front('\r');
 		bracket.push_front('\n');
 		bracket.push_front('\v');
 		bracket.push_front('\f');
	}
	
	void applyWord() {
		applyAlnum();
 		bracket.push_front('_');
	}
	
	void applyXdigit() {
		applyDigit();
		bracket.push_front('a');
		bracket.push_front('f');
		applyRange();
		bracket.push_front('A');
		bracket.push_front('F');
		applyRange();
	}
	
	map<string, fun_ptr> mapClassToApply;
	
	void initMapClassToApply() {
		mapClassToApply.emplace("alnum", applyAlnum);
		mapClassToApply.emplace("alpha", applyAlpha);
		mapClassToApply.emplace("ascii", applyAscii);
		mapClassToApply.emplace("blank", applyBlank);
		mapClassToApply.emplace("cntrl", applyCntrl);
		mapClassToApply.emplace("digit", applyDigit);
		mapClassToApply.emplace("graph", applyGraph);
		mapClassToApply.emplace("lower", applyLower);
		mapClassToApply.emplace("print", applyPrint);
		mapClassToApply.emplace("punct", applyPunct);
		mapClassToApply.emplace("space", applySpace);
		mapClassToApply.emplace("upper", applyUpper);
		mapClassToApply.emplace("word", applyWord);
		mapClassToApply.emplace("xdigit", applyXdigit);
	}
	
	void applyClass(char *clazz) {
		string cl_name = string(clazz);
		mapClassToApply.find(cl_name)->second();
		free(clazz);
	}
		
	void printBracket() {
		cout << "\n Bracket:";
		for (char el : bracket) {
			cout << " " << el;
		}
		cout << endl;
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
bracket_expression : '[' matching_list    ']'	{cout << "matching"; printBracket(); pushNfa(new Nfa(bracket, true));}
                   | '[' nonmatching_list ']'	{cout << "non_matching"; printBracket(); pushNfa(new Nfa(bracket, false));}
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
range_expression : start_range end_range {cout << "{ENDRANGE}"; applyRange();}
                 | start_range '-'	{cout << "[-]"; bracket.push_front('-'); cout << "{ENDRANGE}"; applyRange();}
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
character_class : Open_colon class_name Colon_close		{cout << "CLASS: " << $2 << " "; applyClass($2);}
                ;

%%

int main(int argc, char** argv) {
	if (argc < 3) {
		cerr << "Usage: grep <regex> <string>" << endl;
		exit(0);
	}
	
	initMapClassToApply();
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
		
	
	Nfa* top = nfas.top();
	top->toDot("nfa.dot");
	top->print();
	
	top->evaluate(argv[2]);
	
	top->eliminate_eps();
	top->print();
	
	top->toDot("nfa2.dot");
	
	top->evaluate(argv[2]);

	
	Dfa* dfa = new Dfa(top);

	dfa->toDot("dfa.dot");
	dfa->print();
	dfa->minimise_hopcroft();
	
	dfa->toDot("dfa2.dot");
	dfa->evaluate(argv[2]);
	dfa->print();	
}


