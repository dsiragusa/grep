%{
	#include "parser.tab.h"
/*{SPEC}	{printf("SPEC_CHAR\n"); return SPEC_CHAR;}
SPEC [[\()|$^.{*+?]*/
%}

%option noyywrap

QUOTED \\[[\()|$^.{*+?]
ORD [^[\()|$^.{*+?]
META (\^|\-|\])

%%
\^	{return L_ANCHOR;}
\$	{return R_ANCHOR;}

\n		{printf("\n");}
{ORD}	{yylval.s = yytext[0]; return ORD_CHAR;}
{QUOTED}	{return QUOTED_CHAR;}
\*		{return DUP_STAR;}
\+		{return DUP_PLUS;}
\?		{return DUP_OPT;}
%%

