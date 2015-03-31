%{
	#include <cstdio>
	#include <iostream>
	#include "src/Nfa.h"
	
	using namespace std;
	
	int yylex();
	extern "C" int yyparse();
	extern "C" FILE* yyin;
	
	extern "C" int yyerror(char *p) { cerr << "Error!" << endl; }
	list<Nfa *> nfa_stack;
	list<char> op_stack;
%}

%union {
    int s;
}

%token<s>  ORD_CHAR QUOTED_CHAR DUP_COUNT DUP_STAR DUP_PLUS DUP_OPT
%token<s>    L_ANCHOR R_ANCHOR
%token    COLL_ELEM_SINGLE COLL_ELEM_MULTI META_CHAR
%token    Open_equal Equal_close Open_dot Dot_close Open_colon Colon_close
%token    class_name

%start  extended_reg_exp
%%


extended_reg_exp   :                      ERE_branch
                   | extended_reg_exp '|' ERE_branch
                   ;
ERE_branch         :            ERE_expression
                   | ERE_branch ERE_expression
                   ;
ERE_expression     : one_char_or_coll_elem_ERE
                   | L_ANCHOR	{op_stack.push_back('^');}
                   | R_ANCHOR	{op_stack.push_back('$');}
                   | '(' extended_reg_exp ')'
                   | ERE_expression ERE_dupl_symbol
                   ;
one_char_or_coll_elem_ERE  : ORD_CHAR {nfa_stack.push_back(new Nfa($1));}
                   | QUOTED_CHAR
                   | '.'
                   ;
                   /*
                   | bracket_expression
                   ;
                   */
ERE_dupl_symbol    : DUP_STAR	{nfa_stack.back()->apply_cardinality(KLEENE_STAR);}
                   | DUP_PLUS	{nfa_stack.back()->apply_cardinality(PLUS);}
                   | DUP_OPT	{nfa_stack.back()->apply_cardinality(OPTION);}
                   | '{' DUP_COUNT               '}'	{nfa_stack.back()->apply_cardinality(1, SIMPLE_COUNT);}
                   | '{' DUP_COUNT ','           '}'	{nfa_stack.back()->apply_cardinality(1, UNBOUNDED);}
                   | '{' DUP_COUNT ',' DUP_COUNT '}'	{nfa_stack.back()->apply_cardinality(1, 2);}
                   ;
/*                   
bracket_expression : '[' matching_list ']'
               | '[' nonmatching_list ']'
               ;
matching_list  : bracket_list
               ;
nonmatching_list : '^' bracket_list
               ;
bracket_list   : follow_list
               | follow_list '-'
               ;
follow_list    :             expression_term
               | follow_list expression_term
               ;
expression_term : single_expression
               | range_expression
               ;
single_expression : end_range
               | character_class
               | equivalence_class
               ;
range_expression : start_range end_range
               | start_range '-'
               ;
start_range    : end_range '-'
               ;
end_range      : COLL_ELEM_SINGLE
               | collating_symbol
               ;
collating_symbol : Open_dot COLL_ELEM_SINGLE Dot_close
               | Open_dot COLL_ELEM_MULTI Dot_close
               | Open_dot META_CHAR Dot_close
               ;
equivalence_class : Open_equal COLL_ELEM_SINGLE Equal_close
               | Open_equal COLL_ELEM_MULTI Equal_close
               ;
character_class : Open_colon class_name Colon_close
               ;
*/
%%

int main(int argc, char** argv) {
	// determinisation et obtenier un dfa pour validation
	yyin = stdin;

	do {
		yyparse();
		
		for (auto au : nfa_stack) {
			au->print();
			au->evaluate(argv[1]);
		}
	} while (!feof(yyin));
	
}


