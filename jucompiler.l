%{
/*  ****************************  */
/*  Authors:                      */
/*    Diogo Ferrer    2017247199  */
/*    Guilherme Cruz  2016xxxxxx  */
/*  ****************************  */
%}

%{
  #include <stdlib.h>
  #include <stdbool.h>
  #include <string.h>
  #include "y.tab.h"

  int line = 1;
  int column = 1;

  int comment_line = 1;
  int comment_column = 1;

  int string_line = 1;
  int string_column = 1;
  
  int buffer_size;
  int string_size;
  char *string;

  bool print_tokens = false;
  bool just_lexical = false;
  bool string_error = false;

  void yyerror(char *s);
  char* build_str(char *s);
%}

  /* Definitions */
line_end      \r\n|\n|\r
id            [a-zA-Z_$][a-zA-Z_$0-9]*
int_lit       0|[1-9]+("_"*[0-9]+)*
digits        [0-9]+("_"*[0-9]+)*
exponent      [eE][+-]?{digits}
real_lit      {digits}"."{digits}?{exponent}?|"."{digits}{exponent}?|{digits}{exponent}

%X MULTILINE_COMMENT
%X INLINE_COMMENT
%X IN_STRING

%%
  /* Rules */

"/*" {
  BEGIN(MULTILINE_COMMENT);
  comment_line = line;
  comment_column = column;
  column += yyleng;
}

<MULTILINE_COMMENT>. {
  column += yyleng;
}

<MULTILINE_COMMENT>{line_end} {
  line++;
  column = 1;
}

<MULTILINE_COMMENT><<EOF>> {
  printf("Line %d, col %d: unterminated comment\n", comment_line, comment_column);
  yyterminate();
}

<MULTILINE_COMMENT>"*/" {
  BEGIN(0);
  column += yyleng;
}

"//" {
  BEGIN(INLINE_COMMENT);
  column += yyleng;
}

<INLINE_COMMENT>{line_end} {
  BEGIN(0);
  line++;
  column = 1;
}

<INLINE_COMMENT>. {
  column += yyleng;
}

\" {
  BEGIN(IN_STRING);
  string = malloc(10 * sizeof(char));
  buffer_size = 10;
  string[0] = '\0';
  string_size = 0;
  string_error = false;
  string_line = line;
  string_column = column;
  column += yyleng;
}

<IN_STRING>\" {
  BEGIN(0);
  column += yyleng;
  yytext = build_str(string);
  yyleng = strlen(yytext);
  if(print_tokens && !string_error) {
    printf("STRLIT(%s)\n", yytext);
  }
  yylval.string = (char*) strdup(yytext);
  free(string);
  if(!just_lexical && !string_error) {
    return STRLIT;
  }
}

<IN_STRING>\\[fnrt\\\"] {
  char *token = yytext;
  string_size += 2;
  if(string_size >= buffer_size - 1) {
    string = realloc(string, (buffer_size + 10) * sizeof(char));
    buffer_size += 10;
  }
  strcat(string, token);
  column += yyleng;
}

<IN_STRING><<EOF>> {
  printf("Line %d, col %d: unterminated string literal\n", string_line, string_column);
  free(string);
  yyterminate();
}

<IN_STRING>{line_end} {
  printf("Line %d, col %d: unterminated string literal\n", string_line, string_column);
  BEGIN(0);
  free(string);
  line++;
  column = 1;
}

<IN_STRING>\\[^\n\r]? {
  printf("Line %d, col %d: invalid escape sequence (%s)\n", line, column, yytext);
  string_error = true;
  column += yyleng;
}

<IN_STRING>. {
  char* token = yytext;
  string_size++;
  if(string_size >= buffer_size - 1) {
    string = realloc(string, (buffer_size + 10) * sizeof(char));
    buffer_size += 10;
  } 
  strcat(string, token);
  column += yyleng;
}

";" {
  if(print_tokens)
    printf("SEMICOLON\n");
  column += yyleng;
  if(!just_lexical)
    return SEMICOLON;
}

"," {
  if(print_tokens)
    printf("COMMA\n");
  column += yyleng;
  if(!just_lexical)
    return COMMA;
}

">" {
  if(print_tokens)
    printf("GT\n");
  column += yyleng;
  if(!just_lexical)
    return GT;
}

"%" {
  if(print_tokens)
    printf("MOD\n");
  column += yyleng;
  if(!just_lexical)
    return MOD;
}

"!" {
  if(print_tokens)
    printf("NOT\n");
  column += yyleng;
  if(!just_lexical)
    return NOT;
}

"&""&" {
  if(print_tokens)
    printf("AND\n");
  column += yyleng;
  if(!just_lexical)
    return AND;
}

"=" {
  if(print_tokens)
    printf("ASSIGN\n");
  column += yyleng;
  if(!just_lexical)
    return ASSIGN;
}

"*" {
  if(print_tokens)
    printf("STAR\n");
  column += yyleng;
  if(!just_lexical)
    return STAR;
}

"/" {
  if(print_tokens)
    printf("DIV\n");
  column += yyleng;
  if(!just_lexical)
    return DIV;
}

"{" {
  if(print_tokens)
    printf("LBRACE\n");
  column += yyleng;
  if(!just_lexical)
    return LBRACE;
}

"}" {
  if(print_tokens)
    printf("RBRACE\n");
  column += yyleng;
  if(!just_lexical)
    return RBRACE;
}

"(" {
  if(print_tokens)
    printf("LPAR\n");
  column += yyleng;
  if(!just_lexical)
    return LPAR;
}

")" {
  if(print_tokens)
    printf("RPAR\n");
  column += yyleng;
  if(!just_lexical)
    return RPAR;
}

"[" {
  if(print_tokens)
    printf("LSQ\n");
  column += yyleng;
  if(!just_lexical)
    return LSQ;
}

"]" {
  if(print_tokens)
    printf("RSQ\n");
  column += yyleng;
  if(!just_lexical)
    return RSQ;
}

"<" {
  if(print_tokens)
    printf("LT\n");
  column += yyleng;
  if(!just_lexical)
    return LT;
}

"-" {
  if(print_tokens)
    printf("MINUS\n");
  column += yyleng;
  if(!just_lexical)
    return MINUS;
}

"+" {
  if(print_tokens)
    printf("PLUS\n");
  column += yyleng;
  if(!just_lexical)
    return PLUS;
}

"!""=" {
  if(print_tokens)
    printf("NE\n");
  column += yyleng;
  if(!just_lexical)
    return NE;
}

"-"">" {
  if(print_tokens)
    printf("ARROW\n");
  column += yyleng;
}

"^" {
  if(print_tokens)
    printf("XOR\n");
  column += yyleng;
}

if {
  if(print_tokens)
    printf("IF\n");
  column += yyleng;
  if(!just_lexical)
    return IF;
}

int {
  if(print_tokens)
    printf("INT\n");
  column += yyleng;
  if(!just_lexical)
    return INT;
}

">""=" {
  if(print_tokens)
    printf("GE\n");
  column += yyleng;
  if(!just_lexical)
    return GE;
}

"=""=" {
  if(print_tokens)
    printf("EQ\n");
  column += yyleng;
  if(!just_lexical)
    return EQ;
}

"<""=" {
  if(print_tokens)
    printf("LE\n");
  column += yyleng;
  if(!just_lexical)
    return LE;
}

"|""|" {
  if(print_tokens)
    printf("OR\n");
  column += yyleng;
  if(!just_lexical)
    return OR;
}

"<""<" {
  if(print_tokens)
    printf("LSHIFT\n");
  column += yyleng;
}
">"">" {
  if(print_tokens)
    printf("RSHIFT\n");
  column += yyleng;
}

else {
  if(print_tokens)
    printf("ELSE\n");
  column += yyleng;
  if(!just_lexical)
    return ELSE;
}

void {
  if(print_tokens)
    printf("VOID\n");
  column += yyleng;
  if(!just_lexical)
    return VOID;
}

class {
  if(print_tokens)
    printf("CLASS\n");
  column += yyleng;
  if(!just_lexical)
    return CLASS;
}

while {
  if(print_tokens)
    printf("WHILE\n");
  column += yyleng;
  if(!just_lexical)
    return WHILE;
}

public {
  if(print_tokens)
    printf("PUBLIC\n");
  column += yyleng;
  if(!just_lexical)
    return PUBLIC;
}

double {
  if(print_tokens)
    printf("DOUBLE\n");
  column += yyleng;
  if(!just_lexical)
    return DOUBLE;
}

static {
  if(print_tokens)
    printf("STATIC\n");
  column += yyleng;
  if(!just_lexical)
    return STATIC;
}

return {
  if(print_tokens)
    printf("RETURN\n");
  column += yyleng;
  if(!just_lexical)
    return RETURN;
}

String {
  if(print_tokens)
    printf("STRING\n");
  column += yyleng;
  if(!just_lexical)
    return STRING;
}

boolean {
  if(print_tokens)
    printf("BOOL\n");
  column += yyleng;
  if(!just_lexical)
    return BOOL;
}

"."length {
  if(print_tokens)
    printf("DOTLENGTH\n");
  column += yyleng;
  if(!just_lexical)
    return DOTLENGTH;
}

true|false {
  if(print_tokens)
    printf("BOOLLIT(%s)\n", yytext);
  column += yyleng;
  if(!just_lexical)
    return BOOLLIT;
}

Integer"."parseInt {
  if(print_tokens)
    printf("PARSEINT\n");
  column += yyleng;
  if(!just_lexical)
    return PARSEINT;
}

System"."out"."print {
  if(print_tokens)
    printf("PRINT\n");
  column += yyleng;
  if(!just_lexical)
    return PRINT;
}

"\+\+"|"\-\-"|null|Integer|System|abstract|continue|for|new|switch|assert|default|package|synchronized|do|goto|private|this|break|implements|protected|throw|byte|import|throws|case|enum|instanceof|transient|catch|extends|short|try|char|final|interface|finally|long|strictfp|volatile|const|float|native|super {
  if(print_tokens)
    printf("RESERVED(%s)\n", yytext);
  column += yyleng;
  yylval.string = (char*) strdup(yytext);
  if(!just_lexical)
    return RESERVED;
}

{int_lit} {
  if(print_tokens)
    printf("INTLIT(%s)\n", yytext);
  column += yyleng;
  yylval.string = (char*) strdup(yytext);
  if(!just_lexical)
    return INTLIT;
}

{real_lit} {
  if(print_tokens)
    printf("REALLIT(%s)\n", yytext);
  column += yyleng;
  yylval.string = (char*) strdup(yytext);
  if(!just_lexical)
    return REALLIT;
}

{id} {
  if(print_tokens)
    printf("ID(%s)\n", yytext);
  column += yyleng;
  yylval.string = (char*) strdup(yytext);
  if(!just_lexical)
    return ID;
}

{line_end} {
  line++;
  column = 1;
}

" "|\t|\f {
  column += yyleng;
}

. {
  printf("Line %d, col %d: illegal character (%s)\n", line, column, yytext);
  column += yyleng;
}

%%
  /* Subroutines */

void yyerror (char *s) {
    if(!strcmp(yytext, "")) {
        yyleng = 0;
    }
    printf ("Line %d, col %d: %s: %s\n", line, column - yyleng, s, yytext);
}

char* build_str(char *s) {
  char *str = malloc(strlen(s) + 3);
  if(!str) {
    return NULL;
  }

  str[0] = '"';
  str[1] = '\0';
  strcat(str, s);
  strcat(str, "\"");
  return str;
}

int yywrap() {
  return 1;
}
