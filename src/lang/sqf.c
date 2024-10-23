#include <lang/sqf.h>

int32_t get_field_strict_offset(type_t* type, char* name) {
    symbol_t* sym = type->fields;
    uint32_t offset = 0;

    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return offset;

        offset++;
        sym = sym->next;
    }

    return -1;
}

void sqf_gen_list(generator_t* gen, ast_node_t* node) {
    if (!node)
        return;

    gen->indent++;

    ast_node_t* current = node;
    while (current) {
        generator_put(gen, "\n");
        generator_print_indent(gen);
        generator_generate(gen, current);
        current = current->next;
        generator_put(gen, ";");
    }

    gen->indent--;
    generator_put(gen, "\n");
    generator_print_indent(gen);
}

void sqf_gen_params(generator_t* gen, ast_node_t* node) {
    gen->indent++;
    generator_print_indent(gen);
    gen->indent--;
    generator_put(gen, "params [");

    ast_node_t* current = node;
    while (current) {
        generator_put(gen, "\"_%s\"", current->decl.name);
        current = current->next;

        if (current)
            generator_put(gen, ", ");
    }

    generator_put(gen, "]");
}

void sqf_gen_array(generator_t* gen, ast_node_t* node) {
    generator_put(gen, "[");

    ast_node_t* current = node;
    while (current) {
        generator_generate(gen, current);
        current = current->next;

        if (current)
            generator_put(gen, ", ");
    }

    generator_put(gen, "]");
}

void sqf_gen_member_access_get(generator_t* gen, ast_node_t* node) {

}

void sqf_gen_member_access_set(generator_t* gen, ast_node_t* node, ast_node_t* value) {
    if (node->type == AST_EXPRESSION) {
        generator_put(gen, "(");
        generator_generate(gen, node);
    }
}

void sqf_gen_assign(generator_t* gen, ast_node_t* node) {
    if (node->l->type == AST_EXPRESSION) {
        switch (node->l->op->type) {
            case OPERATOR_DOT:
                sqf_gen_member_access_set(gen, node->l, node->r);
                break;

            default:
                generator_generate(gen, node->l);
                generator_put(gen, " %s ", node->op->name);
                generator_generate(gen, node->r);
        }
    }
}

void sqf_gen_keyword(generator_t* gen, ast_node_t* node) {
    switch (node->kw->type) {
        case KEYWORD_RETURN:
            generator_generate(gen, node->args);
            break;

        case KEYWORD_STRUCT:
            break;

        default:
            printf("Unknown keyword: %d\n", node->kw->type);
            break;
    }
}

void sqf_gen_operator(generator_t* gen, ast_node_t* node) {
    // For custom operators
}

void sqf_gen_expression(generator_t* gen, ast_node_t* node) {
    switch (node->op->type) {
        case OPERATOR_DOT:
            sqf_gen_member_access_get(gen, node);
            break;

        case OPERATOR_ASSIGN:
            sqf_gen_assign(gen, node);
            break;

        default:
            generator_gen_expression(gen, node);
    }
}

void sqf_gen_var_decl(generator_t* gen, ast_node_t* node) {
    generator_put(gen, "private _%s", node->decl.name);
}

void sqf_gen_func_decl(generator_t* gen, ast_node_t* node) {
    generator_put(gen, "private _%s", node->decl.name);
    generator_put(gen, " = {\n");
    sqf_gen_params(gen, node->args);
    generator_put(gen, ";");
    sqf_gen_list(gen, node->body);
    generator_put(gen, "}");
}

void sqf_gen_block(generator_t* gen, ast_node_t* node) {
    sqf_gen_list(gen, node->body);
}

void sqf_gen_identifier(generator_t* gen, ast_node_t* node) {
    generator_put(gen, "_%s", node->id);
}

void sqf_gen_constant(generator_t* gen, ast_node_t* node) {
    switch (node->cnst.type->primitive) {
        case PRIMITIVE_TYPE_NUMBER:
            generator_put(gen, "%d", node->cnst.num);
            break;

        case PRIMITIVE_TYPE_STRING:
            generator_put(gen, "\"%s\"", node->cnst.str);
            break;

        default:
            printf("Unknown constant type: %d\n", node->cnst.type->primitive);
    }
}

void sqf_gen_call(generator_t* gen, ast_node_t* node) {
    sqf_gen_array(gen, node->args);
    generator_put(gen, " call _%s", node->id);
}

language_generator_t sqf_generator = {
    .gen_keyword        = sqf_gen_keyword,
    .gen_operator       = sqf_gen_operator,
    .gen_expression     = sqf_gen_expression,
    .gen_var_decl       = sqf_gen_var_decl,
    .gen_func_decl      = sqf_gen_func_decl,
    .gen_block          = sqf_gen_block,
    .gen_identifier     = sqf_gen_identifier,
    .gen_constant       = sqf_gen_constant,
    .gen_call           = sqf_gen_call,
};