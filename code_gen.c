#include "ast_tree.h"

extern bool error;

int curr_str;
int curr_var;
int used_label;
int reserved_label;

void gen_code(Tree_Node *root, Class_env *env) {
    printf("@"STR"dec = private unnamed_addr constant [4 x i8] c\"%%d\\0A\\00\"\n");
    printf("@"STR"double = private unnamed_addr constant [7 x i8] c\"%%.16E""\\0A\\00\"\n");
    printf("@"STR"bools = private unnamed_addr constant[13 x i8] c\"false\\0A\\00true\\0A\\00\"\n");

    curr_str = 0;
    find_static_str(root);
    printf("\n");

    curr_str = 0;
    gen_class_variables(root, env);

    printf("\ndeclare i32 @printf(i8*, ...) nounwind\n");
    printf("declare i32 @atoi(i8*) #1\n");
    printf("\ndefine i32 @main(i32 %%argc, i8** %%argv) {\ncall void @"FUNC"main.StringArray(i32 %%argc, i8** %%argv)\nret i32 0\n}\n\n");

    int i;
    for(i = 0; i < root->children_count; i++) {
        Tree_Node *curr = root->children[i];

        if(curr && !strcmp(curr->key, "MethodDecl")) {
            curr_var = -1;
            reserved_label = 0;
            used_label = 0;
            gen_method(curr);
        }
    }
}

char *get_arg_name(Tree_Node *args, int index) {
    int i = 0;
    int curr = 0;

    while(curr != index) {
        if(args->children[i++]) {
            curr++;
        }
    }

    return args->children[i]->children[1]->value;
}

void gen_method(Tree_Node *node) {
    char *key = method_name(node->children[0]);
    int arg_count = method_arg_count(node->children[0]);
    char **args = method_args(node->children[0], arg_count);
    char *return_type = method_return_type(node->children[0]);

    printf("define %s @"FUNC"%s", llvm_type(return_type), key);

    int i;
    for(i = 0; i < arg_count; i++) {
        printf(".%s", args[i]);
    }

    printf("(");
    for(i = 0; i < arg_count; i++) {
        if(!strcmp(args[i], "StringArray")) {
            printf("i32 %%"ARGS"argc, i8** %%"ARGS"argv");
            break;
        }
        if(i) {
            printf(", ");
        }
        printf("%s %%%s", llvm_type(args[i]), get_arg_name(node->children[0]->children[2], i));
    }

    printf(") {\n");
    printf(LABEL"0:\n");

    for(i = 0; i < arg_count; i++) {
        if(!strcmp(args[i], "StringArray")) break;

        printf("%%%d = alloca %s\n", ++curr_var, llvm_type(args[i]));
        printf("store %s %%%s, %s* %%%d\n", llvm_type(args[i]), get_arg_name(node->children[0]->children[2], i), llvm_type(args[i]), curr_var);
    }

    Tree_Node *body_node = node->children[1];
    for(i = 0; i < body_node->children_count; i++) {
        Tree_Node *curr = body_node->children[i];
        get_gen_function(curr->key)(curr);
    }

    printf("ret %s ", llvm_type(return_type));
    if(!strcmp(return_type, "Int")) {
        printf("0");
    } else if(!strcmp(return_type, "Double")) {
        printf("0.0");
    } else if(!strcmp(return_type, "Bool")) {
        printf("true");
    }

    printf("\n}\n\n");
}

void gen_print(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);

    char *anno = node->children[0]->anno;
    if(!strcmp(anno, "Int")) {
        printf("call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @str.dec, i32 0, i32 0), i32 %%%d)\n", curr_var++);
    } else if(!strcmp(anno, "Double")) {
        printf("call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @str.double, i32 0, i32 0), double %%%d)\n", curr_var++);
    } else if(!strcmp(anno, "String")) {
        printf("call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([%d x i8], [%d x i8]* @str.%d, i32 0, i32 0))\n", print_llvm_str(node->children[0]->value, false) + 2, print_llvm_str(node->children[0]->value, false) + 2, curr_str++);
        curr_var++;
    } else if(!strcmp(anno, "Bool")) {
        printf("%%%d = zext i1 %%%d to i32 ;extends and fills with 0s\n", curr_var + 1, curr_var);
        curr_var++;
        printf("%%%d = mul i32 7, %%%d\n", curr_var + 1, curr_var);
        curr_var++;
        printf("%%%d = getelementptr inbounds [13 x i8], [13 x i8]* @str.bools, i32 0, i32 %%%d\n", curr_var + 1, curr_var);
        curr_var++;
        printf("call i32 (i8*, ...) @printf(i8* %%%d)\n", curr_var);
        curr_var++;
    }
}

void strip_(char *string) {
    int aux = 0;
    int aux_2 = 0;

    while(string[aux_2] != '\0') {
        if(string[aux_2] == '_') {
            aux_2++;
        } else {
            string[aux] = string[aux_2];
            aux++;
            aux_2++;
        }
    }

    string[aux] = '\0';
}

void gen_dec_lit(Tree_Node *node) {
    strip_(node->value);
    printf("%%%d = add i32 0, %s\n", ++curr_var, node->value);
}

void gen_real_lit(Tree_Node *node) {
    strip_(node->value);
    printf("%%%d = fadd double 0.0, %.16E\n", ++curr_var, atof(node->value));
}

void gen_bool_lit(Tree_Node *node) {
    printf("%%%d = trunc i32 %d to i1\n", ++curr_var, strcmp(node->value, "true") == 0 ? 1 : 0);
}

void gen_var_decl(Tree_Node *node) {
    printf("%%%s = alloca %s\n", node->children[1]->value, llvm_type(node->children[0]->key));
}

void gen_id(Tree_Node *node) {
    if(node->table_node->global) {
        printf("%%%d = load %s, %s* @"GLOBAL"%s\n", ++curr_var, llvm_type(node->anno), llvm_type(node->anno), node->value);
    } else if(node->table_node->arg) {
        printf("%%%d = load %s, %s* %%%d", ++curr_var, llvm_type(node->anno), llvm_type(node->anno), node->table_node->index);
    } else {
        printf("%%%d = load %s, %s* %%%s\n", ++curr_var, llvm_type(node->anno), llvm_type(node->anno), node->value);
    }
}

void gen_assign(Tree_Node *node) {
    get_gen_function(node->children[1]->key)(node->children[1]);

    if (!strcmp(node->children[0]->anno, "Double") && !strcmp(node->children[1]->anno, "Int")) {
        printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, curr_var);
        curr_var++;
    }

    if(node->children[0]->table_node->global) {
        printf("store %s %%%d, %s* @"GLOBAL"%s\n", llvm_type(node->anno), curr_var, llvm_type(node->children[0]->anno), node->children[0]->value);
    } else if(node->children[0]->table_node->arg) {
        // This may be trouble
        printf("store %s %%%d, %s* %%%d\n", llvm_type(node->anno), curr_var, llvm_type(node->children[0]->anno), node->children[0]->table_node->index);
    } else {
        printf("store %s %%%d, %s* %%%s\n", llvm_type(node->anno), curr_var, llvm_type(node->children[0]->anno), node->children[0]->value);
    }
}

void gen_parse_args(Tree_Node *node) {
    get_gen_function(node->children[1]->key)(node->children[1]);
    printf("%%%d = add i32 %%%d, 1\n", curr_var + 1, curr_var);
    curr_var++;
    printf("%%%d = getelementptr inbounds i8*, i8** %%"ARGS"argv, i32 %%%d\n", curr_var + 1, curr_var);
    curr_var++;
    printf("%%%d = load i8*, i8** %%%d\n", curr_var + 1, curr_var);
    curr_var++;
    printf("%%%d = call i32 @atoi(i8* %%%d)\n", curr_var + 1, curr_var);
    curr_var++;
}

void gen_dot_length(Tree_Node *node) {
    printf("%%%d = sub i32 %%"ARGS"argc, 1\n", ++curr_var);
    printf("%%%d = add i32 0, %%%d\n", curr_var + 1, curr_var);
    curr_var++;
}

void gen_minus(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);

    if(!strcmp(node->children[0]->anno, "Int")) {
        printf("%%%d = sub i32 0, %%%d\n", curr_var + 1, curr_var);
        curr_var++;
    } else {
        printf("%%%d = fsub double -0.000000e+00, %%%d\n", curr_var + 1, curr_var);
        curr_var++;
    }
}

void gen_plus(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
}

void gen_not(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    printf("%%%d = zext i1 %%%d to i32\n", curr_var + 1, curr_var);
    curr_var++;
    printf("%%%d = icmp eq i32 %%%d, 0\n", curr_var + 1, curr_var);
    curr_var++;
}

void gen_eq(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = fcmp oeq double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else if(!strcmp(anno_1, "Int")) {
        printf("%%%d = icmp eq i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = zext i1 %%%d to i32\n", curr_var + 1, param_1);
        curr_var++;
        printf("%%%d = zext i1 %%%d to i32\n", curr_var + 1, param_2);
        curr_var++;
        printf("%%%d = icmp eq i32 %%%d, %%%d\n", curr_var + 1, curr_var, curr_var-1);
        curr_var++;
    }
}

void gen_ne(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = fcmp une double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else if(!strcmp(anno_1, "Int")) {
        printf("%%%d = icmp ne i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = zext i1 %%%d to i32\n", curr_var + 1, param_1);
        curr_var++;
        printf("%%%d = zext i1 %%%d to i32\n", curr_var + 1, param_2);
        curr_var++;
        printf("%%%d = icmp ne i32 %%%d, %%%d\n", curr_var + 1, curr_var, curr_var-1);
        curr_var++;
    }
}

void gen_lt(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = fcmp olt double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = icmp slt i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    }
}

void gen_le(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = fcmp ole double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = icmp sle i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    }
}

void gen_gt(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = fcmp ogt double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = icmp sgt i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    }
}

void gen_ge(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = fcmp oge double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = icmp sge i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    }
}

void gen_add(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = fadd double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = add i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    }
}

void gen_sub(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = fsub double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = sub i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    }
}

void gen_mult(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = fmul double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = mul i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    }
}

void gen_div(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = fdiv double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = sdiv i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    }
}

void gen_mod(Tree_Node *node) {
    get_gen_function(node->children[0]->key)(node->children[0]);
    int param_1 = curr_var;
    get_gen_function(node->children[1]->key)(node->children[1]);
    int param_2 = curr_var;

    char *anno_1 = node->children[0]->anno;
    char *anno_2 = node->children[1]->anno;
    if(!strcmp(anno_1, "Double") || !strcmp(anno_2, "Double")) {
        if(!strcmp(anno_1, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_1);
            param_1 = ++curr_var;
        }

        if(!strcmp(anno_2, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, param_2);
            param_2 = ++curr_var;
        }

        printf("%%%d = frem double %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    } else {
        printf("%%%d = srem i32 %%%d, %%%d\n", curr_var + 1, param_1, param_2);
        curr_var++;
    }
}

void gen_block(Tree_Node *node) {
    int i;
    for(i = 0; i < node->children_count; i++) {
        if(node->children[i]) {
            get_gen_function(node->children[i]->key)(node->children[i]);
        }
    }
}

void gen_or(Tree_Node *node) {
    int labels = reserved_label + 1;
    reserved_label += 2;

    get_gen_function(node->children[0]->key)(node->children[0]);
    int last = used_label;

    printf("br i1 %%%d, label %%"LABEL"%d, label %%"LABEL"%d\n", curr_var, labels + 1, labels);

    printf("\n"LABEL"%d:\n", labels);
    used_label = labels;
    get_gen_function(node->children[1]->key)(node->children[1]);

    int last_2 = used_label;
    printf("br label %%"LABEL"%d\n", labels + 1);

    printf("\n"LABEL"%d:\n", labels + 1);
    used_label = labels + 1;
    printf("%%%d = phi i1 [ true, %%"LABEL"%d ], [ %%%d, %%"LABEL"%d ]\n", curr_var + 1, last, curr_var, last_2);
    curr_var++;
}

void gen_and(Tree_Node *node) {
    int labels = reserved_label + 1;
    reserved_label += 2;

    get_gen_function(node->children[0]->key)(node->children[0]);
    int last = used_label;

    printf("br i1 %%%d, label %%"LABEL"%d, label %%"LABEL"%d\n", curr_var, labels + 1, labels);

    printf("\n"LABEL"%d:\n", labels);
    used_label = labels;
    get_gen_function(node->children[1]->key)(node->children[1]);

    int last_2 = used_label;
    printf("br label %%"LABEL"%d\n", labels + 1);

    printf("\n"LABEL"%d:\n", labels + 1);
    used_label = labels + 1;
    printf("%%%d = phi i1 [ false, %%"LABEL"%d ], [ %%%d, %%"LABEL"%d ]\n", curr_var + 1, last, curr_var, last_2);
    curr_var++;
}

void gen_while(Tree_Node *node) {
    int labels = reserved_label + 1;
    reserved_label += 3;

    printf("br label %%"LABEL"%d\n", labels);
    used_label = labels;
    printf("\n"LABEL"%d:\n", labels);
    used_label = labels;

    get_gen_function(node->children[0]->key)(node->children[0]);
    printf("br i1 %%%d, label %%"LABEL"%d, label %%"LABEL"%d\n", curr_var, labels + 1, labels + 2);

    printf("\n"LABEL"%d:\n", labels + 1);
    used_label = labels + 1;

    get_gen_function(node->children[1]->key)(node->children[1]);
    printf("br label %%"LABEL"%d\n", labels);
    used_label = labels;

    printf("\n"LABEL"%d:\n", labels + 2);
    used_label = labels + 2;
}

void gen_if(Tree_Node *node) {
    int labels = reserved_label + 1;
    reserved_label += 3;

    get_gen_function(node->children[0]->key)(node->children[0]);
    printf("br i1 %%%d, label %%"LABEL"%d, label %%"LABEL"%d\n", curr_var, labels, labels + 1);

    printf("\n"LABEL"%d:\n", labels);
    used_label = labels;

    get_gen_function(node->children[1]->key)(node->children[1]);
    printf("br label %%"LABEL"%d\n", labels + 2);
    used_label = labels + 2;


    printf("\n"LABEL"%d:\n", labels + 1);
    used_label = labels + 1;
    get_gen_function(node->children[2]->key)(node->children[2]);
    printf("br label %%"LABEL"%d\n", labels + 2);
    used_label = labels + 2;


    printf("\n"LABEL"%d:\n", labels + 2);
    used_label = labels + 2;
}

void gen_call(Tree_Node *node) {
    int arg_count = node->children[0]->env->arg_count;

    if(arg_count == 1 && !strcmp(node->children[1]->anno, "StringArray")) {
        if(strcmp(node->children[0]->env->return_type, "Void")) {
            printf("%%%d = ", curr_var + 1);
            curr_var++;
        }

        printf("call %s @"FUNC"%s", llvm_type(node->children[0]->env->return_type), node->children[0]->env->method_name);

        int i;
        for(i = 0; i < arg_count; i++) {
            printf(".%s", node->children[0]->env->arg_types[i]);
        }

        printf("(i32 %%"ARGS"argc, i8** %%"ARGS"argv)\n");
        return;
    }

    int *tmp_args = (int*) malloc(arg_count * sizeof(int));

    int i, count = 0;
    for(i = 1; i < node->children_count; i++) {
        if(!node->children[i]) continue;

        get_gen_function(node->children[i]->key)(node->children[i]);
        if(!strcmp(node->children[i]->anno, "Int") && !strcmp(node->children[0]->env->arg_types[count], "Double")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, curr_var);
            curr_var++;
        }

        tmp_args[count++] = curr_var;
    }

    if(strcmp(node->children[0]->env->return_type, "Void")) {
        printf("%%%d = ", curr_var + 1);
        curr_var++;
    }

    printf("call %s @"FUNC"%s", llvm_type(node->children[0]->env->return_type), node->children[0]->env->method_name);

    for(i = 0; i < arg_count; i++) {
        printf(".%s", node->children[0]->env->arg_types[i]);
    }

    printf("(");
    for(i = 0; i < arg_count; i++) {
        if(i) {
            printf(", ");
        }
        printf("%s %%%d", llvm_type(node->children[0]->env->arg_types[i]), tmp_args[i]);
    }
    printf(")\n");

    free(tmp_args);
}

void gen_return(Tree_Node *node) {
    if(node->children_count == 1) {
        get_gen_function(node->children[0]->key)(node->children[0]);

        if(!strcmp(node->return_anno, "Double") && !strcmp(node->children[0]->anno, "Int")) {
            printf("%%%d = sitofp i32 %%%d to double\n", curr_var + 1, curr_var);
            curr_var++;
        }
    } else {
        printf("ret void\n");
    }
}

void skip(Tree_Node *node) {}

Method_gen_type get_gen_function(char *name) {
    if(strcmp(name, "Print") == 0) {
        return gen_print;
    } else if(strcmp(name, "DecLit") == 0) {
        return gen_dec_lit;
    } else if(strcmp(name, "RealLit") == 0) {
        return gen_real_lit;
    } else if(strcmp(name, "BoolLit") == 0) {
        return gen_bool_lit;
    } else if(strcmp(name, "VarDecl") == 0) {
        return gen_var_decl;
    } else if(strcmp(name, "Id") == 0) {
        return gen_id;
    } else if(strcmp(name, "Assign") == 0) {
        return gen_assign;
    } else if (strcmp(name, "Length") == 0){
        return gen_dot_length;
    } else if(strcmp(name, "ParseArgs") == 0) {
        return gen_parse_args;
    } else if(strcmp(name, "Minus") == 0) {
        return gen_minus;
    } else if(strcmp(name, "Or") == 0) {
        return gen_or;
    } else if(strcmp(name, "And") == 0) {
        return gen_and;
    } else if(strcmp(name, "Eq") == 0) {
        return gen_eq;
    } else if(strcmp(name, "Ne") == 0) {
        return gen_ne;
    } else if (strcmp(name, "Lt") == 0){
        return gen_lt;
    } else if(strcmp(name, "Le") == 0){
        return gen_le;
    } else if(strcmp(name, "Gt") == 0){
        return gen_gt;
    } else if(strcmp(name, "Ge") == 0){
        return gen_ge;
    } else if(strcmp(name, "Plus") == 0) {
        return gen_plus;
    } else if(strcmp(name, "Add") == 0) {
        return gen_add;
    } else if(strcmp(name, "Sub") == 0) {
        return gen_sub;
    } else if(strcmp(name, "Mul") == 0) {
        return gen_mult;
    } else if(strcmp(name, "Div") == 0) {
        return gen_div;
    } else if(strcmp(name, "Mod") == 0) {
        return gen_mod;
    } else if(strcmp(name, "Not") == 0) {
        return gen_not;
    } else if(strcmp(name, "Block") == 0) {
        return gen_block;
    } else if(strcmp(name, "While") == 0) {
        return gen_while;
    } else if(strcmp(name, "If") == 0) {
        return gen_if;
    } else if(strcmp(name, "Call") == 0) {
        return gen_call;
    } else if(strcmp(name, "Return") == 0) {
        return gen_return;
    }

    return skip;
}

int print_llvm_str(char *str, bool flag) {
    int len = 0;
    int curr = 0;

    while(str[curr] != '\0') {
        if(str[curr] == '\"') {
            curr++;
        } else if(str[curr] == '%') {
            if(flag) printf("%%%%");
            curr++;
            len += 2;
        } else if(str[curr] == '\\') {
            if(str[curr + 1] == 'n') {
                if(flag) printf("\\0A");
            } else if(str[curr + 1] == 't') {
                if(flag) printf("\\09");
            } else if(str[curr + 1] == 'f') {
                if(flag) printf("\\0C");
            } else if(str[curr + 1] == 'r') {
                if(flag) printf("\\0D");
            } else if(str[curr + 1] == '"') {
                if(flag) printf("\\22");
            } else {
                if(flag) printf("\\5C");
            }

            len++;
            curr += 2;
        } else {
            if(flag) putchar(str[curr]);
            len++;
            curr++;
        }
    }

    return len;
}

void find_static_str(Tree_Node *root) {
    int i;
    for(i = 0; i < root->children_count; i++) {
        if(!root->children[i]) continue;

        if(!strcmp(root->children[i]->key, "StrLit")) {
            int count = print_llvm_str(root->children[i]->value, false);
            printf("@"STR"%d = private unnamed_addr constant [%d x i8] c\"", curr_str++, count + 2);

            print_llvm_str(root->children[i]->value, true);
            printf("\\0A");
            printf("\\00\"\n");
        } else {
            find_static_str(root->children[i]);
        }
    }
}

void variable_allocation(Table_node *node, bool global) {
    if(global) {
        printf("@"GLOBAL"%s = global %s 0", node->name, llvm_type(node->type));

        if(!strcmp(node->type, "Double")) {
            printf(".0");
        }
    } else {
        printf("%%"LOCAL"%s = alloca %s", node->name, llvm_type(node->type));
    }
    printf("\n");
}

void gen_class_variables(Tree_Node *root, Class_env *env) {
    Table_node *curr = env->variables;

    while(curr) {
        variable_allocation(curr, true);
        curr = curr->next;
    }
}

char *llvm_type(char *type) {
    if(!strcmp(type, "Int")) {
        return "i32";
    }
    else if(!strcmp(type, "Double")) {
        return "double";
    }
    else if(!strcmp(type, "Bool")) {
        return "i1";
    }
    else if(!strcmp(type, "Void")) {
        return "void";
    }
    else if(!strcmp(type, "String")) {
        return "i8*";
    }
    else if(!strcmp(type, "StringArray")) {
        return "i8**";
    }
    return "FAILURE IN GET_LLVM_TYPE";
}