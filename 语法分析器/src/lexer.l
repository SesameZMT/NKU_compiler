%option noyywrap
%option nounput
%option noinput
%top{
    #include <stdarg.h>
    #include "common.h"
    #include "parser.h"
}
%{
    extern dump_type_t dump_type;

    inline void dump_tokens(const char* format, ...){
        va_list args;
        va_start(args, format);
        if (dump_type == TOKENS)
            vfprintf(yyout, format, args);
        va_end(args);
    }

    /* Your code here, if desired (lab3). */
%}

/* definitions section */
HEXADECIMAL 0[xX][0-9a-fA-F]+
OCTAL 0[0-7]+
LINECOMMENT \/\/[^(\r\n|\n|\r)]*
BLOCKCOMMENT ("/*"([^\*]|(\*)*[^\*/])*(\*)*"*/")
DECIMIAL ([1-9][0-9]*|0)
ID [[:alpha:]_][[:alpha:][:digit:]_]*
EOL (\r\n|\n|\r)
WHITE [\t ]
/*  Your code here (lab3). */

%%
    /* rules section */
{LINECOMMENT} yylineno--;
{BLOCKCOMMENT} yylineno--;
"while" {
    dump_tokens("WHILE\t%s\n", yytext);
    return WHILE;
}
"const" {
    dump_tokens("CONST\t%s\n", yytext);
    return CONST;
}
"int" {
    dump_tokens("INT\t%s\n", yytext);
    return INT;
}

"void" {
    dump_tokens("VOID\t%s\n", yytext);
    return VOID;
}

"if" {
    dump_tokens("IF\t%s\n", yytext);
    return IF;
}

"else" {
    dump_tokens("ELSE\t%s\n", yytext);
    return ELSE;
}

"return" {
    dump_tokens("RETURN\t%s\n", yytext);
    return RETURN;
}

"&&" {
    dump_tokens("AND\t%s\n", yytext);
    return AND;
}
"||" {
    dump_tokens("OR\t%s\n", yytext);
    return OR;
}
"!=" {
    dump_tokens("NOTEQUAL\t%s\n", yytext);
    return NOTEQUAL;  
}
"==" {
    dump_tokens("EQUAL\t%s\n", yytext);
    return EQUAL;
}
"<=" {
    dump_tokens("LESSEQ\t%s\n", yytext);
    return LESSEQ;
}
">=" {
    dump_tokens("MOREEQ\t%s\n", yytext);
    return MOREEQ;
}
"!" {
    dump_tokens("ASSIGN\t%s\n", yytext);
    return NOT;  
}

"=" {
    dump_tokens("ASSIGN\t%s\n", yytext);
    return ASSIGN;
}

"%" {
    dump_tokens("MOD\t%s\n", yytext);
    return MOD;
}

"<" {
    dump_tokens("LESS\t%s\n", yytext);
    return LESS;
}

">" {
    dump_tokens("MORE\t%s\n", yytext);
    return MORE;
}
"+" {
    dump_tokens("ADD\t%s\n", yytext);
    return ADD;
}
"-" {
    dump_tokens("SUB\t%s\n", yytext);
    return SUB;
}
"*" {
    dump_tokens("MUL\t%s\n", yytext);
    return MUL;
}
"/" {
    dump_tokens("DIV\t%s\n", yytext);
    return DIV;
}
"," {
    dump_tokens("COMMA\t%s\n", yytext);
    return COMMA;   
}

";" {
    dump_tokens("SEMICOLON\t%s\n", yytext);
    return SEMICOLON;
}

"(" {
    dump_tokens("LPAREN\t%s\n", yytext);
    return LPAREN;
}

")" {
    dump_tokens("RPAREN\t%s\n", yytext);
    return RPAREN;
}

"{" {
    dump_tokens("LBRACE\t%s\n", yytext);
    return LBRACE;
}

"}" {
    dump_tokens("RBRACE\t%s\n", yytext);
    return RBRACE;
}

{HEXADECIMAL} {
    int temp;
    sscanf(yytext, "%x", &temp);
    dump_tokens("HEXADECIMAL\t%s\t%d\n", yytext, temp);
    yylval.itype = temp;
    return INTEGER;
}
{OCTAL} {
    int temp;
    sscanf(yytext, "%o", &temp);
    dump_tokens("OCTAL\t%s\t%d\n", yytext, temp);
    yylval.itype = temp;
    return INTEGER;
}

{DECIMIAL} {
    int decimal;
    decimal = atoi(yytext);
    dump_tokens("DECIMAL\t%s\t%d\n", yytext, decimal);
    yylval.itype = decimal;
    return INTEGER;
}

{ID} {
    char *lexeme;
    dump_tokens("ID\t%s\n", yytext);
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    return ID;
}

{EOL} yylineno++;

{WHITE}

    /*  Your code here (lab3). */
%%
/* user code section */
