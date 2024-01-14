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
%x COMMENT
DECIMIAL ([1-9][0-9]*|0)
OCTAL (0[0-7]*)
HEX  (0[xX]([0-9]|[a-f]|[A-F])*)
FLOAT (((([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))([eE][+-]?[0-9]+)?)|([0-9]+[eE][+-]?[0-9]+))
HEXFLOAT (0[xX]((([0-9]|[a-f]|[A-F])+(\.)?([0-9]|[a-f]|[A-F])*)|(\.([0-9]|[a-f]|[A-F])+))[pP][+-]?[0-9]+[fFlL]?)
ID [[:alpha:]_][[:alpha:][:digit:]_]*
EOL (\r\n|\n|\r)
WHITE [\t ]
SLCOMMENT \/\/[^\n]*
MLCOMMENT \/\*
/*  Your code here (lab3). */

%%
    /* rules section */
"int" {
    dump_tokens("INT\t%s\n", yytext);
    return INT;
}

"void" {
    dump_tokens("VOID\t%s\n", yytext);
    return VOID;
}

"float" {
    dump_tokens("INT\t%s\n", yytext);
    return FLOAT;
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

"while" {
    dump_tokens("while\t%s\n", yytext);
    return WHILE;
} 

"break" {
    dump_tokens("break\t%s\n", yytext);
    return BREAK;
} 
"continue" {
    dump_tokens("continue\t%s\n", yytext);
    return CONTINUE;
} 
"const" {
    dump_tokens("const\t%s\n", yytext);
    return CONST;
}
"=" {
    dump_tokens("ASSIGN\t%s\n", yytext);
    return ASSIGN;
}

"<" {
    dump_tokens("LESS\t%s\n", yytext);
    return LESS;
}

">"         {
    dump_tokens("GREATER\t%s\n", yytext);
    return GREATER;
}

"!"         {
    dump_tokens("NOT\t%s\n", yytext);
    return NOT;
}
"||"        {
    dump_tokens("LOR\t%s\n", yytext);
    return LOR;
}
"&&"        {
    dump_tokens("LAND\t%s\n", yytext);
    return LAND;
}
"<="        {
    dump_tokens("NGREATER\t%s\n", yytext);
    return NGREATER;
}
">="        {
    dump_tokens("NLESS\t%s\n", yytext);
    return NLESS;
}
"=="        {
    dump_tokens("EQUAL\t%s\n", yytext);
    return EQUAL;
}
"!="        {
    dump_tokens("NEQUAL\t%s\n", yytext);
    return NEQUAL;
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

"%" {
    dump_tokens("MOD\t%s\n", yytext);
    return MOD;
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

","         {
    dump_tokens("COMMA\t%s\n", yytext);
    return COMMA;
}

"[" 	    {
    dump_tokens("LBRACKET\t%s\n", yytext);
    return LBRACKET;
}

"]" 	    {
    dump_tokens("RBRACKET\t%s\n", yytext);
    return RBRACKET;
}

{SLCOMMENT} {}

{MLCOMMENT} {
	BEGIN COMMENT;
}

<COMMENT>[^*\n]*        {}

<COMMENT>"*"+[^*/\n]*    {}

<COMMENT>{EOL} { yylineno++; }
<COMMENT>"*/"        {
	BEGIN INITIAL;
}

{DECIMIAL}  {
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    dump_tokens("INTEGER\t%s\n", yytext,lexeme);
    yylval.strtype = lexeme;
    return INTEGER;
}

{OCTAL}  {
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    dump_tokens("OCTAL\t%s\n", yytext,lexeme);
    yylval.strtype = lexeme;
    return OCTAL;
}

{HEX}  {
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    dump_tokens("HEX\t%s\n", yytext,lexeme);
    yylval.strtype = lexeme;
    return HEX;
}

{ID} {
    char *lexeme;
    dump_tokens("ID\t%s\n", yytext);
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    return ID;
}

{FLOAT} {
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    dump_tokens("FLOAT\t%s\n", yytext,lexeme);
    yylval.strtype = lexeme;
    return FLT;
}

{HEXFLOAT} {
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    dump_tokens("FLOAT\t%s\n", yytext,lexeme);
    yylval.strtype = lexeme;
    return FLT;
}
{EOL} yylineno++;

{WHITE}

    /*  Your code here (lab3). */
%%
/* user code section */