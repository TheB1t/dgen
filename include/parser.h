#pragma once

#include <common.h>
#include <lexer.h>

#if defined(LOGS_WITH_FILE_AND_LINE)
#define PARSER_LOG(fmt, ...) printf("[PARSER] %s:%-4d | " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define PARSER_LOG(fmt, ...) printf("[PARSER] " fmt "\n", ##__VA_ARGS__)
#endif

DECL_STRUCT(parser);

STRUCT(parser) {
    token_list_t*   tokens;
    language_t*     lang;
    uint32_t        current;
};

extern ast_node_t*  rpn_parse_expression(parser_t* parser);

extern parser_t*    create_parser(language_t* lang, token_list_t* tokens);

extern void         parse_number(parser_t* parser, ast_node_t* node);
extern void         parse_string(parser_t* parser, ast_node_t* node);
extern void         parse_identifier(parser_t* parser, ast_node_t* node);
extern void         parse_keyword(parser_t* parser, ast_node_t* node);
extern void         parse_array(parser_t* parser, ast_node_t* node);
extern void         parse_operator(parser_t* parser, ast_node_t* node);

extern ast_node_t*  parser_analyze(parser_t* parser);
extern void         parser_free_ast(ast_node_t** node);