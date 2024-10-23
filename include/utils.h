#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <lexer.h>
#include <parser.h>

extern ast_node_t*  alloc_node(ast_node_type_e type);

extern token_t*     current_token(parser_t* parser);
extern void         next_token(parser_t* parser);
extern void         prev_token(parser_t* parser);

extern bool         match(parser_t* parser, token_type_e type);
extern token_t*     consume(parser_t* parser, token_type_e type);

extern uint32_t     ast_list_size(ast_node_t* node);
extern void         ast_list_insert(ast_node_t* node, ast_node_t* new);
extern void         print_token(token_t* token);
extern void         print_node(ast_node_t* node, uint32_t ident);
extern void         string_to_upper(char *str);
extern char*        read_file(char* path);
extern void         write_file(char* path, char* buffer);