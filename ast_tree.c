#include "ast_tree.h"

Tree_Node *new_node(char *key, char *value) {
	Tree_Node *new_node = (Tree_Node*) malloc(sizeof(Tree_Node));

    if(!new_node) {
        return NULL;
    }

	new_node->key = strdup(key);
    new_node->value = value == NULL ? value : strdup(value);

    new_node->children_count = 0;
    new_node->children = NULL;

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
        field_decl_tree(program, second_node->children[2], second_node->children[0]->key);
        second_node->children[2] = NULL;
    }

    if(!strcmp(second_node->key,"ProgramRec")) {
        int i;
        for(i = 0; i < second_node->children_count; i++) {
            program_tree(program, second_node->children[i]);
        }
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
        add_child(method_body, second_node);

        if(!strcmp(second_node->key, "VarDecl")) {
            var_decl_tree(method_body, second_node->children[2], second_node->children[0]->key);
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

    Tree_Node *param_decl = new_node("ParamDecl", NULL);
    add_child(param_decl, second_node->children[0]);
    add_child(param_decl, second_node->children[1]);
    add_child(method_params, param_decl);

    method_params_tree(method_params, second_node->children[2]); 
}

void field_decl_tree(Tree_Node *program, Tree_Node *second_node, char *field_type) {
    if(!second_node) {
        return;
    }

    Tree_Node *field_type_node = new_node(field_type, NULL);
    Tree_Node *id = second_node->children[0];
    Tree_Node *field_decl = new_node("FieldDecl", NULL);

    add_child(field_decl, field_type_node);
    add_child(field_decl, id);
    add_child(program, field_decl);

    field_decl_tree(program, second_node->children[1], field_type);
}

void var_decl_tree(Tree_Node *method_body, Tree_Node *second_node, char *var_type) {
    if(!second_node) {
        return;
    }

    Tree_Node *var_type_node = new_node(var_type, NULL);
    Tree_Node *id = second_node->children[0];
    Tree_Node *var_decl = new_node("VarDecl", NULL);

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
        add_child(statement, second_node->children[0]);
        statement_tree(statement, second_node->children[1]);
        second_node->children[1] = NULL;
    }
}

void call_tree(Tree_Node *call, Tree_Node *second_node) {
    if(!second_node) {
        return;
    }

    add_child(call, second_node->children[0]);
    call_tree(call, second_node->children[1]);
}

void destroy_tree(Tree_Node *root) {
    if(!root) {
        return;
    }

    int i;
    for(i = 0; i < root->children_count; i++) {
        if(root->children[i])
            continue;
        destroy_tree(root->children[i]);
    }

    free(root->children);
    free(root->key);
    free(root);
}
