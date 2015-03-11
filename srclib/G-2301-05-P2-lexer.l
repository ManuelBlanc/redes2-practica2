%{
#include "G-2301-05-P2-parser.h"

#define YY_INPUT(buf, result, max_size)	\
{                                      	\
    /* Leer del socket */              	\
}                                      	\

%}
extern YYSTYPE yylval;

%option noyywrap
%option always-interactive

%%

[0-9]+    	{ yylval.number = atoi(yytext); return NUMBER;	}
[a-z]     	{ yylval.var = *yytext - 'a'; return VAR;     	}
[-+()*\n=]	{ return *yytext;                             	}
[ \t]     	{                                             	}
.         	{ yyerror("Unknown character");               	}

//message    =  [ ":" prefix SPACE ] command [ params ] crlf
//prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
//command    =  1*letter / 3digit
//params     =  *14( SPACE middle ) [ SPACE ":" trailing ]
           //=/ 14( SPACE middle ) [ SPACE [ ":" ] trailing ]
//
//nospcrlfcl =  %x01-09 / %x0B-0C / %x0E-1F / %x21-39 / %x3B-FF
                //; any octet except NUL, CR, LF, " " and ":"
//middle     =  nospcrlfcl *( ":" / nospcrlfcl )
//trailing   =  *( ":" / " " / nospcrlfcl )
//
//SPACE      =  %x20        ; space character
//crlf       =  %x0D %x0A   ; "carriage return" "linefeed"


%%
