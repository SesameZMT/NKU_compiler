%{
	#include<stdio.h>
	#include<stdlib.h>
	int line = 1;
    int column = 1;
    int charnum=0;
    char path[100];

    void clearcolumn();
    void addcolumn(int);
%}

VOID void
INT int
CHAR char
IF if
WHILE while
FOR for
BREAK break
CONTINUE continue
RETURN return
SEMICOLON ;
COMMA ,
LPAREN \(
RPAREN \)
LBRACE \{
RBRACE \}
CMPEQUAL \=\=
EQUAL \=
ADD \+
SUB \-
MUL \*
DIV \/
SMALLER \<
BIGGER \>




digit [0-9]    
octal_digit [0-7]   
hexadecimal_digit [0-9a-fA-F]
identifier_nondigit [a-zA-Z_]

decimal_const [1-9]{digit}*
octal_const (0){octal_digit}*
hexadecimal_const (0x|0X){hexadecimal_digit}*

Identifier ({identifier_nondigit})({identifier_nondigit}|{digit})*




%%

{decimal_const} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d%-20s\n","decimal_const",yytext,line,column,yytext);
    charnum = yyleng;
    } 
{octal_const} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d%-20s\n","octal_const",yytext,line,column,yytext);
    charnum = yyleng;
    } 
{hexadecimal_const} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d%-20s\n","hexadecimal_const",yytext,line,column,yytext);
    charnum = yyleng;
    }
{VOID} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","VOID",yytext,line,column);
    charnum = yyleng;
    }
{INT} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","INT",yytext,line,column);
    charnum = yyleng;
    }
{CHAR} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","CHAR",yytext,line,column);
    charnum = yyleng;
    }
{IF} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","IF",yytext,line,column);
    charnum = yyleng;
    }
{WHILE} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","WHILE",yytext,line,column);
    charnum = yyleng;
    }
{FOR} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","FOR",yytext,line,column);
    charnum = yyleng;
    }
{BREAK} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","BREAK",yytext,line,column);
    charnum = yyleng;
    }
{CONTINUE} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","CONTINUE",yytext,line,column);
    charnum = yyleng;
    }
{RETURN} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","RETURN",yytext,line,column);
    charnum = yyleng;
    }
{Identifier} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d%-20p\n","Identifier",yytext,line,column,yytext);
    charnum = yyleng;
    }
{SEMICOLON} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","SEMICOLON",yytext,line,column);
    charnum = yyleng;
    }
{COMMA} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","COMMA",yytext,line,column);
    charnum = yyleng;
    }
{LPAREN} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","LPAREN",yytext,line,column);
    charnum = yyleng;
    }
{RPAREN} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","RPAREN",yytext,line,column);
    charnum = yyleng;
    }
{LBRACE} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","LBRACE",yytext,line,column);
    charnum = yyleng;
    }
{RBRACE} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","RBRACE",yytext,line,column);
    charnum = yyleng;
    }
{CMPEQUAL} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","CMPEQUAL",yytext,line,column);
    charnum = yyleng;
    }
{EQUAL} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","EQUAL",yytext,line,column);
    charnum = yyleng;
    }
{ADD} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","ADD",yytext,line,column);
    charnum = yyleng;
    }
{SUB} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","SUB",yytext,line,column);
    charnum = yyleng;
    }
{MUL} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","MUL",yytext,line,column);
    charnum = yyleng;
    }
{DIV} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","DIV",yytext,line,column);
    charnum = yyleng;
    }
{SMALLER} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","SMALLER",yytext,line,column);
    charnum = yyleng;
    }
{BIGGER} {
    addcolumn(charnum);
    printf("%-20s%-20s%-10d%-10d\n","BIGGER",yytext,line,column);
    charnum = yyleng;
    }


\n {
    ++line;
    clearcolumn();
    }

. {
    addcolumn(1);
    }
[ \t]+ {}

%%
int main(){
    scanf("%s",path);
	yyin=fopen(path,"r");
	yylex();
	return 0;
}
int yywrap(){
	return 1;
}
void clearcolumn(){
    column = 1;
    charnum = 0;
}
void addcolumn(int temp){
    column += temp;
}