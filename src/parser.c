#include <parser.h>
#include <utils.h>

parser_t* create_parser(language_t* lang, token_list_t* tokens) {
    parser_t* parser = (parser_t*)malloc(sizeof(parser_t));
    memset(parser, 0, sizeof(parser_t));

    parser->lang = lang;
    parser->tokens = tokens;
    parser->current = 0;

    return parser;
}

ast_node_t* parse_block(parser_t* parser) {
    ast_node_t* root = rpn_parse_expression(parser);
    ast_node_t* cur = root;
    while (1) {
        cur = rpn_parse_expression(parser);
        if (cur == NULL)
            break;

        ast_list_insert(root, cur);
    }

    return root;
}

void parse_stmt_declaration(parser_t* parser, ast_node_t* node, stmt_decl_part_e* parts, uint32_t count) {
    token_t* tmp = NULL;

    for (uint32_t i = 0; i < count; i++) {
        switch (parts[i]) {
            case STMT_PART_PARENT_OPEN:
                consume(parser, TOKEN_PARENT_OPEN);
                break;

            case STMT_PART_PARENT_CLOSE:
                consume(parser, TOKEN_PARENT_CLOSE);
                break;

            case STMT_PART_SCOPE_OPEN:
                consume(parser, TOKEN_SCOPE_OPEN);
                break;

            case STMT_PART_SCOPE_CLOSE:
                consume(parser, TOKEN_SCOPE_CLOSE);
                break;

            case STMT_PART_COLON:
                consume(parser, TOKEN_COLON);
                break;

            case STMT_PART_COMMA:
                consume(parser, TOKEN_COMMA);
                break;

            case STMT_PART_SEMICOLON:
                consume(parser, TOKEN_SEMICOLON);
                break;

            case STMT_PART_EXPRESSION:
                if (node->args == NULL)
                    node->args = rpn_parse_expression(parser);
                else
                    ast_list_insert(node->args, rpn_parse_expression(parser));
                break;

            case STMT_PART_BLOCK:
                node->body = parse_block(parser);
                break;

            case STMT_PART_EXPRESSIONS:
                while (1) {
                    if (node->args == NULL)
                        node->args = rpn_parse_expression(parser);
                    else
                        ast_list_insert(node->args, rpn_parse_expression(parser));

                    if (match(parser, TOKEN_COMMA))
                        consume(parser, TOKEN_COMMA);
                    else
                        break;
                }
                break;

            case STMT_PART_IDENTIFIER:
                tmp = consume(parser, TOKEN_IDENTIFIER);
                strncpy(node->id, tmp->value, MAX_NAME_SIZE);
                break;

            default:
                PARSER_LOG("NOT IMPLEMENTED: Statement declaration parameter: %d", node->kw->decl[i]);
                exit(1);
                break;
        }
    }
}

void parse_number(parser_t* parser, ast_node_t* node) {
    token_t* token = consume(parser, TOKEN_LITERAL_NUMERIC);

    node->type          = AST_CONSTANT;

    node->cnst.type     = lang_find_primitive_type(parser->lang, PRIMITIVE_TYPE_NUMBER);
    node->cnst.num      = atol(token->value);

    node->line          = token->line;
    node->column        = token->column;
}

void parse_string(parser_t* parser, ast_node_t* node) {
    token_t* token = consume(parser, TOKEN_LITERAL_STRING);

    node->type          = AST_CONSTANT;

    node->cnst.type     = lang_find_primitive_type(parser->lang, PRIMITIVE_TYPE_STRING);
    strncpy(node->cnst.str, token->value, MAX_NAME_SIZE);

    node->line          = token->line;
    node->column        = token->column;
}

void parse_arguments(parser_t* parser, ast_node_t* node, token_type_e start, token_type_e end, token_type_e separator) {
    consume(parser, start);

    while (!match(parser, end)) {
        if (node->args == NULL)
            node->args = rpn_parse_expression(parser);
        else
            ast_list_insert(node->args, rpn_parse_expression(parser));

        if (match(parser, separator))
            consume(parser, separator);
        // else if (!match(parser, end)) {
        //     PARSER_LOG("Error: Unexpected token: ");
        //     print_token(current_token(parser));
        //     exit(1);
        // }
    }

    consume(parser, end);
}

void parse_variable_declaration(parser_t* parser, ast_node_t* node, char* name, type_t* type) {
    token_t* token = current_token(parser);

    node->type = AST_DECL_VARIABLE;
    strncpy(node->decl.name, name, MAX_NAME_SIZE);

    node->decl.type = type;

    if (token->type == TOKEN_BRACKET_OPEN) {
        consume(parser, TOKEN_BRACKET_OPEN);
        token = consume(parser, TOKEN_LITERAL_NUMERIC);
        consume(parser, TOKEN_BRACKET_CLOSE);

        node->decl.size = atol(token->value);
    }
}

void parse_function_declaration(parser_t* parser, ast_node_t* node, char* name, type_t* type) {
    node->type = AST_DECL_FUNCTION;
    node->decl.type = type;
    strncpy(node->decl.name, name, MAX_NAME_SIZE);

    parse_stmt_declaration(parser, node, parser->lang->func_decl, parser->lang->func_decl_size);
}

void parse_declaration(parser_t* parser, ast_node_t* node, token_t* type_token) {
    token_t* identifier_token = consume(parser, TOKEN_IDENTIFIER);
    token_t* token = current_token(parser);

    char* name = identifier_token->value;
    type_t* type = lang_find_type(parser->lang, type_token->value);

    if (type == NULL) {
        PARSER_LOG("Error: Type not found: %s", type_token->value);
        exit(1);
    }

    switch (token->type) {
        case TOKEN_PARENT_OPEN:
            parse_function_declaration(parser, node, name, type);
            break;

        case TOKEN_SCOPE_OPEN:
            printf("Parse object initialization %s\n", identifier_token->value);
            node->type = AST_OBJECT_INIT;
            strncpy(node->decl.name, identifier_token->value, MAX_NAME_SIZE);
            parse_arguments(parser, node, TOKEN_SCOPE_OPEN, TOKEN_SCOPE_CLOSE, TOKEN_COMMA);
            break;

        default:
            parse_variable_declaration(parser, node, name, type);
    }

    node->line = identifier_token->line;
    node->column = identifier_token->column;
}

void parse_identifier(parser_t* parser, ast_node_t* node) {
    token_t* identifier_token = consume(parser, TOKEN_IDENTIFIER);
    token_t* token = current_token(parser);

    switch (token->type) {
        case TOKEN_IDENTIFIER:
            parse_declaration(parser, node, identifier_token);
            break;

        case TOKEN_PARENT_OPEN:
            node->type = AST_FUNCTION_CALL;
            strncpy(node->id, identifier_token->value, MAX_NAME_SIZE);
            parse_arguments(parser, node, TOKEN_PARENT_OPEN, TOKEN_PARENT_CLOSE, TOKEN_COMMA);
            break;

        case TOKEN_BRACKET_OPEN:
            node->type = AST_ARRAY_ACCESS;
            strncpy(node->id, identifier_token->value, MAX_NAME_SIZE);
            parse_arguments(parser, node, TOKEN_BRACKET_OPEN, TOKEN_BRACKET_CLOSE, TOKEN_COMMA);
            break;

        default:
            // PARSER_LOG("WARNING: Generate default symbol node: %s", identifier_token->value.str);
            node->type = AST_IDENTIFIER;
            strncpy(node->id, identifier_token->value, MAX_NAME_SIZE);
            break;
    }

    node->line = identifier_token->line;
    node->column = identifier_token->column;
}

void parse_keyword(parser_t* parser, ast_node_t* node) {
    token_t* keyword_token = consume(parser, TOKEN_KEYWORD);

    node->type = AST_KEYWORD;
    node->kw = lang_find_keyword(parser->lang, keyword_token->value);

    parse_stmt_declaration(parser, node, node->kw->decl, node->kw->decl_size);

    if (node->kw->init_node)
        node->kw->init_node(parser->lang, node);

    node->line = keyword_token->line;
    node->column = keyword_token->column;
}

void parse_array(parser_t* parser, ast_node_t* node) {
    token_t* token = current_token(parser);

    node->line = token->line;
    node->column = token->column;

    node->type = AST_ARRAY;
    parse_arguments(parser, node, TOKEN_BRACKET_OPEN, TOKEN_BRACKET_CLOSE, TOKEN_COMMA);
}

void parse_operator(parser_t* parser, ast_node_t* node) {
    token_t* operator_token = consume(parser, TOKEN_OPERATOR);
    node->type = AST_EXPRESSION;

    // switch (operator_token->type) {
    //     case TOKEN_QUESTION_MARK:
    //         node->op.op = OPERATOR_TERNARY_IF;
    //         break;

    //     case TOKEN_COLON:
    //         node->op.op = OPERATOR_TERNARY_ELSE;
    //         break;

    //     default:
            node->op = lang_find_operator(parser->lang, operator_token->value);
    // }

    node->line = operator_token->line;
    node->column = operator_token->column;
}

ast_node_t* parser_analyze(parser_t* parser) {
    ast_node_t* root_ast = alloc_node(AST_ROOT);
    ast_node_t* root = NULL;

    while (1) {
        if (root == NULL)
            root = rpn_parse_expression(parser);
        else
            ast_list_insert(root, rpn_parse_expression(parser));

        if (match(parser, TOKEN_END_OF_FILE)) {
            consume(parser, TOKEN_END_OF_FILE);
            break;
        }
    }

    root_ast->body = root;

    return root_ast;
}

void parser_free_ast(ast_node_t** node) {
    if (node == NULL)
        return;

    ast_node_t* n = *node;

    if (n == NULL)
        return;

    if (n->type == AST_EXPRESSION) {
        // if (n->op.op == OPERATOR_TERNARY_IF) {
        //     parser_free_ast(&n->op.cond);
        //     parser_free_ast(&n->op.if_true);
        //     parser_free_ast(&n->op.if_false);
        // } else {
            parser_free_ast(&n->l);
            parser_free_ast(&n->r);
        // }
    }

    parser_free_ast(&n->next);
    parser_free_ast(&n->args);
    parser_free_ast(&n->body);

    free(n);
    *node = NULL;
}