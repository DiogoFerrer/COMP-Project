#include "ast_tree.h"

extern bool error;

Tree_Node *new_node(char *key, char *value, int line, int column) {
	Tree_Node *new_node = (Tree_Node*) malloc(sizeof(Tree_Node));

    if(!new_node) {
        return NULL;
    }

	new_node->key = strdup(key);
    new_node->value = value;

    new_node->children_count = 0;
    new_node->children = NULL;

    new_node->anno = NULL;
    new_node->env = NULL;
    new_node->return_anno = NULL;

    new_node->line = line;
    new_node->column = column;
    new_node->table_node = NULL;

	return new_node;
}

void add_child(Tree_Node *parent, Tree_Node *child) {
    parent->children_count++;
    parent->children = realloc(parent->children, parent->children_count * sizeof(Tree_Node*));
    
    int child_index = parent->children_count - 1;
    parent->children[child_index] = child;
}

void print_tree(Tree_Node *root, int level) {
    if(!root) {
        return;
    }

    int i;
    for(i = 0; i < level - 1; i++) {
        printf("..");
    }

    printf("%s", root->key);

    if(root->value) {
        printf("(%s)", root->value);
    }

    if(root->anno) {
        printf(" - ");
        print_eq(root->anno);
    }

    if(root->env) {
        printf(" - (");
        for(i = 0; i < root->env->arg_count; i++) {
            if(i) {
                printf(",");
            }
            print_eq(root->env->arg_types[i]);
        }

        printf(")");
    }

    printf("\n");

    for(i = 0; i < root->children_count; i++) {
        print_tree(root->children[i], level + 1);
    }
}

void program_tree(Tree_Node *program, Tree_Node *second_node) {
    if(!second_node || (
        strcmp(second_node->key, "FieldDecl") &&
        strcmp(second_node->key, "MethodDecl") &&
        strcmp(second_node->key, "ProgramRec")
    )) {
        return;
    } else if(strcmp(second_node->key, "ProgramRec")) {
        add_child(program, second_node);
    }

    if(!strcmp(second_node->key, "FieldDecl")) {
        field_decl_tree(program, second_node->children[2], second_node->children[0]);
        second_node->children[2] = NULL;
    }

    if(!strcmp(second_node->key, "ProgramRec")) {
        int i;
        for(i = 0; i < second_node->children_count; i++) {
            program_tree(program, second_node->children[i]);
        }

        if(second_node->children) {
            free(second_node->children);
        }
        free(second_node->key);
        free(second_node);
    }
}

void method_body_tree(Tree_Node *method_body, Tree_Node *second_node) {
    if(!second_node) {
        return;
    }

    bool statement = false;

    int i;
    for(i = 0; i < STATEMENT_COUNT; i++) {
        if(!strcmp(statements[i], second_node->key)) {
            statement = true;
            break;
        }
    }
    if(!statement) {
        return;
    }

    if(strcmp(second_node->key, "MethodBodyRec")) {
        if(strcmp(second_node->key, "Error")) {
            add_child(method_body, second_node);
        }

        if(!strcmp(second_node->key, "VarDecl")) {
            var_decl_tree(method_body, second_node->children[2], second_node->children[0]);
            second_node->children[2] = NULL;
        }
    } else {
        int j;
        for(j = 0; j < second_node->children_count; j++) {
            method_body_tree(method_body, second_node->children[j]);
        }
    }
}

void method_params_tree(Tree_Node *method_params, Tree_Node *second_node) {
    if(!second_node) {
        return;
    }

    Tree_Node *param_decl = new_node("ParamDecl", NULL, second_node->children[0]->line, second_node->children[0]->column);
    add_child(param_decl, second_node->children[0]);
    add_child(param_decl, second_node->children[1]);
    add_child(method_params, param_decl);

    method_params_tree(method_params, second_node->children[2]);

    if(second_node->children) {
        free(second_node->children);
    }
    free(second_node->key);
    free(second_node);
}

void field_decl_tree(Tree_Node *program, Tree_Node *second_node, Tree_Node *field_type) {
    if(!second_node) {
        return;
    }

    Tree_Node *field_type_node = new_node(field_type->key, NULL, field_type->line, field_type->column);
    Tree_Node *id = second_node->children[0];
    Tree_Node *field_decl = new_node("FieldDecl", NULL, field_type->line, field_type->column);

    add_child(field_decl, field_type_node);
    add_child(field_decl, id);
    add_child(program, field_decl);

    field_decl_tree(program, second_node->children[1], field_type);
    if(second_node->children) {
        free(second_node->children);
    }
    free(second_node->key);
    free(second_node);
}

void var_decl_tree(Tree_Node *method_body, Tree_Node *second_node, Tree_Node *var_type) {
    if(!second_node) {
        return;
    }

    Tree_Node *var_type_node = new_node(var_type->key, NULL, var_type->line, var_type->column);
    Tree_Node *id = second_node->children[0];
    Tree_Node *var_decl = new_node("VarDecl", NULL, var_type->line, var_type->column);

    add_child(var_decl, var_type_node);
    add_child(var_decl, id);
    add_child(method_body, var_decl);

    var_decl_tree(method_body, second_node->children[1], var_type);
}

void statement_tree(Tree_Node *statement, Tree_Node *second_node){
    if(!second_node) {
        return;
    }

    if(!strcmp(second_node->key, "StatementRec")) {
        if(second_node->children[0])
            add_child(statement, second_node->children[0]);
        statement_tree(statement, second_node->children[1]);
        if(second_node->children) {
            free(second_node->children);
        }
        free(second_node->key);
        free(second_node);
    }
}

void call_tree(Tree_Node *call, Tree_Node *second_node) {
    if(!second_node) {
        return;
    }

    add_child(call, second_node->children[0]);
    call_tree(call, second_node->children[1]);

    if(second_node->children)
        free(second_node->children);
    free(second_node->key);
    free(second_node);
}

void destroy_tree(Tree_Node *root) {
    if(!root) {
        return;
    }

    int i;
    for(i = 0; i < root->children_count; i++) {
        if(root->children[i])
            destroy_tree(root->children[i]);
    }

    free(root->children);
    if(root->value)
        free(root->value);
    if(root->return_anno)
        free(root->return_anno);
    free(root->key);
    free(root);
}

Class_env *semantics(Tree_Node *root, Class_env *env) {
    if(!root) {
        return NULL;
    }

    char *name = root->children[0]->value;
    env = add_class_environment(name);
    find_class_variables(root, env);
    program_semantics(root, env);
    return env;
}

Method_env *get_method_env(Class_env *parent, int k) {
    Method_env *env = parent->methods;

    int i;
    for(i = 0; i < k; i++) {
        if(!env) {
            return NULL;
        }
        env = env->next;
    }

    return env;
}

void find_class_variables(Tree_Node *root, Class_env *env) {
    int method_count = 0;

    int i;
    for(i = 0; i < root->children_count; i++) {
        if(!root->children[i]) continue;

        if(!strcmp(root->children[i]->key, "FieldDecl")) {
            if(!add_class_variable(root->children[i]->children[1]->value, root->children[i]->children[0]->key, env)) {
                error = true;
                printf("Line %d, col %d: Symbol %s already defined\n", root->children[i]->children[1]->line, root->children[i]->children[1]->column, root->children[i]->children[1]->value);
            }
        } else if(!strcmp(root->children[i]->key, "MethodDecl")) {
            Tree_Node *header = root->children[i]->children[0];

            int arg_count = method_arg_count(header);
            char **args = method_args(header, arg_count);
            char *name = method_name(header);
            char *return_type = method_return_type(header);

            int val = add_class_method(name, return_type, arg_count, args, env, root->children[i]);
            Tree_Node *args_node = header->children[2];
            Method_env *method_env = get_method_env(env, method_count);

            int m, param_n = 0;
            for(m = 0; m < args_node->children_count; m++) {
                Tree_Node *param_decl_node = args_node->children[m];

                if(!param_decl_node) continue;

                char *key = param_decl_node->children[1]->value;
                char *type = param_decl_node->children[0]->key;
                int val = add_method_variable(key, type, method_env, true, param_n);

                if(!val) {
                    error = true;
                    printf("Line %d, col %d: Symbol %s already defined\n", param_decl_node->children[1]->line, param_decl_node->children[1]->column, key);
                }
                param_n++;
            }

            if(!val) {
                error = true;
                printf("Line %d, col %d: Symbol %s(" , root->children[i]->children[0]->children[1]->line, root->children[i]->children[0]->children[1]->column, root->children[i]->children[0]->children[1]->value);

                int j;
                for(j = 0; j < arg_count; j++) {
                    if(j != 0) {
                        printf(",");
                    }

                    print_eq(args[j]);
                }

                printf(") already defined\n");
            }
            method_count++;
        }
    }
}

char *method_name(Tree_Node *header) {
    return header->children[1]->value;
}

char *method_return_type(Tree_Node *header) {
    return header->children[0]->key;
}

int method_arg_count(Tree_Node *header) {
    Tree_Node *args = header->children[2];
    int count = 0;

    int i;
    for(i = 0; i < args->children_count; i++) {
        if(!args->children[i]) continue;

        count++;
    }
    return count;
}

char **method_args(Tree_Node *header, int arg_count) {
    char **args = malloc(arg_count * sizeof(char*));
    int count = 0;

    Tree_Node *params = header->children[2];

    int i;
    for(i = 0; i < params->children_count; i++) {
        if(!params->children[i]) continue;

        args[count] = params->children[i]->children[0]->key;
        count++;
    }
    return args;
}

Method_type get_function(char *name) {
    if(!strcmp(name, "Int")) {
        return int_semantics;
    } else if(!strcmp(name, "Bool")) {
        return bool_semantics;
    } else if(!strcmp(name, "BoolLit")) {
        return bool_lit_semantics;
    } else if(!strcmp(name, "Double")) {
        return double_semantics;
    } else if(!strcmp(name, "DecLit")) {
        return dec_lit_semantics;
    } else if(!strcmp(name, "Id")) {
        return id_semantics;
    } else if(!strcmp(name, "RealLit")) {
        return real_lit_semantics;
    } else if(!strcmp(name, "StrLit")) {
        return str_lit_semantics;
    } else if(!strcmp(name, "StringArray")) {
        return str_arr_semantics;
    } else if(!strcmp(name, "Void")) {
        return void_semantics;
    } else if(!strcmp(name, "Assign")) {
        return assign_semantics;
    } else if(!strcmp(name, "Or")) {
        return or_semantics;
    } else if(!strcmp(name, "And")) {
        return and_semantics;
    } else if(!strcmp(name, "Eq")) {
        return eq_semantics;
    } else if(!strcmp(name, "Ne")) {
        return ne_semantics;
    } else if(!strcmp(name, "Lt")) {
        return lt_semantics;
    } else if(!strcmp(name, "Le")) {
        return le_semantics;
    } else if(!strcmp(name, "Gt")) {
        return gt_semantics;
    } else if(!strcmp(name, "Ge")) {
        return ge_semantics;
    } else if(!strcmp(name, "Add")) {
        return add_semantics;
    } else if(!strcmp(name, "Sub")) {
        return sub_semantics;
    } else if(!strcmp(name, "Mul")) {
        return mult_semantics;
    }  else if(!strcmp(name, "Div")) {
        return div_semantics;
    } else if(!strcmp(name, "Mod")) {
        return mod_semantics;
    } else if(!strcmp(name, "Not")) {
        return not_semantics;
    } else if(!strcmp(name, "Minus")) {
        return minus_semantics;
    } else if(!strcmp(name, "Plus")) {
        return plus_semantics;
    } else if(!strcmp(name, "Length")) {
        return length_semantics;
    } else if(!strcmp(name, "Call")) {
        return call_semantics;
    }  else if(!strcmp(name, "ParseArgs")) {
        return parse_args_semantics;
    } else if(!strcmp(name, "VarDecl")) {
        return var_decl_semantics;
    } else if(!strcmp(name, "Block")) {
        return block_semantics;
    } else if(!strcmp(name, "DoWhile")) {
        return do_while_semantics;
    } else if(!strcmp(name, "If")) {
        return if_semantics;
    } else if(!strcmp(name, "Print")) {
        return print_semantics;
    } else if(!strcmp(name, "Return")) {
        return return_semantics;
    } else if(!strcmp(name, "While")) {
        return while_semantics;
    } else if(!strcmp(name, "Xor")) {
        return xor_semantics;
    } else if(!strcmp(name, "Rshift")) {
        return rshift_semantics;
    } else if(!strcmp(name, "Lshift")) {
        return lshift_semantics;
    }

    return NULL;
}

bool program_semantics(Tree_Node *root, Class_env *env) {
    Method_env *method_env = env->methods;
    Method_env *prev = method_env;
    while(method_env) {
        method_decl_semantics(method_env->root, method_env);
        if(method_env->dup) {
            prev->next = method_env->next;
            method_env = method_env->next;
        } else {
            prev = method_env;
            method_env = method_env->next;
        }
    }

    return true;
}

bool method_decl_semantics(Tree_Node *method_decl, Method_env *env) {
    header_semantics(method_decl->children[0], env);
    method_body_semantics(method_decl->children[1], env);
    return true;
}

bool header_semantics(Tree_Node *header, Method_env *env) {
    return true;
}

bool method_body_semantics(Tree_Node *method_body, Method_env *env) {
    int i;
    for(i = 0; i < method_body->children_count; i++) {
        if(!method_body->children[i]) continue;

        get_function(method_body->children[i]->key)(method_body->children[i], env);
    }
    return true;
}

bool field_decl_semantics(Tree_Node *field_decl) {
    return true;
}

char *var_decl_semantics(Tree_Node *var_decl, Method_env *env) {
    char *name = var_decl->children[1]->value;
    char *type = var_decl->children[0]->key;

    int val = add_method_variable(name, type, env, false, -1);
    if(!val) {
        error = true;
        printf("Line %d, col %d: Symbol %s already defined\n", var_decl->children[1]->line, var_decl->children[1]->column, name);
    }

    return NULL;
}

char *while_semantics(Tree_Node *while_node, Method_env *env) {
    char *condition_type = get_function(while_node->children[0]->key)(while_node->children[0], env);
    if(strcmp(condition_type, "Bool")) {
        error = true;
        printf("Line %d, col %d: Incompatible type ", while_node->children[0]->line, while_node->children[0]->column);
        print_eq(condition_type);
        printf(" in while statement\n");
    }

    get_function(while_node->children[1]->key)(while_node->children[1], env);
    return NULL;
}

char *return_semantics(Tree_Node *return_node, Method_env *env) {
    return_node->return_anno = strdup(env->return_type);

    if(return_node->children_count == 1) {
        char *type = get_function(return_node->children[0]->key)(return_node->children[0], env);

        if(!strcmp(env->return_type, "Void")) {
            error = true;
            printf("Line %d, col %d: Incompatible type ", return_node->children[0]->line, return_node->children[0]->column);
            print_eq(return_node->children[0]->anno);
            printf(" in return statement\n");
            return NULL;
        } else if(!strcmp(type, env->return_type) || (!strcmp(type, "Int") && !strcmp(env->return_type, "Double"))) {
		} else {
            error = true;
            printf("Line %d, col %d: Incompatible type ", return_node->children[0]->line, return_node->children[0]->column);
            print_eq(type);
            printf(" in return statement\n");
        }
        return NULL;
    }
    if(strcmp(env->return_type, "Void")) {
        error = true;
        printf("Line %d, col %d: Incompatible type void in return statement\n", return_node->line, return_node->column);
    }

    return NULL;
}

char *print_semantics(Tree_Node *print_node, Method_env *env) {
    char *type = get_function(print_node->children[0]->key)(print_node->children[0], env);

    if(!(!strcmp(type, "Int") || !strcmp(type, "Bool") || !strcmp(type, "Double") || !strcmp(type, "String"))) {
        error = true;
        printf("Line %d, col %d: Incompatible type ", print_node->children[0]->line, print_node->children[0]->column);
        print_eq(type);
        printf(" in System.out.print statement\n");
    }

    return type;
}

char *if_semantics(Tree_Node *if_node, Method_env *env) {
    char *condition_type = get_function(if_node->children[0]->key)(if_node->children[0], env);

    if(strcmp(condition_type, "Bool")) {
        error = true;
        printf("Line %d, col %d: Incompatible type ", if_node->children[0]->line, if_node->children[0]->column);
        print_eq(condition_type);
        printf(" in if statement\n");
    }

    get_function(if_node->children[1]->key)(if_node->children[1], env);
    get_function(if_node->children[2]->key)(if_node->children[2], env);

    return NULL;
}

char *do_while_semantics(Tree_Node *do_while_node, Method_env *env) {
    get_function(do_while_node->children[0]->key)(do_while_node->children[0], env);

    char *condition_type = get_function(do_while_node->children[1]->key)(do_while_node->children[1], env);

    if(strcmp(condition_type, "Bool")) {
        error = true;

        printf("Line %d, col %d: Incompatible type ", do_while_node->children[1]->line, do_while_node->children[1]->column);
        print_eq(condition_type);
        printf(" in do statement\n");
    }

    return NULL;
}

char *block_semantics(Tree_Node *block_node, Method_env *env) {
    int i;
    for(i = 0; i < block_node->children_count; i++) {
        if(block_node->children[i]) {
            get_function(block_node->children[i]->key)(block_node->children[i], env);
        }
    }

    return NULL;
}

char *parse_args_semantics(Tree_Node *parse_args_node, Method_env *env) {
    char *first = get_function(parse_args_node->children[0]->key)(parse_args_node->children[0], env);
    char *second = get_function(parse_args_node->children[1]->key)(parse_args_node->children[1], env);

    if(strcmp(first, "StringArray") || strcmp(second, "Int")) {
        error = true;

        printf("Line %d, col %d: Operator Integer.parseInt cannot be applied to types ", parse_args_node->line, parse_args_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    parse_args_node->anno = "Int";
    return "Int";
}

char *call_semantics(Tree_Node *call_node, Method_env *env) {
    char *call_key = call_node->children[0]->value;
    int count = 0;

    int i;
    for(i = 1; i < call_node->children_count; i++) {
        if(call_node->children[i]) {
            get_function(call_node->children[i]->key)(call_node->children[i], env);
            count++;
        }
    }

    char **params = malloc(count * sizeof(char*));
    int j;
    for(i = 1, j = 0; i < call_node->children_count; i++) {
        if(call_node->children[i]) {
            params[j++] = call_node->children[i]->anno;
        }
    }

    int function = find_method(call_key, env, count, params);
    if(function == -1) {
        error = true;

        printf("Line %d, col %d: Cannot find symbol %s(", call_node->children[0]->line, call_node->children[0]->column, call_node->children[0]->value);

        for(i = 0; i < count; i++) {
            if(i != 0) {
                printf(",");
            }

            print_eq(params[i]);
        }

        printf(")\n");
        call_node->children[0]->anno = "Undef";
        call_node->anno = "Undef";
        free(params);
        return "Undef";
    } else if(function == -2) {
        error = true;

        printf("Line %d, col %d: Reference to method %s(", call_node->children[0]->line, call_node->children[0]->column, call_node->children[0]->value);
        for(i = 0; i < count; i++) {
            if(i != 0) {
                printf(",");
            }

            print_eq(params[i]);
        }

        printf(") is ambiguous\n");
        call_node->anno = "Undef";
        call_node->children[0]->anno = "Undef";
        free(params);
        return "Undef";
    }

    Method_env *m_env = get_method_env(env->parent, function);
    call_node->anno = method_return_type(m_env->root->children[0]);

    Tree_Node *m_id = call_node->children[0];
    m_id->env = m_env;
    free(params);

    return call_node->anno;
}

char *length_semantics(Tree_Node *length_node, Method_env *env) {
    char *function = get_function(length_node->children[0]->key)(length_node->children[0], env);

    if(strcmp(function, "StringArray")) {
        error = true;

        printf("Line %d, col %d: Operator .length cannot be applied to type ", length_node->line, length_node->column + 1);
        print_eq(function);
        printf("\n");
    }

    length_node->anno = "Int";
    return "Int";
}

char *plus_semantics(Tree_Node *plus_node, Method_env *env) {
    char *function = get_function(plus_node->children[0]->key)(plus_node->children[0], env);

    if(!strcmp(function, "Double") || !strcmp(function, "Int")) {
        plus_node->anno = function;
    } else {
        error = true;

        printf("Line %d, col %d: Operator + cannot be applied to type ", plus_node->line, plus_node->column);
        print_eq(function);
        printf("\n");

        plus_node->anno = "Undef";
    }

    return plus_node->anno;
}

char *minus_semantics(Tree_Node *minus_node, Method_env *env) {
    char *function = get_function(minus_node->children[0]->key)(minus_node->children[0], env);

    if(!strcmp(function, "Double") || !strcmp(function, "Int")) {
        minus_node->anno = function;
    } else {
        error = true;

        printf("Line %d, col %d: Operator - cannot be applied to type ", minus_node->line, minus_node->column);
        print_eq(function);
        printf("\n");

        minus_node->anno = "Undef";
    }

    return minus_node->anno;
}

char *not_semantics(Tree_Node *not_node, Method_env *env) {
    char *function = get_function(not_node->children[0]->key)(not_node->children[0], env);

    if(strcmp(function, "Bool")) {
        error = true;

        printf("Line %d, col %d: Operator ! cannot be applied to type ", not_node->line, not_node->column);
        print_eq(function);
        printf("\n");
    }

    not_node->anno = "Bool";
    return "Bool";
}

char *mod_semantics(Tree_Node *mod_node, Method_env *env) {
    char *first = get_function(mod_node->children[0]->key)(mod_node->children[0], env);
    char *second = get_function(mod_node->children[1]->key)(mod_node->children[1], env);

    if(!strcmp(first, "Int") || !strcmp(second, "Int")) {
        mod_node->anno = "Int";
    } else if((!strcmp(first, "Double") && !strcmp(second, "Int")) ||
            (!strcmp(first, "Int") && !strcmp(second, "Double")) ||
            (!strcmp(first, "Double") && !strcmp(second, "Double"))) {
        mod_node->anno = "Double";
    } else {
        error = true;

        printf("Line %d, col %d: Operator %% cannot be applied to types ", mod_node->line, mod_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");

        mod_node->anno = "Undef";
    }

    return mod_node->anno;
}

char *div_semantics(Tree_Node *div_node, Method_env *env) {
    char *first = get_function(div_node->children[0]->key)(div_node->children[0], env);
    char *second = get_function(div_node->children[1]->key)(div_node->children[1], env);

    if(!strcmp(first, "Int") && !strcmp(second, "Int")) {
        div_node->anno = second;
    } else if(!strcmp(first, "Int") && !strcmp(second, "Double")) {
        div_node->anno = second;
    } else if(!strcmp(first, "Double") && !strcmp(second, "Int")) {
        div_node->anno = first;
    } else if(!strcmp(first, "Double") && !strcmp(second, "Double")) {
        div_node->anno = first;
    } else {
        error = true;

        printf("Line %d, col %d: Operator / cannot be applied to types ", div_node->line, div_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");

        div_node->anno = "Undef";
    }

    return div_node->anno;
}

char *mult_semantics(Tree_Node *mult_node, Method_env *env) {
    char *first = get_function(mult_node->children[0]->key)(mult_node->children[0], env);
    char *second = get_function(mult_node->children[1]->key)(mult_node->children[1], env);

    if(!strcmp(first, "Int") && !strcmp(second, "Int")) {
        mult_node->anno = second;
    } else if(!strcmp(first, "Int") && !strcmp(second, "Double")) {
        mult_node->anno = second;
    } else if(!strcmp(first, "Double") && !strcmp(second, "Int")) {
        mult_node->anno = first;
    } else if(!strcmp(first, "Double") && !strcmp(second, "Double")) {
        mult_node->anno = first;
    } else {
        error = true;

        printf("Line %d, col %d: Operator * cannot be applied to types ", mult_node->line, mult_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");

        mult_node->anno = "Undef";
    }

    return mult_node->anno;
}

char *sub_semantics(Tree_Node *sub_node, Method_env *env) {
    char *first = get_function(sub_node->children[0]->key)(sub_node->children[0], env);
    char *second = get_function(sub_node->children[1]->key)(sub_node->children[1], env);

    if(!strcmp(first, "Int") && !strcmp(second, "Int")) {
        sub_node->anno = second;
    } else if(!strcmp(first, "Int") && !strcmp(second, "Double")) {
        sub_node->anno = second;
    } else if(!strcmp(first, "Double") && !strcmp(second, "Int")) {
        sub_node->anno = first;
    } else if(!strcmp(first, "Double") && !strcmp(second, "Double")) {
        sub_node->anno = first;
    } else {
        error = true;

        printf("Line %d, col %d: Operator - cannot be applied to types ", sub_node->line, sub_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");

        sub_node->anno = "Undef";
    }

    return sub_node->anno;
}

char *add_semantics(Tree_Node *add_node, Method_env *env) {
    char *first = get_function(add_node->children[0]->key)(add_node->children[0], env);
    char *second = get_function(add_node->children[1]->key)(add_node->children[1], env);

    if(!strcmp(first, "Int") && !strcmp(second, "Int")) {
        add_node->anno = second;
    } else if(!strcmp(first, "Int") && !strcmp(second, "Double")) {
        add_node->anno = second;
    } else if(!strcmp(first, "Double") && !strcmp(second, "Int")) {
        add_node->anno = first;
    } else if(!strcmp(first, "Double") && !strcmp(second, "Double")) {
        add_node->anno = first;
    } else {
        error = true;

        printf("Line %d, col %d: Operator + cannot be applied to types ", add_node->line, add_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");

        add_node->anno = "Undef";
    }

    return add_node->anno;
}

char *gt_semantics(Tree_Node *gt_node, Method_env *env) {
    char *first = get_function(gt_node->children[0]->key)(gt_node->children[0], env);
    char *second = get_function(gt_node->children[1]->key)(gt_node->children[1], env);

    gt_node->anno = "Bool";

    if(!arithmetic_semantics(first, second)) {
        error = 1;

        printf("Line %d, col %d: Operator > cannot be applied to types ", gt_node->line, gt_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return gt_node->anno;
}

char *ge_semantics(Tree_Node *ge_node, Method_env *env) {
    char *first = get_function(ge_node->children[0]->key)(ge_node->children[0], env);
    char *second = get_function(ge_node->children[1]->key)(ge_node->children[1], env);

    ge_node->anno = "Bool";

    if(!arithmetic_semantics(first, second)) {
        error = 1;

        printf("Line %d, col %d: Operator >= cannot be applied to types ", ge_node->line, ge_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return ge_node->anno;
}

char *le_semantics(Tree_Node *le_node, Method_env *env) {
    char *first = get_function(le_node->children[0]->key)(le_node->children[0], env);
    char *second = get_function(le_node->children[1]->key)(le_node->children[1], env);

    le_node->anno = "Bool";

    if(!arithmetic_semantics(first, second)) {
        error = 1;

        printf("Line %d, col %d: Operator <= cannot be applied to types ", le_node->line, le_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return le_node->anno;
}

char *lt_semantics(Tree_Node *lt_node, Method_env *env) {
    char *first = get_function(lt_node->children[0]->key)(lt_node->children[0], env);
    char *second = get_function(lt_node->children[1]->key)(lt_node->children[1], env);

    lt_node->anno = "Bool";

    if(!arithmetic_semantics(first, second)) {
        error = true;

        printf("Line %d, col %d: Operator < cannot be applied to types ", lt_node->line, lt_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return lt_node->anno;
}

char *ne_semantics(Tree_Node *ne_node, Method_env *env) {
    char *first = get_function(ne_node->children[0]->key)(ne_node->children[0], env);
    char *second = get_function(ne_node->children[1]->key)(ne_node->children[1], env);

    ne_node->anno = "Bool";

    if(!cmp_semantics(first, second)) {
        error = true;

        printf("Line %d, col %d: Operator != cannot be applied to types ", ne_node->line, ne_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return ne_node->anno;
}

char *eq_semantics(Tree_Node *eq_node, Method_env *env) {
    char *first = get_function(eq_node->children[0]->key)(eq_node->children[0], env);
    char *second = get_function(eq_node->children[1]->key)(eq_node->children[1], env);

    eq_node->anno = "Bool";

    if(!cmp_semantics(first, second)) {
        error = true;

        printf("Line %d, col %d: Operator == cannot be applied to types ", eq_node->line, eq_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return eq_node->anno;
}

char *and_semantics(Tree_Node *and_node, Method_env *env) {
    char *first = get_function(and_node->children[0]->key)(and_node->children[0], env);
    char *second = get_function(and_node->children[1]->key)(and_node->children[1], env);

    and_node->anno = "Bool";

    if(strcmp(first, "Bool") || strcmp(second, "Bool")) {
        error = true;

        printf("Line %d, col %d: Operator && cannot be applied to types ", and_node->line, and_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return and_node->anno;
}

char *or_semantics(Tree_Node *or_node, Method_env *env) {
    char *first = get_function(or_node->children[0]->key)(or_node->children[0], env);
    char *second = get_function(or_node->children[1]->key)(or_node->children[1], env);

    or_node->anno = "Bool";

    if(strcmp(first, "Bool") || strcmp(second, "Bool")) {
        error = true;

        printf("Line %d, col %d: Operator || cannot be applied to types ", or_node->line, or_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return or_node->anno;
}

char *xor_semantics(Tree_Node *xor_node, Method_env *env) {
    char *first = get_function(xor_node->children[0]->key)(xor_node->children[0], env);
    char *second = get_function(xor_node->children[1]->key)(xor_node->children[1], env);

    xor_node->anno = "Bool";

    if(strcmp(first, "Bool") || strcmp(second, "Bool")) {
        error = true;

        printf("Line %d, col %d: Operator ^ cannot be applied to types ", xor_node->line, xor_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return xor_node->anno;
}

char *rshift_semantics(Tree_Node *rshift_node, Method_env *env) {
    char *first = get_function(rshift_node->children[0]->key)(rshift_node->children[0], env);
    char *second = get_function(rshift_node->children[1]->key)(rshift_node->children[1], env);

    if(strcmp(first, "Int") || strcmp(second, "Int")) {
        error = true;

        printf("Line %d, col %d: Operator >> cannot be applied to types ", rshift_node->line, rshift_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return "Int";
}

char *lshift_semantics(Tree_Node *lshift_node, Method_env *env) {
    char *first = get_function(lshift_node->children[0]->key)(lshift_node->children[0], env);
    char *second = get_function(lshift_node->children[1]->key)(lshift_node->children[1], env);

    if(strcmp(first, "Int") || strcmp(second, "Int")) {
        error = true;

        printf("Line %d, col %d: Operator << cannot be applied to types ", lshift_node->line, lshift_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");
    }

    return "Int";
}

char *assign_semantics(Tree_Node *assign_node, Method_env *env) {
    char *first = get_function(assign_node->children[0]->key)(assign_node->children[0], env);
    char *second = get_function(assign_node->children[1]->key)(assign_node->children[1], env);

    if(!strcmp(first, "Int") && !strcmp(second, "Int")) {
        assign_node->anno = first;
    } else if(!strcmp(first, "Double") && !strcmp(second, "Int")) {
        assign_node->anno = first;
    } else if(!strcmp(first, "Double") && !strcmp(second, "Double")) {
        assign_node->anno = first;
    } else if(!strcmp(first, "Bool") && !strcmp(second, "Bool")) {
        assign_node->anno = first;
    } else {
        error = true;

        printf("Line %d, col %d: Operator = cannot be applied to types ", assign_node->line, assign_node->column);
        print_eq(first);
        printf(", ");
        print_eq(second);
        printf("\n");

        assign_node->anno = first;
    }

    return assign_node->anno;
}

char *void_semantics(Tree_Node *void_node, Method_env *env) {
    return void_node->key;
}

char *str_arr_semantics(Tree_Node *str_arr_node, Method_env *env) {
    str_arr_node->anno = str_arr_node->key;

    return str_arr_node->anno;
}

char *str_lit_semantics(Tree_Node *str_lit_node, Method_env *env) {
    str_lit_node->anno = "String";
    return str_lit_node->anno;
}

char *real_lit_semantics(Tree_Node *real_lit_node, Method_env *env) {
    char *real_lit = real_lit_node->value;
    char *buff = malloc(1024 * sizeof(char));

    int i = 0;
    int j = 0;
    bool dot = true;
    bool exp = false;

    while(real_lit[i] != '\0') {
        char curr = real_lit[i];

        if(curr == 'E' || curr == 'e' || curr == '.' || curr == '-' || (curr >= '0' && curr <= '9')) {
            if(curr == 'E' || curr == 'e') {
                exp = true;
            }
            if(curr != '.' && curr != '0' && !exp) {
                dot = false;
            }
            buff[j++] = real_lit[i];
        }

        i++;
    }

    buff[j] = '\0';

    if(!dot) {
        double real = atof(buff);

        if(real > DBL_MAX || isinf(real) || real == 0) {
            error = 1;
            printf("Line %d, col %d: Number %s out of bounds\n", real_lit_node->line, real_lit_node->column, real_lit_node->value);
        }
    }

    free(buff);
    real_lit_node->anno = "Double";

    return real_lit_node->anno;
}

char *int_semantics(Tree_Node *int_node, Method_env *env) {
    return int_node->key;
}

char *id_semantics(Tree_Node *id_node, Method_env *env) {
    Table_node *node = find_var(id_node->value, env);

    if(!node) {
        error = true;

        printf("Line %d, col %d: Cannot find symbol %s\n", id_node->line, id_node->column, id_node->value);
        id_node->anno = "Undef";
    } else {
        id_node->anno = node->type;
        id_node->table_node = node;
    }

    return id_node->anno;
}

void removeChar(char *s, int c){ 
  
    int j, n = strlen(s); 
    for (int i=j=0; i<n; i++) 
       if (s[i] != c) 
          s[j++] = s[i]; 
      
    s[j] = '\0'; 
} 

char *dec_lit_semantics(Tree_Node *dec_lit_node, Method_env *env) {
    dec_lit_node->anno = "Int";

    char aux[1024];
    strcpy(aux, dec_lit_node->value);
    removeChar(aux, '_');
    long dec = strtol(aux, NULL, 10);

    if(dec > INT_MAX || dec < INT_MIN) {
        error = true;
        printf("Line %d, col %d: Number %s out of bounds\n", dec_lit_node->line, dec_lit_node->column, dec_lit_node->value);
    }

    return dec_lit_node->anno;
}

char *double_semantics(Tree_Node *double_node, Method_env *env) {
    return double_node->key;
}

char *bool_lit_semantics(Tree_Node *bool_lit_node, Method_env *env) {
    bool_lit_node->anno = "Bool";
    return bool_lit_node->anno;
}

char *bool_semantics(Tree_Node *bool_node, Method_env *env) {
    return bool_node->key;
}

bool arithmetic_semantics(char *first, char *second) {
    if((!strcmp(first, "Int") && !strcmp(second, "Int")) ||
        (!strcmp(first, "Int") && !strcmp(second, "Double")) ||
        (!strcmp(first, "Double") && !strcmp(second, "Int")) ||
        (!strcmp(first, "Double") && !strcmp(second, "Double"))) {
        return true;
    }

    return false;
}

bool cmp_semantics(char *first, char *second) {
    if ((!strcmp(first, "Int") && !strcmp(second, "Int")) ||
        (!strcmp(first, "Int") && !strcmp(second, "Double")) ||
        (!strcmp(first, "Double") && !strcmp(second, "Int")) ||
        (!strcmp(first, "Double") && !strcmp(second, "Double")) ||
        (!strcmp(first, "Bool") && !strcmp(second, "Bool"))) {
        return true;
    }

    return false;
}