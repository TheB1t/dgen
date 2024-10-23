#pragma once

#include <common.h>

#define MAX_STMT_PARTS 16

DECL_ENUM(operator);
DECL_STRUCT(operator);
DECL_ENUM(stmt_decl_part);
DECL_ENUM(keyword);
DECL_STRUCT(keyword);
DECL_ENUM(type);
DECL_ENUM(primitive_type);
DECL_STRUCT(type);
DECL_STRUCT(scope);
DECL_ENUM(symbol_type);
DECL_STRUCT(symbol);
DECL_STRUCT(metadata);
DECL_STRUCT(ast_node);
DECL_ENUM(ast_node_type);
DECL_STRUCT(generator);
DECL_STRUCT(language_generator);
DECL_STRUCT(language);

ENUM(operator) {
    OPERATOR_CUSTOM_UNARY,
    OPERATOR_CUSTOM_BINARY,

    OPERATOR_DOT,

    OPERATOR_ADD,
    OPERATOR_SUB,
    OPERATOR_MUL,
    OPERATOR_DIV,
    OPERATOR_MOD,
    OPERATOR_POW,
    OPERATOR_INC,
    OPERATOR_DEC,

    OPERATOR_AND,
    OPERATOR_OR,
    OPERATOR_NOT,

    OPERATOR_BITWISE_AND,
    OPERATOR_BITWISE_OR,
    OPERATOR_BITWISE_XOR,
    OPERATOR_BITWISE_NOT,

    OPERATOR_LESS,
    OPERATOR_GREATER,
    OPERATOR_LESS_EQUAL,
    OPERATOR_GREATER_EQUAL,
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,

    OPERATOR_ASSIGN,
    OPERATOR_ASSIGN_ADD,
    OPERATOR_ASSIGN_SUB,
    OPERATOR_ASSIGN_MUL,
    OPERATOR_ASSIGN_DIV,
    OPERATOR_ASSIGN_MOD,
};

STRUCT(operator) {
    char            name[MAX_NAME_SIZE];
    operator_e      type;
    uint32_t        precedence;
};

ENUM(stmt_decl_part) {
    STMT_PART_PARENT_OPEN,
    STMT_PART_PARENT_CLOSE,
    STMT_PART_SCOPE_OPEN,
    STMT_PART_SCOPE_CLOSE,
    STMT_PART_COMMA,
    STMT_PART_COLON,
    STMT_PART_SEMICOLON,
    STMT_PART_EXPRESSION,
    STMT_PART_EXPRESSIONS,
    STMT_PART_BLOCK,
    STMT_PART_IDENTIFIER,
};

ENUM(keyword) {
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_FOR,
    KEYWORD_RETURN,
    KEYWORD_BREAK,
    KEYWORD_CONTINUE,
    KEYWORD_STRUCT,
    KEYWORD_CONST,
    KEYWORD_VAR,
    KEYWORD_SWITCH,
    KEYWORD_CASE,
    KEYWORD_DEFAULT,
    KEYWORD_TRUE,
    KEYWORD_FALSE,
};

STRUCT(keyword) {
    char                name[MAX_NAME_SIZE];
    keyword_e           type;

    stmt_decl_part_e    decl[MAX_STMT_PARTS];
    uint32_t            decl_size;

    int                 (*init_node)(language_t* lang, ast_node_t* node);
    int                 (*validate_node)(language_t* lang, ast_node_t* node);
};

ENUM(type) {
    TYPE_TYPE_PRIMITIVE,
    TYPE_TYPE_STRUCT,
    TYPE_TYPE_ALIAS,
};

ENUM(primitive_type) {
    PRIMITIVE_TYPE_CUSTOM,
    PRIMITIVE_TYPE_NUMBER,
    PRIMITIVE_TYPE_STRING,
    PRIMITIVE_TYPE_BOOLEAN,
    PRIMITIVE_TYPE_VOID,
    PRIMITIVE_TYPE_OBJECT,
};

STRUCT(type) {
    char                name[MAX_NAME_SIZE];
    type_e              type;

    uint32_t            quantity;   // 0 for pure type, > 0 for type array
    primitive_type_e    primitive;  // For primitive types

    symbol_t*           fields; // For structs
    type_t*             parent;
    type_t*             next;
};

STRUCT(scope) {
    scope_t*    parent;
    scope_t*    childs;
    scope_t*    next;
    symbol_t*   symbols;
};

ENUM(symbol_type) {
    SYMBOL_TYPE_VARIABLE,
    SYMBOL_TYPE_FUNCTION,
};

STRUCT(symbol) {
    char            name[MAX_NAME_SIZE];
    symbol_type_e   type;
    type_t*         type_info; // Return type for functions and variable type for variables

    // For functions
    symbol_t*       params;

    symbol_t*       next;
};

#define FOREACH_AST_NODE(F) \
    F(ROOT) \
    F(DECL_VARIABLE) \
    F(DECL_FUNCTION) \
    F(ASSIGNMENT) \
    F(EXPRESSION) \
    F(IDENTIFIER) \
    F(CONSTANT) \
    F(FUNCTION_CALL) \
    F(ARRAY) \
    F(ARRAY_ACCESS) \
    F(KEYWORD) \
    F(OBJECT_INIT)

#define AST_NODE_ENUM(V)        AST_##V,
#define AST_NODE_ENUM_STR(V)   [AST_##V] = #V,

ENUM(ast_node_type) {
    AST_NODE_ENUM(UNKNOWN)
    FOREACH_AST_NODE(AST_NODE_ENUM)
    AST_NODE_ENUM(MAX)
};

extern const char* AST_NODE_STR[];

STRUCT(ast_node) {
    ast_node_type_e type;

    uint32_t line;
    uint32_t column;

    union {
        struct {
            union {
                struct {
                    ast_node_t* l;
                    ast_node_t* r;
                };

                struct {
                    ast_node_t* cond;
                    ast_node_t* if_true;
                    ast_node_t* if_false;
                };
            };

            operator_t*  op;
        };

        struct {
            char        name[MAX_NAME_SIZE];
            type_t*     type;

            // For arrays
            uint32_t    size;
        } decl;

        struct {
            type_t*     type;
            int32_t     num;
            char        str[MAX_NAME_SIZE];
        } cnst;

        keyword_t*  kw;
    };

    char        id[MAX_NAME_SIZE];

    ast_node_t* args;
    ast_node_t* body;
    ast_node_t* next;
    ast_node_t* next_stmt; // Used for if/else if/else statements
};

STRUCT(generator) {
    language_generator_t* lang;

    char*       buf;
    char*       cur;
    uint32_t    buf_size;

    int32_t     indent;
    uint32_t    indent_size;
};

STRUCT(language_generator) {
    void    (*gen_keyword)      (generator_t* gen, ast_node_t* node);
    void    (*gen_operator)     (generator_t* gen, ast_node_t* node);
    void    (*gen_expression)   (generator_t* gen, ast_node_t* node);
    void    (*gen_var_decl)     (generator_t* gen, ast_node_t* node);
    void    (*gen_func_decl)    (generator_t* gen, ast_node_t* node);
    void    (*gen_block)        (generator_t* gen, ast_node_t* node);
    void    (*gen_identifier)   (generator_t* gen, ast_node_t* node);
    void    (*gen_constant)     (generator_t* gen, ast_node_t* node);
    void    (*gen_call)         (generator_t* gen, ast_node_t* node);
};

STRUCT(language) {
    char*           name;

    stmt_decl_part_e*   func_decl;
    uint32_t            func_decl_size;

    scope_t*        root;
    scope_t*        current;

    type_t*         types;

    operator_t*     operators;
    keyword_t*      keywords;

    uint32_t        operator_size;
    uint32_t        keyword_size;
};

extern void         generator_put(generator_t* gen, const char* format, ...);
extern void         generator_print_indent(generator_t* gen);
extern void         generator_gen_expression(generator_t* gen, ast_node_t* node);
extern generator_t* generator_create(language_generator_t* lang, char* buf, uint32_t buf_size);
extern void         generator_generate(generator_t* gen, ast_node_t* node);

extern void         lang_scope_enter(language_t* lang);
extern void         lang_scope_leave(language_t* lang);

extern type_t*      lang_find_type(language_t* lang, char* name);
extern type_t*      lang_find_struct(language_t* lang, char* name);
extern type_t*      lang_find_primitive_type(language_t* lang, primitive_type_e primitive);
extern operator_t*  lang_find_operator(language_t* lang, char* name);
extern keyword_t*   lang_find_keyword(language_t* lang, char* name);
extern symbol_t*    lang_find_symbol(language_t* lang, char* name);
extern symbol_t*    lang_find_field_in_struct(type_t* type, char* name);

extern type_t*      lang_add_type(language_t* lang, char* name, char* parent, uint32_t quantity);
extern type_t*      lang_add_struct(language_t* lang, char* name);
extern type_t*      lang_add_primitive_type(language_t* lang, char* name, primitive_type_e primitive);
extern symbol_t*    lang_add_symbol(language_t* lang, char* name, symbol_type_e type, type_t* type_info);

extern symbol_t*    lang_symbol_add_argument(symbol_t* sym, char* name, type_t* type_info);
extern symbol_t*    lang_struct_add_field(type_t* type, char* name);

extern language_t*  lang_create();
extern void         lang_free(language_t* lang);