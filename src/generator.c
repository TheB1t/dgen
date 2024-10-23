#include <lang.h>
#include <stdarg.h>

void generator_put(generator_t* gen, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int32_t wrote = vsprintf(gen->cur, format, args);
    va_end(args);

    if (wrote < 0) {
        fprintf(stderr, "Error writing to buffer\n");
        exit(1);
    }

    gen->cur += wrote;
    gen->cur[0] = '\0';
}

void generator_print_indent(generator_t* gen) {
    if (gen->indent < 0)
        return;

    for (uint32_t i = 0; i < gen->indent * gen->indent_size; i++)
        generator_put(gen, " ");
}

void generator_gen_expression(generator_t* gen, ast_node_t* node) {
    switch (node->op->type) {
        case OPERATOR_CUSTOM_BINARY:
        case OPERATOR_CUSTOM_UNARY:
            if (gen->lang->gen_operator)
                gen->lang->gen_operator(gen, node);
            else
                printf("No generator for operators\n");

            break;

        case OPERATOR_INC:
        case OPERATOR_DEC:
        case OPERATOR_NOT:
        case OPERATOR_BITWISE_NOT:
            generator_put(gen, "%s", node->op->name);
            generator_generate(gen, node->l);
            break;

        case OPERATOR_ASSIGN:
            generator_generate(gen, node->l);
            generator_put(gen, " %s ", node->op->name);
            generator_generate(gen, node->r);
            break;

        default:
            generator_put(gen, "(");
            generator_generate(gen, node->l);
            generator_put(gen, " %s ", node->op->name);
            generator_generate(gen, node->r);
            generator_put(gen, ")");
    }
}

generator_t* generator_create(language_generator_t* lang, char* buf, uint32_t buf_size) {
    NEW(gen, generator_t);

    gen->lang = lang;
    gen->indent = -1;
    gen->indent_size = 4;
    gen->cur = buf;
    gen->buf_size = buf_size;
    gen->cur[0] = '\0';

    return gen;
}

void generator_generate(generator_t* gen, ast_node_t* node) {
    if (!node)
        return;

    switch (node->type) {
        case AST_ROOT:
            if (gen->lang->gen_block)
                gen->lang->gen_block(gen, node);
            else
                printf("No generator for blocks\n");

            break;

        case AST_KEYWORD:
            if (gen->lang->gen_keyword)
                gen->lang->gen_keyword(gen, node);
            else
                printf("No generator for keywords\n");

            break;

        case AST_EXPRESSION:
            if (gen->lang->gen_expression)
                gen->lang->gen_expression(gen, node);
            else
                generator_gen_expression(gen, node);
            break;

        case AST_DECL_VARIABLE:
            if (gen->lang->gen_var_decl)
                gen->lang->gen_var_decl(gen, node);
            else
                printf("No generator for variable declarations\n");

            break;

        case AST_DECL_FUNCTION:
            if (gen->lang->gen_func_decl)
                gen->lang->gen_func_decl(gen, node);
            else
                printf("No generator for function declarations\n");

            break;

        case AST_IDENTIFIER:
            if (gen->lang->gen_identifier)
                gen->lang->gen_identifier(gen, node);
            else
                printf("No generator for identifiers\n");

            break;

        case AST_CONSTANT:
            if (gen->lang->gen_constant)
                gen->lang->gen_constant(gen, node);
            else
                printf("No generator for constants\n");

            break;

        case AST_FUNCTION_CALL:
            if (gen->lang->gen_call)
                gen->lang->gen_call(gen, node);
            else
                printf("No generator for function calls\n");

            break;

        default:
            fprintf(stderr, "Unknown node type %s\n", AST_NODE_STR[node->type]);
            exit(1);
    }
}