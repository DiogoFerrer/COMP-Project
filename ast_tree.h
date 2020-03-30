#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define STATEMENT_COUNT 10

static char *statements[] = {
    "Block",
    "If",
    "Print",
    "Return",
    "While",
    "Assign",
    "Call",
    "ParseArgs",
    "MethodBodyRec",
    "VarDecl"
};

typedef struct node {
    char *key;
    char *value;
    int children_count;
    struct node **children;
} Tree_Node;

Tree_Node *new_node(char *key, char *value);
void add_child(Tree_Node *parent, Tree_Node *child);
void print_tree(Tree_Node *root, int level);

void program_tree(Tree_Node *program, Tree_Node *sub_root);
void method_body_tree(Tree_Node *program, Tree_Node *sub_root);
void method_params_tree(Tree_Node *method_params, Tree_Node *sub_root);
void field_decl_tree(Tree_Node *program, Tree_Node *sub_root, char *type);
void var_decl_tree(Tree_Node *method_body, Tree_Node *sub_root, char *type);
void statement_tree(Tree_Node *statement, Tree_Node *sub_root);
void call_tree(Tree_Node *call, Tree_Node *sub_root);

void destroy_tree(Tree_Node *root);
