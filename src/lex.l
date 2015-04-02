%{
	#include "parser.tab.h"
%}

%x DUP
%x BRA_NEG
%x BRA_FIRST
%x BRA
%x BRA_COL

QUOTED \\[[\()|$^.{*+?]
ORD [^[\()|$^.{*+?]

%%
\n
\{		{BEGIN DUP; return '{';}
<DUP>[0-9]+	{yylval.num = atoi(yytext); return DUP_COUNT;}
<DUP>,		{return ',';}
<DUP>[ \t]+
<DUP>\}	{BEGIN 0; return '}';}

\[		{BEGIN BRA_NEG; return '[';}
<BRA_NEG>\^	{BEGIN BRA_FIRST; return '^';}
<BRA_NEG>-	{BEGIN BRA; yylval.c = '-'; return COLL_ELEM;}
<BRA_NEG>\]	{BEGIN BRA; yylval.c = ']'; return COLL_ELEM;}
<BRA_NEG>\[:	{BEGIN BRA_COL; return Open_colon;}
<BRA_NEG>.	{BEGIN BRA; yylval.c = yytext[0]; return COLL_ELEM;}

<BRA_FIRST>-	{BEGIN BRA; yylval.c = '-'; return COLL_ELEM;}
<BRA_FIRST>\]	{BEGIN BRA; yylval.c = ']'; return COLL_ELEM;}
<BRA_FIRST>\[:	{BEGIN BRA_COL; return Open_colon;}
<BRA_FIRST>.	{BEGIN BRA; yylval.c = yytext[0]; return COLL_ELEM;}

<BRA_COL>(alnum|alpha|ascii|blank|cntrl|digit|graph|lower|print|punct|space|upper|word|xdigit)	{yylval.s = yytext; return class_name;}
<BRA_COL>:]	{BEGIN BRA; return Colon_close;}

<BRA>-	{return '-';}
<BRA>\]		{BEGIN 0; return ']';}
<BRA>\[:	{BEGIN BRA_COL; return Open_colon;}

<BRA>.	{yylval.c = yytext[0]; return COLL_ELEM;}

{ORD}	{yylval.c = yytext[0]; return ORD_CHAR;}
{QUOTED}	{yylval.c = yytext[0]; return QUOTED_CHAR;}
.		{return yytext[0];}
%%

