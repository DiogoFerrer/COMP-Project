#include "ast_tree.h"

extern bool error;

void add_order(bool method, Class_env *env) {
    Order_node *new_order_node = malloc(sizeof(Order_node));
    new_order_node->method = method;
    new_order_node->next = NULL;

    if(!env->order) {
        env->order = new_order_node;
        return;
    }

    Order_node *curr = env->order;
    while(curr->next) {
        curr = curr->next;
    }

    curr->next = new_order_node;
}

Table_node *add_table_node(char *key, char *type, bool arg, bool global, int index) {
    Table_node *new_node = malloc(sizeof(Table_node));

    if(!new_node) {
        return NULL;
    }

    new_node->name = strdup(key);
    new_node->type = type;
    new_node->arg = arg;
    new_node->global = global;
    new_node->index = index;
    new_node->next = NULL;

    return new_node;
}

Method_env *add_method_environment(Class_env *class_env, char *return_type, int arg_count, char **args, char *name, Tree_Node *root, bool dup) {
    Method_env *new_env = malloc(sizeof(Method_env));

    if(!new_env) {
        return NULL;
    }

    new_env->parent = class_env;
    new_env->return_type = strdup(return_type);
    new_env->arg_types = args;
    new_env->arg_count = arg_count;
    new_env->method_name = strdup(name);
    new_env->variables = NULL;
    new_env->root = root;
    new_env->dup = dup;
    new_env->next = NULL;

    return new_env;
}

Class_env *add_class_environment(char *name) {
    Class_env *new_env = malloc(sizeof(Class_env));

    if(!new_env) {
        return NULL;
    }

    new_env->class_name = strdup(name);
    new_env->methods = NULL;
    new_env->variables = NULL;
    new_env->order = NULL;

    return new_env;
}

bool add_method_variable(char *name, char *type, Method_env *parent, bool param, int index) {
    if(!parent->variables) {
        Table_node *new_node = add_table_node(name, type, param, false, index);
        parent->variables = new_node;
        return true;
    }

    Table_node *curr = parent->variables;
    Table_node *previous;

    while(curr) {
        if(!strcmp(curr->name, name)) {
            return false;
        }

        previous = curr;
        curr = curr->next;
    }

    Table_node *new_node = add_table_node(name, type, param, false, index);
    previous->next = new_node;
    return true;
}

bool add_class_variable(char *name, char *type, Class_env *parent) {
    if(!parent->variables) {
        Table_node *new_node = add_table_node(name, type, false, true, -1);
        parent->variables = new_node;
        add_order(false, parent);
        return true;
    }

    Table_node *curr = parent->variables;
    Table_node *previous;

    while(curr) {
        if(!strcmp(curr->name, name)) {
            return false;
        }

        previous = curr;
        curr = curr->next;
    }

    Table_node *new_node = add_table_node(name, type, false, true, -1);
    previous->next = new_node;

    add_order(false, parent);

    return true;
}

bool add_class_method(char *name, char *return_type, int arg_count, char **args, Class_env *parent, Tree_Node *root) {
    if(!parent->methods) {
        Method_env *new_node = add_method_environment(parent, return_type, arg_count, args, name, root, false);
        parent->methods = new_node;
        add_order(true, parent);
        return true;
    }

    bool dup = false;
    Method_env *curr = parent->methods;
    Method_env *previous;

    while(curr) {
        if(methodcmp(curr, name, arg_count, args)) {
            dup = true;
        }
        previous = curr;
        curr = curr->next;
    }

    Method_env *new_node = add_method_environment(parent, return_type, arg_count, args, name, root, dup);
    previous->next = new_node;

    if(!dup) {
        add_order(true, parent);
        return true;
    } else {
        return false;
    }
}

bool methodcmp(Method_env *a, char *name, int arg_count, char **args) {
    if(strcmp(a->method_name, name) || a->arg_count != arg_count) {
        return false;
    }

    int i;
    for(i = 0; i < arg_count; i++) {
        if(strcmp(args[i], a->arg_types[i])) {
            return false;
        }
    }

    return true;
}

Table_node *find_var(char *name, Method_env *parent) {
    Table_node *node = parent->variables;
    while(node) {
        if(!strcmp(node->name, name)) {
            return node;
        }
        node = node->next;
    }

    node = parent->parent->variables;
    while(node) {
        if(!strcmp(node->name, name)) {
            return node;
        }
        node = node->next;
    }

    return NULL;
}

int find_method(char *name, Method_env *env, int arg_count, char **args) {
    Method_env *methods = env->parent->methods;

    int aux = false;
    int aux_i = -1;
    int index = -1;

    while(methods) {
        index++;
        if(strcmp(methods->method_name, name) || arg_count != methods->arg_count) {
            methods = methods->next;
            continue;
        }

        bool curr_aux = false;
        bool found = true;

        int i;
        for(i = 0; i < methods->arg_count; i++) {
            if(!strcmp(methods->arg_types[i], args[i])) {
                continue;
            } else if(!strcmp(methods->arg_types[i], "Double") && !strcmp(args[i], "Int")) {
                curr_aux = true;
                continue;
            } else {
                found = false;
                curr_aux = false;
                break;
            }
        }

        if(!found) {
            methods = methods->next;
        } else if(curr_aux) {
            aux = true;
            aux_i = index;
            methods = methods->next;
            continue;
        } else {
            return index;
        }
    }

    if(!aux) {
        return -1;
    } else if(aux) {
        return aux_i;
    }

    return -2;
}

void print_all(Class_env *env) {
    if(!env) {
        return;
    }

    printf("===== Class %s Symbol Table =====\n", env->class_name);

    Order_node *order_node = env->order;
    Table_node *variable_node = env->variables;
    Method_env *method_env = env->methods;

    while(order_node) {
        if(order_node->method) {
            printf("%s\t(", method_env->method_name);

            int i;
            for(i = 0; i < method_env->arg_count; i++) {
                if(i != 0) {
                    printf(",");
                }

                print_eq(method_env->arg_types[i]);
            }

            printf(")\t");
            print_eq(method_env->return_type);
            printf("\n");
            method_env = method_env->next;
        } else {
            printf("%s\t\t", variable_node->name);
            print_eq(variable_node->type);
            printf("\n");
            variable_node = variable_node->next;
        }
        order_node = order_node->next;
    }

    method_env = env->methods;
    while(method_env) {
        printf("\n");
        printf("===== Method %s(", method_env->method_name);

        int i;
        for(i = 0; i < method_env->arg_count; i++) {
            if(i != 0) {
                printf(",");
            }

            print_eq(method_env->arg_types[i]);
        }

        printf(") Symbol Table =====\n");
        printf("return\t\t");
        print_eq(method_env->return_type);
        printf("\n");

        variable_node = method_env->variables;
        while(variable_node) {
            printf("%s\t\t", variable_node->name);
            print_eq(variable_node->type);

            if(variable_node->arg) {
                printf("\tparam");
            }
            printf("\n");

            variable_node = variable_node->next;
        }

        method_env = method_env->next;
    }

    printf("\n");
}

void print_eq(char *type) {
    if(!strcmp(type, "Undef")) {
        printf("undef");
    } else if(!strcmp(type, "StringArray")) {
        printf("String[]");
    } else if(!strcmp(type, "Int")) {
        printf("int");
    } else if(!strcmp(type, "Double")) {
        printf("double");
    } else if(!strcmp(type, "Bool")) {
        printf("boolean");
    } else if(!strcmp(type, "Void")) {
        printf("void");
    } else if(!strcmp(type, "String")) {
        printf("String");
    }
}

void destroy_vars(Table_node *variable) {
    while(variable) {
        Table_node *curr = variable;
        variable = variable->next;
        free(curr->name);
        free(curr);
    }
}

void destroy_env(Class_env *env) {
    if(!env) {
        return;
    }

    free(env->class_name);

    Method_env *method_env = env->methods;
    while(method_env) {
        Method_env *curr = method_env;
        method_env = method_env->next;
        destroy_vars(curr->variables);
        free(curr->method_name);
        free(curr->return_type);
        free(curr->arg_types);
        free(curr);
    }

    destroy_vars(env->variables);
    Order_node *order_node = env->order;
    while(order_node) {
        Order_node *curr = order_node;
        order_node = order_node->next;
        free(curr);
    }

    free(env);
}