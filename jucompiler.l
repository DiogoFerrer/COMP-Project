%{
/*  ****************************  */
/*  Authors:                      */
/*    Diogo Ferrer    2017247199  */
/*    Guilherme Cruz  2016xxxxxx  */
/*  ****************************  */
%}

  /* Definitions */
int           [0-9]+
char          [^\r\n\"]
letter        [a-z]|[A-Z]|\$|_
int_lit       ([1-9]+({int}|_)*{int})|{int}
exponent      (e|E)(\+|\-)?{int_lit}+
escape_seq    \f|\n|\r|\t|\\|\"

%Start ERROR ALL
%%
  /* Rules */
<ALL>;                                                                                        { printf("SEMICOLON\n"); }
<ALL>,                                                                                        { printf("COMMA\n");     }
<ALL>>                                                                                        { printf("GT\n");        }
<ALL>%                                                                                        { printf("MOD\n");       }
<ALL>!                                                                                        { printf("NOT\n");       }
<ALL>&&                                                                                       { printf("AND\n");       }
<ALL>\=                                                                                       { printf("ASSIGN\n");    }
<ALL>\*                                                                                       { printf("STAR\n");      }
<ALL>\/                                                                                       { printf("DIV\n");       }
<ALL>\{                                                                                       { printf("LBRACE\n");    }
<ALL>\}                                                                                       { printf("RBRACE\n");    }
<ALL>\(                                                                                       { printf("LPAR\n");      }
<ALL>\)                                                                                       { printf("RPAR\n");      }
<ALL>\[                                                                                       { printf("LSQ\n");       }
<ALL>\]                                                                                       { printf("RSQ\n");       }
<ALL>\<                                                                                       { printf("LT\n");        }
<ALL>\-                                                                                       { printf("MINUS\n");     }
<ALL>\+                                                                                       { printf("PLUS\n");      }
<ALL>!=                                                                                       { printf("NE\n");        }
<ALL>->                                                                                       { printf("ARROW\n");     }
<ALL>\^                                                                                       { printf("XOR\n");       }
<ALL>if                                                                                       { printf("IF\n");        }
<ALL>int                                                                                      { printf("INT\n");       }
<ALL>>\=                                                                                      { printf("GE\n");        }
<ALL>\=\=                                                                                     { printf("EQ\n");        }
<ALL>\<\=                                                                                     { printf("LE\n");        }
<ALL>\|\|                                                                                     { printf("OR\n");        }
<ALL>\<\<                                                                                     { printf("LSHIFT\n");    }
<ALL>\>\>                                                                                     { printf("RSHIFT\n");    }
<ALL>else                                                                                     { printf("ELSE\n");      }
<ALL>void                                                                                     { printf("VOID\n");      }
<ALL>class                                                                                    { printf("CLASS\n");     }
<ALL>while                                                                                    { printf("WHILE\n");     }
<ALL>public                                                                                   { printf("PUBLIC\n");    }
<ALL>double                                                                                   { printf("DOUBLE\n");    }
<ALL>static                                                                                   { printf("STATIC\n");    }
<ALL>return                                                                                   { printf("RETURN\n");    }
<ALL>String                                                                                   { printf("STRING\n");    }
<ALL>boolean                                                                                  { printf("BOOL\n");      }
<ALL>\.length                                                                                 { printf("DOTLENGTH\n"); }
<ALL>true|false                                                                               { printf("BOOLIT\n");    }
<ALL>Integer\.parseInt                                                                        { printf("PARSEINT\n");  }
<ALL>System\.out\.print                                                                       { printf("PRINT\n");     }
<ALL>"\+\+"|"\-\-"|null|Integer|System                                                        { printf("RESERVED\n");  }

<ALL>{int_lit}                                                                                { printf("INTLIT\n");    }
<ALL>\"({char}|{escape_seq})*\"                                                               { printf("STRLIT\n");    }
<ALL>{letter}+({letter}|{int})*                                                               { printf("ID(%s)\n", yytext);        }
<ALL>({int_lit}(\.({int_lit}{exponent}?)?|{exponent}))|({int_lit}?\.{int_lit}?{exponent}?)    { printf("REALLIT\n");   }

<ALL>" "|\t|\n|"//".*                                                                         {                        }

<ERROR>(.|\n)*    {         }
%%
  /* Subroutines */
int main(int argc, char **argv) {
  if(argc == 1) {
    BEGIN ERROR;
  } else if(strcmp(argv[1], "-l") == 0) {
    BEGIN ALL;
  }
  yylex();
  return 0;
}

int yywrap() {
  return 1;
}
