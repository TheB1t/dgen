#pragma once

#include <common.h>
#include <lang.h>

#define TOKEN_LIST_SIZE 4096

DECL_STRUCT(token);
DECL_STRUCT(token_list);
DECL_ENUM(token_type);

#define FOREACH_TOKEN(F) \
    F(END_OF_FILE) \
    F(COLON) \
    F(SEMICOLON) \
    F(LITERAL_NUMERIC) \
    F(LITERAL_STRING) \
    F(PARENT_OPEN) \
    F(PARENT_CLOSE) \
    F(SCOPE_OPEN) \
    F(SCOPE_CLOSE) \
    F(BRACKET_OPEN) \
    F(BRACKET_CLOSE) \
    F(COMMA) \
    F(OPERATOR) \
    F(IDENTIFIER) \
    F(KEYWORD) \
    F(QUESTION_MARK)

#define TOKEN_ENUM(V)           TOKEN_##V,
#define TOKEN_ENUM_STR(V)      [TOKEN_##V] = #V,

ENUM(token_type) {
    TOKEN_ENUM(UNKNOWN)
    FOREACH_TOKEN(TOKEN_ENUM)
    TOKEN_ENUM(MAX)
};

extern const char* TOKEN_STR[];

STRUCT(token) {
    ENUM(token_type)    type;

    char            value[MAX_NAME_SIZE];

    uint32_t        line;
    uint32_t        column;
};

STRUCT(token_list) {
    token_t   tokens[TOKEN_LIST_SIZE];
    token_t*  cur;
    uint32_t        count;
};

extern token_list_t*    lexer_analyze(language_t* lang, char* source);