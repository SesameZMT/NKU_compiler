%{
	#include<stdio.h>
	#include<stdlib.h>
	int line = 1;
    int column = 1;
    int charnum=0;

    void clearcolumn();
    void addcolumn(int);
%}

VOID "void"
INT "int"
CHAR "char"
IF "if"
WHILE "while"
FOR "for"
BREAK "break"
CONTINUE "continue"
RETURN "return"
SEMICOLON ;
LPAREN \(
RPAREN \)
LBRACE \{
RBRACE \}




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
{octal_const} {printf("line%d:(octal_const, %s)\n",line,yytext);} 
{hexadecimal_const} {printf("line%d:(hexadecimal_const, %s)\n",line,yytext);}
{Identifier} {printf("line%d:(Identifier, %s)\n",line,yytext);}
{SEMICOLON} {printf("line%d:(SEMICOLON, %s)\n",line,yytext);}
{LPAREN} {printf("line%d:(LPAREN, %s)\n",line,yytext);}
{RPAREN} {printf("line%d:(RPAREN, %s)\n",line,yytext);}

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
	yyin=fopen("data.txt","r");
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