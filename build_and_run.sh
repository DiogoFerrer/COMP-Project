#!/bin/bash

lex jucompiler.l
yacc -d jucompiler.y
clang-3.9 -o jucompiler lex.yy.c y.tab.c ast_tree.c
./jucompiler $1 < $2
