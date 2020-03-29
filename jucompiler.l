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

  bool lex_verbose_flag = false;
  bool are_syntax_error = false;
  
  bool valid_line = true;
  int col_eof = 1, line_eof = 1;
  int line = 1;
  int column = 1;
  int comment_line = 1;
  int comment_column = 1;
  int string_line = 1;
  int string_column = 1;
  int error_column = 1;

  char* string = NULL;
  int semicolon = 1;
  
  void print(char* str){
    if(lex_verbose_flag){
      printf("%s", str);
    }
  }

  void print_intlit(char* type, char* str) {
    if(lex_verbose_flag) {
      printf("%s(%s)\n", type, str);
    }
  }

  void print_strlit(char* type, char* str) {
    if(lex_verbose_flag) {
      printf("%s(\"%s\")\n", type, str);
    }
  }
%}

  /* Definitions */
int           [0-9]+
char          [^(\r\n\")]
letter        [a-z]|[A-Z]|\$|_
int_lit       ([1-9]+({int}|_)*{int})|{int}
exponent      (e|E)(\+|\-)?(({int}_*)|{int_lit})*

EXCEPTIONS        ([^\""\\"\n\r])
ACCEPTESCAPES     (\\(f|n|r|t|\"))
STRLIT            ({EXCEPTIONS}|{ACCEPTESCAPES}|"\\\\"|"("|")")*

%X MULTILINE_COMMENT
%X INLINE_COMMENT
%X ERRORS
%X IN_STRING
%X ERROR_STRING

%%
  /* Rules */

"/*" {
  BEGIN(MULTILINE_COMMENT);
}

<MULTILINE_COMMENT>"*/" {
  BEGIN(0);
}

<MULTILINE_COMMENT>\n {

}

<MULTILINE_COMMENT>. {
  
}

"//" {
  BEGIN(INLINE_COMMENT);
}

<INLINE_COMMENT>\n {
  BEGIN(0);
}

<INLINE_COMMENT>. {

}

\" {
  if(string == NULL) {
    string = (char*) malloc(sizeof(char));
  }
  string[0] = '\0';
  BEGIN(IN_STRING);
}

<IN_STRING>\\[^fnrt\\\n\"] {
  printf("invalid escape sequence (%s)\n", yytext);
  BEGIN(ERROR_STRING);
}

<IN_STRING>{STRLIT} {
  string = (char*) malloc(yyleng*sizeof(char));
  strcpy(string, yytext);
}

<IN_STRING>\" {
  print_strlit("STRLIT", string);
  BEGIN(0);
}

<IN_STRING>\n {
  printf("unterminated string literal\n");
  BEGIN(0);
}

<IN_STRING>\\ {
  printf("invalid escape sequence (%s)\n", yytext);
  BEGIN(ERROR_STRING);
}

<IN_STRING><<EOF>> {
  printf("unterminated string literal\n");
  return 0;
}

<IN_STRING>. {

}

<ERROR_STRING>\\[^fnrt\\\n\"] {
  printf("invalid escape sequence (%s)\n", yytext);
  BEGIN(0);
}

<ERROR_STRING>{STRLIT} {

}

<ERROR_STRING>\" {
  BEGIN(0);
}

<ERROR_STRING>\n {
  printf("unterminated string literal\n");
  BEGIN(0);
}

<ERROR_STRING>\\ {
  printf("invalid escape sequence (%s)\n", yytext);
  BEGIN(0);
}

<ERROR_STRING><<EOF>> {
  printf("unterminated string literal\n");
  return 0;
}

<ERROR_STRING>. {

}

";"                                                                                        { print("SEMICOLON\n"); }
","                                                                                        { print("COMMA\n");     }
">"                                                                                        { print("GT\n");        }
"%"                                                                                        { print("MOD\n");       }
"!"                                                                                        { print("NOT\n");       }
"&""&"                                                                                       { print("AND\n");       }
"="                                                                                       { print("ASSIGN\n");    }
"*"                                                                                       { print("STAR\n");      }
"/"                                                                                       { print("DIV\n");       }
"{"                                                                                       { print("LBRACE\n");    }
"}"                                                                                       { print("RBRACE\n");    }
"("                                                                                       { print("LPAR\n");      }
")"                                                                                       { print("RPAR\n");      }
"["                                                                                       { print("LSQ\n");       }
"]"                                                                                       { print("RSQ\n");       }
"<"                                                                                       { print("LT\n");        }
"-"                                                                                       { print("MINUS\n");     }
"+"                                                                                       { print("PLUS\n");      }
"!""="                                                                                       { print("NE\n");        }
"-"">"                                                                                       { print("ARROW\n");     }
"^"                                                                                       { print("XOR\n");       }
if                                                                                       { print("IF\n");        }
int                                                                                      { print("INT\n");       }
">""="                                                                                      { print("GE\n");        }
"=""="                                                                                     { print("EQ\n");        }
"<""="                                                                                     { print("LE\n");        }
"|""|"                                                                                     { print("OR\n");        }
"<""<"                                                                                     { print("LSHIFT\n");    }
">"">"                                                                                     { print("RSHIFT\n");    }
else                                                                                     { print("ELSE\n");      }
void                                                                                     { print("VOID\n");      }
class                                                                                    { print("CLASS\n");     }
while                                                                                    { print("WHILE\n");     }
public                                                                                   { print("PUBLIC\n");    }
double                                                                                   { print("DOUBLE\n");    }
static                                                                                   { print("STATIC\n");    }
return                                                                                   { print("RETURN\n");    }
String                                                                                   { print("STRING\n");    }
boolean                                                                                  { print("BOOL\n");      }
"."length                                                                                 { print("DOTLENGTH\n"); }
true|false                                                                               { print_intlit("BOOLIT", yytext);    }
Integer"."parseInt                                                                        { print("PARSEINT\n");  }
System"."out"."print                                                                       { print("PRINT\n");     }
"\+\+"|"\-\-"|null|Integer|System|abstract|continue|for|new|switch|assert|default|package|synchronized|do|goto|private|this|break|implements|protected|throw|byte|import|throws|case|enum|instanceof|transient|catch|extends|short|try|char|final|interface|finally|long|strictfp|volatile|const|float|native|super                                                        { print_intlit("RESERVED", yytext);  }

{int_lit}                                                                                { print_intlit("INTLIT", yytext);    }
({int_lit}(\.({int_lit}{exponent}?)?|{exponent}))|({int_lit}?\.0*(0(_)*{int_lit})*{int_lit}?{exponent}?)    { print_intlit("REALLIT", yytext);   }
{letter}+({letter}|{int})*                                                               { print_intlit("ID", yytext);        }

" "|\t|\n|                                                                 {                        }

. {
  printf("illegal character (%s)\n", yytext);
}

%%
  /* Subroutines */
int main(int argc, char **argv) {
  if(argc == 1) {
    
  } else if(strcmp(argv[1], "-l") == 0) {
    lex_verbose_flag = true;
  }
  yylex();
  return 0;
}

int yywrap() {
  return 1;
}
