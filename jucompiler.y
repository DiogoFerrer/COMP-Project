%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <string.h>
    #include "ast_tree.h"

    int yylex(void);
    int yylex_destroy(void);
    void yyerror (char *s);

    extern bool print_tokens;
    extern bool just_lexical;

    bool tree_flag = false;
    bool table_flag = false;
    int yylex_destroy(void);

    Tree_Node* root;
    bool error = false;

    extern int yyleng;
    extern int line, column;
%}

%union {
    Token_node token;
    Tree_Node *node_ptr;
}

%token <token> STRLIT;
%token <token> RESERVED;
%token <token> ID;
%token <token> BOOLLIT;
%token <token> REALLIT;
%token <token> INTLIT;
%token <token> ASSIGN SEMICOLON COMMA;
%token <token> WHILE DO;
%token <token> PUBLIC STATIC CLASS;
%token <token> PRINT DOTLENGTH PARSEINT RETURN;
%token <token> INT BOOL DOUBLE STRING VOID;
%token <token> ELSE IF;
%token <token> PLUS MINUS STAR DIV MOD RSHIFT LSHIFT;
%token <token> GE LE NE EQ LT GT;
%token <token> OR AND NOT XOR;
%token <token> LSQ RSQ;
%token <token> LBRACE RBRACE;
%token <token> LPAR RPAR;
%token PLUSPREC PRECEDENCE NO_ELSE;

%type <node_ptr> expression assignment methodInvocation parseArgs statement params paramsRec id methodBody void varDeclRec strlit error fieldDeclRec;
%type <node_ptr> program programRec fieldDecl methodDecl methodHeader typeRule methodBodyRec formalParams varDecl formalParamsRec statementRec expression_supp;

%left COMMA;
%left ASSIGN;
%left OR;
%left AND;
%left XOR;
%left EQ NE;
%left LE LT GT GE;
%left PLUS MINUS;
%left STAR DIV MOD RSHIFT LSHIFT;
%right PRECEDENCE;
%left LBRACE LPAR LSQ;
%nonassoc NO_ELSE;
%nonassoc ELSE;

%%

program: 
    CLASS id LBRACE programRec RBRACE {
        $$ = new_node("Program", NULL, $2->line, $2->column);
        add_child($$, $2);
        program_tree($$, $4);
        root = $$;
    }
    ;

programRec: 
    fieldDecl programRec        { $$ = new_node("ProgramRec", NULL, $1->line, $1->column); add_child($$, $1); add_child($$, $2); }
    | methodDecl programRec     { $$ = new_node("ProgramRec", NULL, $1->line, $1->column); add_child($$, $1); add_child($$, $2); }
    | SEMICOLON programRec      { $$ = new_node("ProgramRec", NULL, 0, 0); add_child($$, $2); }
    | %empty                    { $$ = NULL; }
    ;

fieldDecl:
    PUBLIC STATIC typeRule id fieldDeclRec SEMICOLON      { $$ = new_node("FieldDecl", NULL, $4->line, $4->column); add_child($$, $3); add_child($$, $4); add_child($$, $5); }
    | error SEMICOLON                                     { $$ = new_node("Error", NULL, $2.line, $2.column); tree_flag = false; }
    ;

fieldDeclRec:
    COMMA id fieldDeclRec   { $$ = new_node("FieldDeclRec", NULL, $2->line, $2->column); add_child($$, $2); add_child($$, $3); }
    | %empty                { $$ = NULL;}
    ;

methodDecl: 
    PUBLIC STATIC methodHeader methodBody   { $$ = new_node("MethodDecl", NULL, $1.line, $1.column); add_child($$, $3); add_child($$, $4); }
    ;

methodHeader:
    typeRule id LPAR formalParams RPAR  { $$ = new_node("MethodHeader", NULL, $1->line, $1->column); add_child($$, $1); add_child($$, $2); add_child($$, $4); }
    | void id LPAR formalParams RPAR    { $$ = new_node("MethodHeader", NULL, $1->line, $1->column); add_child($$, $1); add_child($$, $2); add_child($$, $4); }
    ;

methodBody:
    LBRACE methodBodyRec RBRACE { $$ = new_node("MethodBody", NULL, $1.line, $1.column); method_body_tree($$, $2); }
    ;

methodBodyRec:
    varDecl methodBodyRec       { $$ = new_node("MethodBodyRec", NULL, $1->line, $1->column); add_child($$, $1); add_child($$, $2); }
    | statement methodBodyRec   { $$ = new_node("MethodBodyRec", NULL, 0, 0); add_child($$, $1); add_child($$, $2); }
    | %empty                    { $$ = NULL; }
    ;

formalParams:
    %empty                          { $$ = new_node("MethodParams", NULL, line, column - yyleng); }
    | STRING LSQ RSQ id             {
                                        $$ = new_node("MethodParams", NULL, $1.line, $1.column);
                                        Tree_Node* params_decl = new_node("ParamDecl", NULL, $1.line, $1.column); 
                                        Tree_Node* string_array = new_node("StringArray", NULL, $1.line, $1.column);
                                        add_child(params_decl, string_array); 
                                        add_child(params_decl, $4);
                                        add_child($$, params_decl);
                                    }
    | typeRule id formalParamsRec   {
                                        $$ = new_node("MethodParams", NULL, $1->line, $1->column);
                                        Tree_Node* params_decl = new_node("ParamDecl", NULL, $1->line, $1->column); 
                                        add_child(params_decl, $1);
                                        add_child(params_decl, $2);
                                        add_child($$, params_decl); 
                                        method_params_tree($$, $3);
                                    }
    ;

formalParamsRec:
    %empty                                  { $$ = NULL; }
    | COMMA typeRule id formalParamsRec     { $$ = new_node("FormalParamsRec", NULL, $1.line, $1.column); add_child($$, $2); add_child($$, $3); add_child($$, $4); }
    ;

varDecl:
    typeRule id varDeclRec SEMICOLON    { $$ = new_node("VarDecl", NULL, $1->line, $1->column); add_child($$, $1); add_child($$, $2); add_child($$, $3); }
    ;

varDeclRec:
    %empty                  { $$ = NULL; }
    | COMMA id varDeclRec   { $$ = new_node("VarDeclRec", NULL, $2->line, $2->column); add_child($$, $2); add_child($$, $3); }           
    ;

typeRule: 
    BOOL        { $$ = new_node("Bool", NULL, $1.line, $1.column); }
    | INT       { $$ = new_node("Int", NULL, $1.line, $1.column); }
    | DOUBLE    { $$ = new_node("Double", NULL, $1.line, $1.column); }
    ;

statement:
    RETURN SEMICOLON                                      { $$ = new_node("Return", NULL, $1.line, $1.column); }
    | RETURN expression_supp SEMICOLON                         { $$ = new_node("Return", NULL, $1.line, $1.column); add_child($$, $2); }
    | SEMICOLON                                           { $$ = NULL; }
    | assignment SEMICOLON                                { $$ = $1; }
    | methodInvocation SEMICOLON                          { $$ = $1; }
    | parseArgs SEMICOLON                                 { $$ = $1; }
    | PRINT LPAR expression_supp RPAR SEMICOLON                { $$ = new_node("Print", NULL, $1.line, $1.column); add_child($$, $3); }
    | PRINT LPAR strlit RPAR SEMICOLON                    { $$ = new_node("Print", NULL, $1.line, $1.column); add_child($$, $3); }
    | WHILE LPAR expression_supp RPAR statement                {
                                                            $$ = new_node("While", NULL, $1.line, $1.column);
                                                            add_child($$, $3);
                                                            if(!$5)
                                                                add_child($$, new_node("Block", NULL, $1.line, $1.column));
                                                            else
                                                                add_child($$, $5);
                                                          }
    | IF LPAR expression_supp RPAR statement ELSE statement    {
                                                            $$ = new_node("If", NULL, $1.line, $1.column);
                                                            add_child($$, $3);
                                                            if(!$5)
                                                                add_child($$, new_node("Block", NULL, 0, 0));
                                                            else 
                                                                add_child($$, $5); 
                                                            if(!$7)
                                                                add_child($$, new_node("Block", NULL, 0, 0));
                                                            else
                                                                add_child($$, $7);
                                                          } 
    | IF LPAR expression_supp RPAR statement %prec NO_ELSE     {
                                                            $$ = new_node("If", NULL, $1.line, $1.column);
                                                            add_child($$, $3);
                                                            if(!$5)
                                                                add_child($$, new_node("Block", NULL, $3->line, $3->column));
                                                            else
                                                                add_child($$, $5);
                                                            add_child($$, new_node("Block", NULL, 0, 0));
                                                          }
    | LBRACE statementRec RBRACE                          {
                                                            $$ = new_node("Block", NULL, $1.line, $1.column);
                                                            statement_tree($$, $2);
                                                            int count = 0;
                                                            int k;
                                                            for(k = 0; k < $$->children_count; k++) {
                                                                if($$->children[k] != NULL) {
                                                                    count++;
                                                                }
                                                            }
                                                            if(count == 1) {
                                                                free($$);
                                                                $$ = $$->children[0];
                                                            } else if(count == 0) {
                                                                free($$);
                                                                $$ = NULL;
                                                            }
                                                          }
    | error SEMICOLON                                     { $$ = new_node("Error", NULL, 0, 0); tree_flag = false; }
    ;

strlit: 
    STRLIT  { $$ = new_node("StrLit", $1.str, $1.line, $1.column); }
    ;

statementRec:
    %empty                      {$$ = NULL;}
    | statement statementRec    {$$ = new_node("StatementRec", NULL, 0, 0); add_child($$, $1); add_child($$, $2); }
    ;

expression_supp:
    assignment
    | expression
    ;

expression:
    BOOLLIT                                 { $$ = new_node("BoolLit", $1.str, $1.line, $1.column); }                                            
    | INTLIT                                { $$ = new_node("DecLit", $1.str, $1.line, $1.column); }
    | REALLIT                               { $$ = new_node("RealLit", $1.str, $1.line, $1.column); }
    | LPAR expression_supp RPAR                  { $$ = $2; }
    | PLUS expression   %prec PRECEDENCE    { $$ = new_node("Plus", NULL, $1.line, $1.column); add_child($$, $2); }
    | MINUS expression  %prec PRECEDENCE    { $$ = new_node("Minus", NULL, $1.line, $1.column); add_child($$, $2); }
    | NOT expression    %prec PRECEDENCE    { $$ = new_node("Not", NULL, $1.line, $1.column); add_child($$, $2); }
    | expression PLUS expression            { $$ = new_node("Add", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression MINUS expression           { $$ = new_node("Sub", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression STAR expression            { $$ = new_node("Mul", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression DIV expression             { $$ = new_node("Div", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression MOD expression             { $$ = new_node("Mod", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression EQ expression              { $$ = new_node("Eq", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression GE expression              { $$ = new_node("Ge", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression GT expression              { $$ = new_node("Gt", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression LE expression              { $$ = new_node("Le", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression LT expression              { $$ = new_node("Lt", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression NE expression              { $$ = new_node("Ne", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression AND expression             { $$ = new_node("And", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression OR expression              { $$ = new_node("Or", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression XOR expression             { $$ = new_node("Xor", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression LSHIFT expression          { $$ = new_node("Lshift", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | expression RSHIFT expression          { $$ = new_node("Rshift", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    | id                                    { $$ = $1; }           
    | id DOTLENGTH                          { $$ = new_node("Length", NULL, $1->line, $1->column); add_child($$, $1); }
    | methodInvocation                      { $$ = $1; }
    | parseArgs                             { $$ = $1; }
    | LPAR error RPAR                       { $$ = new_node("Error", NULL, 0, 0); tree_flag = false; }
    ;

parseArgs:
    PARSEINT LPAR id LSQ expression_supp RSQ RPAR   { $$ = new_node("ParseArgs", NULL, $1.line, $1.column); add_child($$, $3); add_child($$, $5); }
    | PARSEINT LPAR error RPAR                 { $$ = new_node("Error", NULL, 0, 0); tree_flag = false; }
    ;

assignment:
    id ASSIGN expression_supp    { $$ = new_node("Assign", NULL, $2.line, $2.column); add_child($$, $1); add_child($$, $3); }
    ;

id:
    ID  { $$ = new_node("Id", $1.str, $1.line, $1.column); }
    ;

void:
    VOID    { $$ = new_node("Void", NULL, $1.line, $1.column); }
    ;

methodInvocation:
    id LPAR params RPAR     { $$ = new_node("Call", NULL, $1->line, $1->column); add_child($$, $1); call_tree($$, $3); }
    | id LPAR error RPAR    { $$ = new_node("Error", NULL, $1->line, $1->column); tree_flag = false; }
    ;

params:
    %empty                  { $$ = NULL; }
    | expression_supp paramsRec  { $$ = new_node("ParamsRec", NULL, $1->line, $1->column); add_child($$, $1); add_child($$, $2); }
    ;

paramsRec:
    %empty                          { $$ = NULL; }
    | COMMA expression_supp paramsRec    { $$ = new_node("ParamsRec", NULL, $2->line, $2->column); add_child($$, $2); add_child($$, $3); }
    ;
%%

int main(int argc, char **argv) {
    Class_env *env = NULL;

    if(argc == 2) {
        if(!strcmp(argv[1], "-l")) {
            print_tokens = true;
            just_lexical = true;
            table_flag = false;
            yylex();
        } else if(!strcmp(argv[1], "-e1")) {
            print_tokens = false;
            just_lexical = true;
            table_flag = false;
            yylex();
        } else if(!strcmp(argv[1], "-t")) {
            print_tokens = false;
            just_lexical = false;
            tree_flag = true;
            table_flag = false;
            yyparse();
        } else if(!strcmp(argv[1], "-e2")) {
            print_tokens = false;
            just_lexical = false;
            tree_flag = false;
            table_flag = false;
            yyparse();
        } else if(!strcmp(argv[1], "-s")) {
            print_tokens = false;
            just_lexical = false;
            tree_flag = true;
            table_flag = true;
            yyparse();
            if(!error) {
                env = semantics(root, env);
            }
        } else if(!strcmp(argv[1], "-e3")) {
            print_tokens = false;
            just_lexical = false;
            tree_flag = false;
            table_flag = false;
            yyparse();
            if(!error) {
                env = semantics(root, env);
            }
        }
    } else {
        yyparse();

        if(!error) {
            env = semantics(root, env);
        }
        if(!error) {
            gen_code(root, env);
        }
    }

    yylex_destroy();

    if(table_flag) {
        print_all(env);
    }

    if(tree_flag) {
        print_tree(root, 1);
    }

    destroy_env(env);
    destroy_tree(root);

    return 0;
}
