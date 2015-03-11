%{
#include <stdio.h>

void yyerror(char *s) {
  fprintf(stderr, "%s\n", s);
}

int sym[26];
%}

%union {
  int number;
  int var;
};

%type <number> expr
%token <var> VAR
%token <number> NUMBER

%%

start	: expr '\n' { printf("%d\n\n", $1); } start
     	| /* NULL */
     	;

expr	: NUMBER
    	| VAR          	{ $$ = sym[$1];         	}
    	| '-' expr     	{ $$ = -$2;             	}
    	| expr '+' expr	{ $$ = $1 + $3;         	}
    	| expr '-' expr	{ $$ = $1 - $3;         	}
    	| expr '*' expr	{ $$ = $1 * $3;         	}
    	| '(' expr ')' 	{ $$ = $2;              	}
    	| VAR '=' expr 	{ sym[$1] = $3; $$ = $3;	}
    	;
