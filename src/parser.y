%{
	#include <cstdio>
	#include <iostream>
	#include "src/Nfa.h"
	
	using namespace std;
	
	int yylex();
	extern "C" int yyparse();
	extern "C" FILE* yyin;
	
	void yyerror(const char *p) { cerr << "\nInvalid regular expression\n"; }
%}

%union {
	int num;
	char *s;
	char c;
}

%glr-parser

%token<c>  ORD_CHAR QUOTED_CHAR <num>DUP_COUNT
%token<c>    COLL_ELEM /*META_CHAR*/
%token    /*Open_equal Equal_close Open_dot Dot_close*/ Open_colon Colon_close
%token<s>    class_name
%type<num>	ERE_dupl_symbol

%start  extended_reg_exp
%%


extended_reg_exp   :                      ERE_branch
                   | extended_reg_exp '|' ERE_branch
                   ;
ERE_branch         :            ERE_expression
                   | ERE_branch ERE_expression
                   ;
ERE_expression     : one_char_or_coll_elem_ERE
                   | '^'	{cout << "^";}
                   | '$'	{cout << "$";}
                   | '(' extended_reg_exp ')'	{cout << "PAREN";}
                   | ERE_expression ERE_dupl_symbol
                   ;
one_char_or_coll_elem_ERE  : ORD_CHAR	{cout << "[" <<$1<<"]";}
                   | QUOTED_CHAR		{cout << "["<<$1<<"]";}
                   | '.'				{cout << "DOT";}
                   | bracket_expression
                   ;
ERE_dupl_symbol    : '*'	{cout << "STAR";}
                   | '+'	{cout << "PLUS";}
                   | '?'	{cout << "OPT";}
                   | '{' DUP_COUNT               '}'	{cout << "{"<<$2<<"}";}
                   | '{' DUP_COUNT ','           '}'	{cout << "{"<<$2<<",}";}
                   | '{' DUP_COUNT ',' DUP_COUNT '}'	{cout << "{"<<$2<<","<<$4<<"}";}
                   ;
            
bracket_expression : '[' matching_list    ']'	{cout << "matching";}
               | '[' nonmatching_list ']'	{cout << "non_matching";}
               ;
matching_list  : bracket_list
               ;
nonmatching_list : '^' bracket_list
               ;
bracket_list   : follow_list
               | follow_list '-'	{cout << "[-]";}
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
               | start_range '-'	{cout << "[-]";}
               ;
start_range    : end_range '-'
               ;
end_range      : COLL_ELEM	{cout << "[" << $1 << "]";}
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
	// determinisation et obtenier un dfa pour validation
	yyin = stdin;

	do {
		yyparse();
		
	} while (!feof(yyin));
	
}


