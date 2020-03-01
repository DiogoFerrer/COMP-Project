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
int_lit       [1-9]+({int}|_)*{int}
exponent      (e|E)(\+|\-)?{int_lit}+
escape_seq    \f|\n|\r|\t|\\|\"
%%
  /* Rules */
;                                                                                        { printf("SEMICOLON\n"); }
,                                                                                        { printf("COMMA\n");     }
>                                                                                        { printf("GT\n");        }
%                                                                                        { printf("MOD\n");       }
!                                                                                        { printf("NOT\n");       }
&&                                                                                       { printf("AND\n");       }
\=                                                                                       { printf("ASSIGN\n");    }
\*                                                                                       { printf("STAR\n");      }
\/                                                                                       { printf("DIV\n");       }
\{                                                                                       { printf("LBRACE\n");    }
\}                                                                                       { printf("RBRACE\n");    }
\(                                                                                       { printf("LPAR\n");      }
\)                                                                                       { printf("RPAR\n");      }
\[                                                                                       { printf("LSQ\n");       }
\]                                                                                       { printf("RSQ\n");       }
\<                                                                                       { printf("LT\n");        }
\-                                                                                       { printf("MINUS\n");     }
\+                                                                                       { printf("PLUS\n");      }
!=                                                                                       { printf("NE\n");        }
->                                                                                       { printf("ARROW\n");     }
\^                                                                                       { printf("XOR\n");       }
if                                                                                       { printf("IF\n");        }
int                                                                                      { printf("INT\n");       }
>\=                                                                                      { printf("GE\n");        }
\=\=                                                                                     { printf("EQ\n");        }
\<\=                                                                                     { printf("LE\n");        }
\|\|                                                                                     { printf("OR\n");        }
\<\<                                                                                     { printf("LSHIFT\n");    }
\>\>                                                                                     { printf("RSHIFT\n");    }
else                                                                                     { printf("ELSE\n");      }
void                                                                                     { printf("VOID\n");      }
class                                                                                    { printf("CLASS\n");     }
while                                                                                    { printf("WHILE\n");     }
public                                                                                   { printf("PUBLIC\n");    }
double                                                                                   { printf("DOUBLE\n");    }
static                                                                                   { printf("STATIC\n");    }
return                                                                                   { printf("RETURN\n");    }
String                                                                                   { printf("STRING\n");    }
boolean                                                                                  { printf("BOOL\n");      }
\.length                                                                                 { printf("DOTLENGTH\n"); }
true|false                                                                               { printf("BOOLIT\n");    }
Integer\.parseInt                                                                        { printf("PARSEINT\n");  }
System\.out\.print                                                                       { printf("PRINT\n");     }
"\+\+"|"\-\-"|null|Integer|System                                                        { printf("RESERVED\n");  }

{int_lit}                                                                                { printf("INTLIT\n");    }
\"({char}|{escape_seq})*\"                                                               { printf("STRLIT\n");    }
{letter}+({letter}|{int})*                                                               { printf("ID\n");        }
({int_lit}(\.({int_lit}{exponent}?)?|{exponent}))|({int_lit}?\.{int_lit}?{exponent}?)    { printf("REALLIT\n");   }

" "|\t|\n|"//".*                                                                         {                        }
%%
  /* Subroutines */
int main() {
  yylex();
  return 0;
}

int yywrap() {
  return 1;
}
