#ifndef TREE_GUARD
#define TREE_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <float.h>
#include <string.h>
#include <math.h>

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

typedef struct token_node {
    char *str;
    int line, column;
} Token_node;

typedef struct order_node {
    bool method;
    struct order_node *next;
} Order_node;

typedef struct table_node {
    char *name;
    char *type;
    bool arg;
    int index;
    bool global;
    void *ptr;
    struct table_node *next;
} Table_node;

typedef struct method_env {
    struct class_env *parent;
    struct method_env *next;
    char *method_name;
    char *return_type;
    Table_node *variables;
    struct node *root;
    bool dup;
    char **arg_types;
    int arg_count;

} Method_env;

typedef struct class_env {
    char *class_name;
    Method_env *methods;
    Table_node *variables;
    Order_node *order;
} Class_env;

Class_env *add_class_environment(char *name);
Method_env *add_method_environment(Class_env *class_env, char *return_type, int arg_count, char **args, char *name, struct node *root, bool dup);
Table_node *add_table_node(char *name, char *type, bool arg, bool global, int index);
bool add_method_variable(char *name, char *type, Method_env *parent, bool param, int index);
bool add_class_variable(char *name, char *type, Class_env *parent);
bool add_class_method(char *name, char *return_type, int arg_count, char **args, Class_env *parent, struct node *root);
void add_order(bool method, Class_env *parent);
bool methodcmp(Method_env *a, char *name, int arg_count, char **args);
void print_all(Class_env *env);
void print_eq(char *type);
int find_method(char *name, Method_env *env, int arg_count, char **args);
Table_node *find_var(char *name, Method_env *parent);

typedef struct node {
    char *key;
    char *value;
    int children_count;
    struct node **children;

    Method_env *env;
    Table_node *table_node;

    char *anno;
    char *return_anno;
    char *str_num;
    int line;
    int column;
} Tree_Node;

typedef char* (*Method_type)(Tree_Node*, Method_env*);
typedef void (*Method_gen_type)(Tree_Node*);

Tree_Node *new_node(char *key, char *value, int line, int column);
void get_tree(Tree_Node *parent, Tree_Node *root);
void add_child(Tree_Node *parent, Tree_Node *child);
void print_tree(Tree_Node *root, int level);

void program_tree(Tree_Node *program, Tree_Node *sub_root);
void method_body_tree(Tree_Node *program, Tree_Node *sub_root);
void method_params_tree(Tree_Node *method_params, Tree_Node *sub_root);
void field_decl_tree(Tree_Node *program, Tree_Node *sub_root, Tree_Node *type);
void var_decl_tree(Tree_Node *method_body, Tree_Node *sub_root, Tree_Node *type);
void statement_tree(Tree_Node *statement, Tree_Node *sub_root);
void call_tree(Tree_Node *call, Tree_Node *sub_root);

void destroy_tree(Tree_Node *root);
void destroy_env(Class_env *env);

Class_env *semantics(Tree_Node *root, Class_env *env);
void find_class_variables(Tree_Node *root, Class_env *env);
char *method_name(Tree_Node *header);
char *method_return_type(Tree_Node *header);
int method_arg_count(Tree_Node *header);
char **method_args(Tree_Node *header, int arg_count);
Method_env *get_method_env(Class_env *parent, int k);
Method_type get_function(char *name);
void find_static_str(Tree_Node *root);
void destroy_env(Class_env *env);
bool program_semantics(Tree_Node *root, Class_env *env);
bool method_decl_semantics(Tree_Node *method_decl, Method_env *env);
bool method_body_semantics(Tree_Node *method_body, Method_env *env);
char *var_decl_semantics(Tree_Node *var_decl, Method_env *env);
bool field_decl_semantics(Tree_Node *field_decl);
bool header_semantics(Tree_Node *header, Method_env *env);
char *while_semantics(Tree_Node *while_node, Method_env *env);
char *return_semantics(Tree_Node *return_node, Method_env *env);
char *print_semantics(Tree_Node *print_node, Method_env *env);
char *if_semantics(Tree_Node *if_node, Method_env *env);
char *do_while_semantics(Tree_Node *do_while_node, Method_env *env);
char *block_semantics(Tree_Node *block_node, Method_env *env);
char *parse_args_semantics(Tree_Node *parse_args_node, Method_env *env);
char *call_semantics(Tree_Node *call_node, Method_env *env);
char *length_semantics(Tree_Node *length_node, Method_env *env);
char *plus_semantics(Tree_Node *plus_node, Method_env *env);
char *minus_semantics(Tree_Node *minus_node, Method_env *env);
char *not_semantics(Tree_Node *not_node, Method_env *env);
char *add_semantics(Tree_Node *add_node, Method_env *env);
char *sub_semantics(Tree_Node *sub_node, Method_env *env);
char *mod_semantics(Tree_Node *mod_node, Method_env *env);
char *div_semantics(Tree_Node *div_node, Method_env *env);
char *mult_semantics(Tree_Node *mult_node, Method_env *env);
char *eq_semantics(Tree_Node *eq_node, Method_env *env);
char *ne_semantics(Tree_Node *ne_node, Method_env *env);
char *gt_semantics(Tree_Node *gt_node, Method_env *env);
char *ge_semantics(Tree_Node *ge_node, Method_env *env);
char *lt_semantics(Tree_Node *lt_node, Method_env *env);
char *le_semantics(Tree_Node *le_node, Method_env *env);
char *and_semantics(Tree_Node *and_node, Method_env *env);
char *or_semantics(Tree_Node *or_node, Method_env *env);
char *xor_semantics(Tree_Node *xor_node, Method_env *env);
char *rshift_semantics(Tree_Node *rshift_node, Method_env *env);
char *lshift_semantics(Tree_Node *lshift_node, Method_env *env);
char *assign_semantics(Tree_Node *assign_node, Method_env *env);
char *expr_semantics(Tree_Node *expr_node, Method_env *env);
char *void_semantics(Tree_Node *void_node, Method_env *env);
char *str_arr_semantics(Tree_Node *str_arr_node, Method_env *env);
char *str_lit_semantics(Tree_Node *str_lit_node, Method_env *env);
char *real_lit_semantics(Tree_Node *real_lit_node, Method_env *env);
char *int_semantics(Tree_Node *int_node, Method_env *env);
char *id_semantics(Tree_Node *id_node, Method_env *env);
char *dec_lit_semantics(Tree_Node *dec_lit_node, Method_env *env);
char *double_semantics(Tree_Node *double_node, Method_env *env);
char *bool_lit_semantics(Tree_Node *bool_lit_node, Method_env *env);
char *bool_semantics(Tree_Node *bool_node, Method_env *env);
bool arithmetic_semantics(char *first, char *second);
bool cmp_semantics(char *first, char *second);

void gen_code(Tree_Node *root, Class_env *env);
void variable_allocation(Table_node *node, bool global);
void gen_class_variables(Tree_Node *root, Class_env *env);
void llvm_type(char *type);
void gen_method(Tree_Node *node);
void gen_print(Tree_Node *node);
void skip(Tree_Node *node);
void gen_dec_lit(Tree_Node *node);
void gen_real_lit(Tree_Node *node);
int print_llvm_str(char *str, int k);
void find_static_str(Tree_Node *root);

#endif