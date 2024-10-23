#include <utils.h>

ast_node_t* alloc_node(ast_node_type_e type) {
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    memset(node, 0, sizeof(ast_node_t));
    node->type = type;

    return node;
}

token_t* current_token(parser_t* parser) {
    if (parser->current < parser->tokens->count)
        return &parser->tokens->tokens[parser->current];

    return NULL;
}

void next_token(parser_t* parser) {
    if (parser->current < parser->tokens->count)
        parser->current++;
    else
        printf("Error: No more tokens\n");
}

void prev_token(parser_t* parser) {
    if (parser->current > 0)
        parser->current--;
}

bool match(parser_t* parser, token_type_e type) {
    token_t* token = current_token(parser);

    return token && token->type == type;
}

token_t* consume(parser_t* parser, token_type_e type) {
    token_t* token = current_token(parser);

    if (match(parser, type)) {
        // printf("Consuming token: ");
        // print_token(current_token(parser));
        next_token(parser);
        return token;
    }

    PARSER_LOG("Error: Unexpected token: ");
    print_token(current_token(parser));
    printf("Expected token: %s\n", TOKEN_STR[type]);
    exit(1);
}

uint32_t ast_list_size(ast_node_t* node) {
    uint32_t size = 0;

    ast_node_t* current = node;
    while (current) {
        size++;
        current = current->next;
    }

    return size;
}

void ast_list_insert(ast_node_t* node, ast_node_t* new) {
    if (!new)
        return;

    ast_node_t* current = node;

    while (current->next)
        current = current->next;

    current->next = new;
}

void print_token(token_t* token) {
    printf("Token[%s](line %d, column %d): %s\n", TOKEN_STR[token->type], token->line, token->column, token->value);
}

void print_ident(uint32_t ident) {
    for (uint32_t i = 0; i < ident; i++)
        printf(" |   ");
}

void print_ll(ast_node_t* node, uint32_t ident) {
    if (!node) {
        print_ident(ident);
        printf("Empty\n");
        return;
    }

    ast_node_t* arg = node;
    while (arg) {
        print_node(arg, ident);
        arg = arg->next;
    }
}

void print_node(ast_node_t* node, uint32_t ident) {
    if (!node) {
        print_ident(ident);
        printf("Empty\n");
        return;
    }

    print_ident(ident);
    printf("(At line %d, column %d)\n", node->line, node->column);
    print_ident(ident);

    switch (node->type) {
        case AST_ROOT:
            printf("Root\n");
            print_ident(ident);
            printf(" |-Body:\n");
            print_ll(node->body, ident + 1);
            break;

        case AST_DECL_VARIABLE:
            printf("Variable declaration: type %s, name %s (size for arr %d)\n", node->decl.type->name, node->decl.name, node->decl.size);
            break;

        case AST_DECL_FUNCTION:
            printf("Function: %s\n", node->decl.name);
            print_ident(ident);
            printf(" |-Arguments:\n");
            print_ll(node->args, ident + 1);
            print_ident(ident);
            printf(" |-Body:\n");
            print_ll(node->body, ident + 1);
            break;

        case AST_FUNCTION_CALL:
            printf("Function call: %s\n", node->id);
            print_ident(ident);
            printf(" |-Arguments:\n");
            print_ll(node->args, ident + 1);
            break;

        case AST_CONSTANT:
            printf("Constant: num %d, str %s\n", node->cnst.num, node->cnst.str);
            break;

        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->id);
            break;

        case AST_EXPRESSION:
            printf("Expression: %s\n", node->op->name);

            switch (node->op->type) {
                case OPERATOR_INC:
                case OPERATOR_DEC:
                case OPERATOR_NOT:
                case OPERATOR_BITWISE_NOT:
                case OPERATOR_CUSTOM_UNARY:
                    print_ident(ident);
                    printf(" |-Left:\n");
                    print_node(node->l, ident + 1);
                    break;

                default:
                    print_ident(ident);
                    printf(" |-Left:\n");
                    print_node(node->l, ident + 1);
                    print_ident(ident);
                    printf(" |-Right:\n");
                    print_node(node->r, ident + 1);
                    break;
            }
            break;

        case AST_ARRAY:
            printf("Array\n");
            print_ident(ident);
            printf(" |-Elements:\n");
            print_ll(node->args, ident + 1);
            break;

        case AST_KEYWORD:
            printf("Keyword: %s\n", node->kw->name);
            print_ident(ident);
            printf(" |-Arguments:\n");
            print_ll(node->args, ident + 1);
            print_ident(ident);
            printf(" |-Body:\n");
            print_ll(node->body, ident + 1);
            break;

        case AST_ARRAY_ACCESS:
            printf("Array access: %s\n", node->id);
            print_ident(ident);
            printf(" |-Index:\n");
            print_node(node->args, ident + 1);
            break;

        case AST_OBJECT_INIT:
            printf("Object init\n");
            print_ident(ident);
            printf(" |-Elements:\n");
            print_ll(node->args, ident + 1);
            break;

        default:
            printf("Unknown node type: %d\n", node->type);
            break;
    }
}

// int operator_precedence(operator_type_e op) {
//     // Most unary operators have precedence 2
//     // All assignment operators have precedence 14

//     switch (op) {
//         case OPERATOR_IN:
//         case OPERATOR_MEMBER_ACCESS:
//             return 1;

//         case OPERATOR_INCREMENT:
//         case OPERATOR_DECREMENT:
//         case OPERATOR_BITWISE_NOT:
//         case OPERATOR_LOGICAL_NOT:
//             return 2;

//         case OPERATOR_MUL:
//         case OPERATOR_DIV:
//         case OPERATOR_MOD:
//             return 3;

//         case OPERATOR_ADD:
//         case OPERATOR_SUB:
//             return 4;

//         case OPERATOR_BITWISE_LEFT_SHIFT:
//         case OPERATOR_BITWISE_RIGHT_SHIFT:
//             return 5;

//         case OPERATOR_LESS_THAN:
//         case OPERATOR_GREATER_THAN:
//         case OPERATOR_LESS_THAN_OR_EQUAL:
//         case OPERATOR_GREATER_THAN_OR_EQUAL:
//             return 6;

//         case OPERATOR_EQUAL:
//         case OPERATOR_NOT_EQUAL:
//             return 7;

//         case OPERATOR_BITWISE_AND:
//             return 8;

//         case OPERATOR_BITWISE_XOR:
//             return 9;

//         case OPERATOR_BITWISE_OR:
//             return 10;

//         case OPERATOR_LOGICAL_AND:
//             return 11;

//         case OPERATOR_LOGICAL_OR:
//             return 12;

//         case OPERATOR_TERNARY_IF:
//         case OPERATOR_TERNARY_ELSE:
//             return 13;

//         case OPERATOR_ASSIGN:
//         case OPERATOR_ASSIGN_ADD:
//         case OPERATOR_ASSIGN_SUB:
//         case OPERATOR_ASSIGN_MUL:
//         case OPERATOR_ASSIGN_DIV:
//         case OPERATOR_ASSIGN_MOD:
//         case OPERATOR_ASSIGN_BITWISE_AND:
//         case OPERATOR_ASSIGN_BITWISE_XOR:
//         case OPERATOR_ASSIGN_BITWISE_OR:
//         case OPERATOR_ASSIGN_BITWISE_LEFT_SHIFT:
//         case OPERATOR_ASSIGN_BITWISE_RIGHT_SHIFT:
//             return 14;

//         case OPERATOR_UNKNOWN:
//         case OPERATOR_MAX:
//             PARSER_LOG("Something went wrong, operator: %s\n", OPERATOR_STR[op]);
//             exit(1);
//     }

//     PARSER_LOG("Unknown operator: %s\n", OPERATOR_STR[op]);
//     exit(1);
// }

void string_to_upper(char *str) {
    while (*str) {
        *str = toupper((unsigned char)*str);
        str++;
    }
}

char* read_file(char* path) {
    FILE* file = fopen(path, "r");

    if (!file) {
        printf("Error: Could not open file %s\n", path);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = 0;
    fclose(file);

    return buffer;
}

void write_file(char* path, char* buffer) {
    FILE* file = fopen(path, "w");

    fwrite(buffer, 1, strlen(buffer), file);

    fclose(file);
}