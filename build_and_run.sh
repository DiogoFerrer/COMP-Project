#!/bin/bash

lex jucompiler.l
yacc -d jucompiler.y
clang -o jucompiler ast_tree.c table.c lex.yy.c y.tab.c
./jucompiler $1 < $2
