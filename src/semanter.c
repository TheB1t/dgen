#include <semanter.h>
#include <utils.h>

void sem_validate_member_access(language_t* lang, type_t* root, ast_node_t* node) {
    if (!node)
        return;

    symbol_t* object = NULL;

    if (root) {
        object = lang_find_field_in_struct(root, node->l->id);
    } else {
        object = lang_find_symbol(lang, node->l->id);
    }

    if (!object) {
        printf("Symbol %s not found\n", node->l->id);
        exit(1);
    }

    if (object->type_info->type != TYPE_TYPE_STRUCT) {
        printf("Symbol %s is not a struct\n", node->l->id);
        exit(1);
    }

    if (node->r->type == AST_EXPRESSION && node->r->op->type == OPERATOR_DOT) {
        sem_validate_member_access(lang, object->type_info, node->r);
    } else {
        symbol_t* field = lang_find_field_in_struct(object->type_info, node->r->id);

        if (!field) {
            printf("Field %s not found\n", node->r->id);
            exit(1);
        }
    }
}

void sem_validate_expression(language_t* lang, ast_node_t* node) {
    if (!node)
        return;

    switch (node->op->type) {
        case OPERATOR_INC:
        case OPERATOR_DEC:
        case OPERATOR_NOT:
        case OPERATOR_BITWISE_NOT:
        case OPERATOR_CUSTOM_UNARY:
            sem_validate_node(lang, node->l);
            break;

        case OPERATOR_DOT:
            sem_validate_member_access(lang, NULL, node);
            break;

        default:
            sem_validate_node(lang, node->l);
            sem_validate_node(lang, node->r);
            break;
    }
}

void sem_validate_node_list(language_t* lang, ast_node_t* node) {
    if (!node)
        return;

    ast_node_t* current = node;
    while (current) {
        sem_validate_node(lang, current);
        current = current->next;
    }
}

void sem_validate_function_declaration(language_t* lang, ast_node_t* node) {
    if (!node)
        return;

    symbol_t* symbol = lang_add_symbol(lang, node->decl.name, SYMBOL_TYPE_FUNCTION, node->decl.type);
    if (!symbol) {
        SEMANTER_LOG("Error: Function '%s' already declared", node->decl.name);
        exit(1);
    }

    lang_scope_enter(lang);
    sem_validate_node_list(lang, node->args);
    sem_validate_node_list(lang, node->body);
    lang_scope_leave(lang);
}

void sem_validate_function_call(language_t* lang, ast_node_t* node) {
    if (!node)
        return;

    symbol_t* symbol = lang_find_symbol(lang, node->id);

    if (!symbol || symbol->type != SYMBOL_TYPE_FUNCTION) {
        SEMANTER_LOG("Error: Function '%s' not found", node->id);
        exit(1);
    }


}

void sem_validate_node(language_t* lang, ast_node_t* node) {
    if (!node)
        return;

    symbol_t* symbol = NULL;
    // print_node(node, 0);

    switch (node->type) {
        case AST_ROOT:
            lang_scope_enter(lang);
            sem_validate_node_list(lang, node->body);
            lang_scope_leave(lang);
            break;

        case AST_DECL_VARIABLE:
            symbol = lang_add_symbol(lang, node->decl.name, SYMBOL_TYPE_VARIABLE, node->decl.type);
            if (!symbol) {
                SEMANTER_LOG("Error: Variable '%s' already declared", node->decl.name);
                exit(1);
            }
            break;

        case AST_DECL_FUNCTION:
            sem_validate_function_declaration(lang, node);
            break;

        case AST_FUNCTION_CALL:
            sem_validate_function_call(lang, node);
            break;

        case AST_ARRAY_ACCESS:
            // TODO: Check if array exists
            break;

        case AST_EXPRESSION:
            sem_validate_expression(lang, node);
            break;

        case AST_KEYWORD:
            lang_scope_enter(lang);
            if (node->kw->validate_node)
                node->kw->validate_node(lang, node);

            sem_validate_node_list(lang, node->args);
            sem_validate_node_list(lang, node->body);
            lang_scope_leave(lang);
            break;

        case AST_IDENTIFIER:
            symbol = lang_find_symbol(lang, node->id);
            if (!symbol) {
                SEMANTER_LOG("Error: Identifier '%s' not found", node->id);
                exit(1);
            }
            break;

        case AST_ARRAY:
            // TODO: Check if array exists
            break;

        case AST_CONSTANT:
            break; // Always valid

        default:
            SEMANTER_LOG("Error: Semanter not supporting node type %s yet", AST_NODE_STR[node->type]);
            exit(1);
    }
}