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
    Tree_Node* root;
%}

%union {
    char *string;
    struct node *node_ptr;
}

%token <string> STRLIT;
%token <string> RESERVED;
%token <string> ID;
%token <string> BOOLLIT;
%token <string> REALLIT;
%token <string> INTLIT;
%token <node_ptr> ASSIGN SEMICOLON COMMA;
%token WHILE;
%token PUBLIC STATIC CLASS;
%token <string> PRINT DOTLENGTH PARSEINT RETURN;
%token INT BOOL DOUBLE STRING VOID;
%token ELSE IF;
%token <string> PLUS MINUS STARF DIV MOD RSHIFT LSHIFT;
%token GE LE NE EQ LT GT;
%token <string> OR AND NOT XOR;
%token LSQ RSQ;
%token LBRACE RBRACE;
%token <string> LPAR RPAR;
%token PRECEDENCE NO_ELSE;

%type <node_ptr> expression assignment methodInvocation parseArgs statement params paramsRec id methodBody void varDeclRec strlit error fieldDeclRec;
%type <node_ptr> program programRec fieldDecl methodDecl methodHeader typeRule methodBodyRec formalParams varDecl formalParamsRec statementRec;

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
        $$ = new_node("Program", NULL);
        add_child($$, $2);
        program_tree($$, $4);
        root = $$;
    }
    ;

programRec: 
    fieldDecl programRec        { $$ = new_node("ProgramRec", NULL); add_child($$, $1); add_child($$, $2); }
    | methodDecl programRec     { $$ = new_node("ProgramRec", NULL); add_child($$, $1); add_child($$, $2); }
    | SEMICOLON programRec      { $$ = new_node("ProgramRec", NULL); add_child($$, $2); }
    | %empty                    { $$ = NULL; }
    ;

fieldDecl:
    PUBLIC STATIC typeRule id fieldDeclRec SEMICOLON      { $$ = new_node("FieldDecl", NULL); add_child($$, $3); add_child($$, $4); add_child($$, $5); }
    | error SEMICOLON                                     { $$ = new_node("Error", NULL); tree_flag = false; }
    ;

fieldDeclRec:
    COMMA id fieldDeclRec   { $$ = new_node("FieldDeclRec", NULL); add_child($$, $2); add_child($$, $3); }
    | %empty                { $$ = NULL;}
    ;

methodDecl: 
    PUBLIC STATIC methodHeader methodBody   { $$ = new_node("MethodDecl", NULL); add_child($$, $3); add_child($$, $4); }
    ;

methodHeader:
    typeRule id LPAR formalParams RPAR  { $$ = new_node("MethodHeader", NULL); add_child($$, $1); add_child($$, $2); add_child($$, $4); }
    | void id LPAR formalParams RPAR    { $$ = new_node("MethodHeader", NULL); add_child($$, $1); add_child($$, $2); add_child($$, $4); }
    ;

methodBody:
    LBRACE methodBodyRec RBRACE { $$ = new_node("MethodBody", NULL); method_body_tree($$, $2); }
    ;

methodBodyRec:
    varDecl methodBodyRec       { $$ = new_node("MethodBodyRec", NULL); add_child($$, $1); add_child($$, $2); }
    | statement methodBodyRec   { $$ = new_node("MethodBodyRec", NULL); add_child($$, $1); add_child($$, $2); }
    | %empty                    { $$ = NULL; }
    ;

formalParams:
    %empty                          { $$ = new_node("MethodParams", NULL); }
    | STRING LSQ RSQ id             {
                                        $$ = new_node("MethodParams", NULL);
                                        Tree_Node* params_decl = new_node("ParamDecl", NULL); 
                                        Tree_Node* string_array = new_node("StringArray", NULL);
                                        add_child(params_decl, string_array); 
                                        add_child(params_decl, $4);
                                        add_child($$, params_decl);
                                    }
    | typeRule id formalParamsRec   {
                                        $$ = new_node("MethodParams", NULL);
                                        Tree_Node* params_decl = new_node("ParamDecl", NULL); 
                                        add_child(params_decl, $1);
                                        add_child(params_decl, $2);
                                        add_child($$, params_decl); 
                                        method_params_tree($$, $3);
                                    }
    ;

formalParamsRec:
    %empty                                  { $$ = NULL; }
    | COMMA typeRule id formalParamsRec     { $$ = new_node("FormalParamsRec",NULL); add_child($$, $2); add_child($$, $3); add_child($$, $4); }
    ;

varDecl:
    typeRule id varDeclRec SEMICOLON    { $$ = new_node("VarDecl", NULL); add_child($$, $1); add_child($$, $2); add_child($$, $3); }
    ;

varDeclRec:
    %empty                  { $$ = NULL; }
    | COMMA id varDeclRec   { $$ = new_node("VarDeclRec", NULL); add_child($$, $2); add_child($$, $3); }           
    ;

typeRule: BOOL  { $$ = new_node("Bool", NULL); }
    | INT       { $$ = new_node("Int", NULL); }        
    | DOUBLE    { $$ = new_node("Double", NULL); }                               
    ;

statement:
    RETURN SEMICOLON                                      { $$ = new_node("Return", NULL); }
    | RETURN expression SEMICOLON                         { $$ = new_node("Return", NULL); add_child($$, $2); }
    | SEMICOLON                                           { $$ = NULL; }
    | assignment SEMICOLON                                { $$ = $1; }
    | methodInvocation SEMICOLON                          { $$ = $1; }
    | parseArgs SEMICOLON                                 { $$ = $1; }
    | PRINT LPAR expression RPAR SEMICOLON                { $$ = new_node("Print", NULL); add_child($$, $3); }
    | PRINT LPAR strlit RPAR SEMICOLON                    { $$ = new_node("Print", NULL); add_child($$, $3); }
    | WHILE LPAR expression RPAR statement                {
                                                            $$ = new_node("While", NULL);
                                                            add_child($$, $3);
                                                            if(!$5)
                                                                add_child($$, new_node("Block", NULL));
                                                            else
                                                                add_child($$, $5);
                                                          }
    | IF LPAR expression RPAR statement ELSE statement    {
                                                            $$ = new_node("If", NULL);
                                                            add_child($$, $3);
                                                            if(!$5)
                                                                add_child($$, new_node("Block", NULL));
                                                            else 
                                                                add_child($$, $5); 
                                                            if(!$7)
                                                                add_child($$, new_node("Block", NULL));
                                                            else
                                                                add_child($$, $7);
                                                          } 
    | IF LPAR expression RPAR statement %prec NO_ELSE     {
                                                            $$ = new_node("If", NULL);
                                                            add_child($$, $3);
                                                            if(!$5)
                                                                add_child($$, new_node("Block", NULL));
                                                            else
                                                                add_child($$, $5);
                                                            add_child($$, new_node("Block", NULL));
                                                          }
    | LBRACE statementRec RBRACE                          { 
                                                            $$ = new_node("Block", NULL);
                                                            statement_tree($$, $2);
                                                            if($$->children_count == 1)
                                                                $$ = $$->children[0];
                                                          }
    | error SEMICOLON                                     { $$ = new_node("Error", NULL); tree_flag = false; }
    ;

strlit: 
    STRLIT  { $$ = new_node("StrLit", $1); }
    ;

statementRec:
    %empty                      {$$ = NULL;}
    | statement statementRec    {$$ = new_node("StatementRec", NULL); add_child($$, $1); add_child($$, $2); }
    ;

expression:
    BOOLLIT                                 { $$ = new_node("BoolLit", $1); }                                            
    | INTLIT                                { $$ = new_node("DecLit", $1); }
    | REALLIT                               { $$ = new_node("RealLit", $1); }
    | LPAR expression RPAR                  { $$ = $2; }
    | PLUS expression   %prec PRECEDENCE    { $$ = new_node("Plus", NULL); add_child($$, $2); }
    | MINUS expression  %prec PRECEDENCE    { $$ = new_node("Minus", NULL); add_child($$, $2); }
    | NOT expression    %prec PRECEDENCE    { $$ = new_node("Not", NULL); add_child($$, $2); }
    | expression PLUS expression            { $$ = new_node("Add", NULL); add_child($$, $1); add_child($$, $3); }
    | expression MINUS expression           { $$ = new_node("Sub", NULL); add_child($$, $1); add_child($$, $3); }
    | expression STAR expression            { $$ = new_node("Mul", NULL); add_child($$, $1); add_child($$, $3); }
    | expression DIV expression             { $$ = new_node("Div", NULL); add_child($$, $1); add_child($$, $3); }
    | expression MOD expression             { $$ = new_node("Mod", NULL); add_child($$, $1); add_child($$, $3); }
    | expression EQ expression              { $$ = new_node("Eq", NULL); add_child($$, $1); add_child($$, $3); }
    | expression GE expression              { $$ = new_node("Ge", NULL); add_child($$, $1); add_child($$, $3); }
    | expression GT expression              { $$ = new_node("Gt", NULL); add_child($$, $1); add_child($$, $3); }
    | expression LE expression              { $$ = new_node("Le", NULL); add_child($$, $1); add_child($$, $3); }
    | expression LT expression              { $$ = new_node("Lt", NULL); add_child($$, $1); add_child($$, $3); }
    | expression NE expression              { $$ = new_node("Ne", NULL); add_child($$, $1); add_child($$, $3); }
    | expression AND expression             { $$ = new_node("And", NULL); add_child($$, $1); add_child($$, $3); }
    | expression OR expression              { $$ = new_node("Or", NULL); add_child($$, $1); add_child($$, $3); }
    | expression XOR expression             { $$ = new_node("Xor", NULL); add_child($$, $1); add_child($$, $3); }
    | expression LSHIFT expression          { $$ = new_node("Lshift", NULL); add_child($$, $1); add_child($$, $3); }
    | expression RSHIFT expression          { $$ = new_node("Rshift", NULL); add_child($$, $1); add_child($$, $3); }
    | id                                    { $$ = $1; }           
    | id DOTLENGTH                          { $$ = new_node("Length", NULL); add_child($$, $1); }
    | assignment                            { $$ = $1; }
    | methodInvocation                      { $$ = $1; }
    | parseArgs                             { $$ = $1; }
    | LPAR error RPAR                       { $$ = new_node("Error", NULL); tree_flag = false; }
    ;

parseArgs:
    PARSEINT LPAR id LSQ expression RSQ RPAR   { $$ = new_node("ParseArgs", NULL); add_child($$, $3); add_child($$, $5); }
    | PARSEINT LPAR error RPAR                 { $$ = new_node("Error", NULL); tree_flag = false; }
    ;

assignment:
    id ASSIGN expression    { $$ = new_node("Assign", NULL); add_child($$, $1); add_child($$, $3); }
    ;

id:
    ID  { $$ = new_node("Id", $1); }
    ;

void:
    VOID    { $$ = new_node("Void", NULL); }
    ;

methodInvocation:
    id LPAR params RPAR     { $$ = new_node("Call", NULL); add_child($$, $1); call_tree($$, $3); }
    | id LPAR error RPAR    { $$ = new_node("Error", NULL); tree_flag = false; }
    ;

params:
    %empty                  { $$ = NULL; }
    | expression paramsRec  { $$ = new_node("ParamsRec", NULL); add_child($$, $1); add_child($$, $2); }
    ;

paramsRec:
    %empty                          { $$ = NULL; }
    | COMMA expression paramsRec    { $$ = new_node("ParamsRec", NULL); add_child($$, $2); add_child($$, $3); }
    ;
%%

int main(int argc, char **argv) {
    if(argc == 2) {
        if(!strcmp(argv[1], "-l")) {
            print_tokens = true;
            just_lexical = true;
            yylex();
        } else if(!strcmp(argv[1], "-e1")) {
            print_tokens = false;
            just_lexical = true;
            yylex();
        } else if(!strcmp(argv[1], "-t")) {
            print_tokens = false;
            just_lexical = false;
            tree_flag = true;
            yyparse();
        } else if(!strcmp(argv[1], "-e2")) {
            print_tokens = false;
            just_lexical = false;
            tree_flag = false;
            yyparse();
        }
    } else {
        yyparse();
    }

    yylex_destroy();

    if(tree_flag) {
        print_tree(root, 1);
    }

    destroy_tree(root);

    return 0;
}
