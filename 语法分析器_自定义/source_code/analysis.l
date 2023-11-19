%{
	#include <stdio.h>
	#include <stdlib.h>
    #include <string.h>
    #include "ast.h"
    #include "analysis.tab.h"

    int tempTag;
    int getTag(char *);
	int line = 1;
    int column = 1;
    int charnum=0;
    char path[100];

    void clearcolumn();
    void addcolumn(int);
%}

NOTE_HEAD \/\/
digit [0-9]    
octal_digit [0-7]   
hexadecimal_digit [0-9a-fA-F]
identifier_nondigit [a-zA-Z_]

decimal_const [1-9]{digit}*
octal_const (0){octal_digit}*
hexadecimal_const (0x|0X){hexadecimal_digit}*

Identifier ({identifier_nondigit})({identifier_nondigit}|{digit})*

single_Note ({NOTE_HEAD})(.*|" ")+
multiline_Note "/*"[^*/]*"*/"
Note ({single_Note}|{multiline_Note})

%%

"("                         {return '(';}   
")"                         {return ')';}   
"{"                         {return '{';}   
"}"                         {return '}';}   
"["                         {return '[';}   
"]"                         {return ']';}   
","                         {return ',';}   
";"                         {return ';';}   
"+"                         {return '+';}   
"-"                         {return '-';}   
"*"                         {return '*';} 
"/"                         {return '/';} 
"%"                         {return '%';}  
"<"                         {return '<';}  
">"                         {return '>';}   
"!"                         {return '!';} 
"="                         {return '=';}

"int"                       {return INT;}
"continue"                  {return CONTINUE;}
"const"                     {return CONST;}
"else"                      {return ELSE;}
"if"                        {return IF;}
"return"                    {return RETURN;}
"void"                      {return VOID;}
"while"                     {return WHILE;}
"break"                     {return BREAK;}

"<="                        {return LESSEQ;}
">="                        {return GREATEQ;}
"!="                        {return NOTEQ;}
"=="                        {return EQ;}
"&&"                        {return AND;}
"||"                        {return OR;}         

" "                         {  /*no action and no return*/ }
"\t"                        {  /*no action and no return*/ }
"\n"                        {  /*no action and no return*/ }
{Note}                      {  /*no action and no return*/ }

{decimal_const}             |
{octal_const}               |
{hexadecimal_const}         {yylval.number = atoi(yytext); return NUMBER;}

{Identifier}                {strcpy(yylval.strValue, yytext); return ID;}

%%

int yywrap(){
    return 1;
}