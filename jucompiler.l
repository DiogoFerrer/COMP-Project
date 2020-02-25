%{
/*  ****************************  */
/*  Authors:                      */
/*    Diogo Ferrer    2017247199  */
/*    Guilherme Cruz  2016xxxxxx  */
/*  ****************************  */
%}

  /* Definitions */
%%
  /* Rules */
%%
  /* Subroutines */
int main() {
  yylex();
  return 0;
}

int yywrap() {
  return 1;
}
