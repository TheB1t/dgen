#include <lang/dgen.h>

int dgen_struct_init_node(language_t* lang, ast_node_t* node) {
    type_t* type = lang_add_struct(lang, node->id);

    ast_node_t* current = node->body;
    while (current) {
        if (current->type == AST_DECL_VARIABLE) {

            symbol_t* sym = lang_struct_add_field(type, current->decl.name);
            sym->type_info = current->decl.type;
        } else {
            printf("Unknown node type %d\n", current->type);
        }

        current = current->next;
    }
    return 0;
}

keyword_t keywords[] = {
    {
        "if", KEYWORD_IF, {
            STMT_PART_PARENT_OPEN,
            STMT_PART_EXPRESSION,
            STMT_PART_PARENT_CLOSE,
            STMT_PART_SCOPE_OPEN,
            STMT_PART_BLOCK,
            STMT_PART_SCOPE_CLOSE,
        }, 6, NULL, NULL,
    }, {
        "else", KEYWORD_ELSE,{
            STMT_PART_SCOPE_OPEN,
            STMT_PART_BLOCK,
            STMT_PART_SCOPE_CLOSE,
        }, 3, NULL, NULL,
    }, {
        "while", KEYWORD_WHILE, {
            STMT_PART_PARENT_OPEN,
            STMT_PART_EXPRESSION,
            STMT_PART_PARENT_CLOSE,
            STMT_PART_SCOPE_OPEN,
            STMT_PART_BLOCK,
            STMT_PART_SCOPE_CLOSE,
        }, 6, NULL, NULL,
    }, {
        "for", KEYWORD_FOR, {
            STMT_PART_PARENT_OPEN,
            STMT_PART_EXPRESSION,
            STMT_PART_EXPRESSION,
            STMT_PART_EXPRESSION,
            STMT_PART_PARENT_CLOSE,
            STMT_PART_SCOPE_OPEN,
            STMT_PART_BLOCK,
            STMT_PART_SCOPE_CLOSE,
        }, 8, NULL, NULL,
    }, {
        "switch", KEYWORD_SWITCH, {
            STMT_PART_PARENT_OPEN,
            STMT_PART_EXPRESSION,
            STMT_PART_PARENT_CLOSE,
            STMT_PART_SCOPE_OPEN,
            STMT_PART_BLOCK,
            STMT_PART_SCOPE_CLOSE,
        }, 6, NULL, NULL,
    }, {
        "case", KEYWORD_CASE, {
            STMT_PART_EXPRESSION,
            STMT_PART_COLON,
            STMT_PART_SCOPE_OPEN,
            STMT_PART_BLOCK,
            STMT_PART_SCOPE_CLOSE,
        }, 5, NULL, NULL,
    }, {
        "default", KEYWORD_DEFAULT, {
            STMT_PART_COLON,
            STMT_PART_SCOPE_OPEN,
            STMT_PART_BLOCK,
            STMT_PART_SCOPE_CLOSE,
        }, 4, NULL, NULL,
    }, {
        "return", KEYWORD_RETURN, {
            STMT_PART_EXPRESSION
        }, 1, NULL, NULL,
    }, {
        "break", KEYWORD_BREAK, {}, 0, NULL, NULL,
    }, {
        "continue", KEYWORD_CONTINUE, {}, 0, NULL, NULL,
    }, {
        "true", KEYWORD_TRUE, {}, 0, NULL, NULL,
    }, {
        "false", KEYWORD_FALSE, {}, 0, NULL, NULL,
    }, {
        "struct", KEYWORD_STRUCT, {
            STMT_PART_IDENTIFIER,
            STMT_PART_SCOPE_OPEN,
            STMT_PART_BLOCK,
            STMT_PART_SCOPE_CLOSE,
        }, 4, dgen_struct_init_node, NULL,
    }
};

operator_t operators[] = {
    { "in", OPERATOR_CUSTOM_BINARY, 1 },

    { ".", OPERATOR_DOT, 2 },

    { "++", OPERATOR_INC, 2 },
    { "--", OPERATOR_DEC, 2 },
    { "!", OPERATOR_NOT, 2 },
    { "~", OPERATOR_BITWISE_NOT, 2 },

    { "*", OPERATOR_MUL, 3 },
    { "/", OPERATOR_DIV, 3 },
    { "%", OPERATOR_MOD, 3 },

    { "+", OPERATOR_ADD, 4 },
    { "-", OPERATOR_SUB, 4 },

    { "<", OPERATOR_LESS, 6 },
    { ">", OPERATOR_GREATER, 6 },
    { "<=", OPERATOR_LESS_EQUAL, 6 },
    { ">=", OPERATOR_GREATER_EQUAL, 6 },

    { "==", OPERATOR_EQUAL, 7 },
    { "!=", OPERATOR_NOT_EQUAL, 7 },

    { "&&", OPERATOR_AND, 11 },
    { "||", OPERATOR_OR, 12 },

    { "=", OPERATOR_ASSIGN, 14 },
};

stmt_decl_part_e func_decl[] = {
    STMT_PART_PARENT_OPEN,
    STMT_PART_EXPRESSIONS,
    STMT_PART_PARENT_CLOSE,
    STMT_PART_SCOPE_OPEN,
    STMT_PART_BLOCK,
    STMT_PART_SCOPE_CLOSE,
};

language_t* get_dgen_language() {
    language_t* lang = lang_create();

    lang->name = "dGen";

    lang->keywords = keywords;
    lang->keyword_size = sizeof(keywords) / sizeof(keyword_t);

    lang->operators = operators;
    lang->operator_size = sizeof(operators) / sizeof(operator_t);

    lang->func_decl = func_decl;
    lang->func_decl_size = sizeof(func_decl) / sizeof(stmt_decl_part_e);

    lang_add_primitive_type(lang, "number", PRIMITIVE_TYPE_NUMBER);
    lang_add_primitive_type(lang, "string", PRIMITIVE_TYPE_STRING);
    lang_add_primitive_type(lang, "boolean", PRIMITIVE_TYPE_BOOLEAN);
    lang_add_primitive_type(lang, "void", PRIMITIVE_TYPE_VOID);
    lang_add_primitive_type(lang, "object", PRIMITIVE_TYPE_OBJECT);

    lang_add_type(lang, "vec3", "number", 3);

    return lang;
}