#include <parser.h>
#include <utils.h>

typedef struct {
    ast_node_t* nodes[100];
    int top;
} rpn_stack_t;

int rpn_is_empty(rpn_stack_t* stack) {
    return stack->top == -1;
}

void rpn_push(rpn_stack_t* stack, ast_node_t* val) {
    if (stack->top == 100) {
        PARSER_LOG("Error: Stack is full");
        exit(1);
    }

    stack->nodes[++stack->top] = val;
}

ast_node_t* rpn_pop(rpn_stack_t* stack) {
    if (rpn_is_empty(stack)) {
        PARSER_LOG("Error: Stack is empty");
        exit(1);
    }

    return stack->nodes[stack->top--];
}

ast_node_t* rpn_peek(rpn_stack_t* stack) {
    if (rpn_is_empty(stack))
        return NULL;

    return stack->nodes[stack->top];
}

bool rpn_compare(ast_node_t* node0, ast_node_t* node1) {
    if (node1->op == NULL)
        return false;

    return node0->op->precedence >= node1->op->precedence;
}

ast_node_t* rpn_parse_expression(parser_t* parser) {
    rpn_stack_t holding_stack = { .top = -1 };
    rpn_stack_t solve_stack = { .top = -1 };

    ast_node_t* tmp;

    uint32_t parenthesis_balance = 0;

    token_t* token = current_token(parser);
    bool running = true;

    ast_node_t* node = NULL;
    while (running) {
        if (!token)
            break;

        switch (token->type) {
            case TOKEN_SEMICOLON:
                consume(parser, TOKEN_SEMICOLON);
                // printf("End of statement (line %d, column %d)\n", token->line, token->column);
                running = false;
                break;

            case TOKEN_END_OF_FILE:
            case TOKEN_COMMA:
            case TOKEN_BRACKET_CLOSE:
            case TOKEN_SCOPE_OPEN:
            case TOKEN_SCOPE_CLOSE:
            case TOKEN_COLON:
                running = false;
                break;

            case TOKEN_PARENT_CLOSE:
                if (parenthesis_balance == 0)
                    running = false;

                break;

            default:
                break;
        }

        if (!running)
            break;

        node = alloc_node(AST_UNKNOWN);

        switch (token->type) {
            case TOKEN_LITERAL_NUMERIC:
                parse_number(parser, node);
                rpn_push(&solve_stack, node);
                break;

            case TOKEN_LITERAL_STRING:
                parse_string(parser, node);
                rpn_push(&solve_stack, node);
                break;

            case TOKEN_IDENTIFIER:
                parse_identifier(parser, node);

                switch (node->type) {
                    case AST_IDENTIFIER:
                    case AST_DECL_VARIABLE:
                    case AST_DECL_FUNCTION:
                    case AST_FUNCTION_CALL:
                    case AST_ARRAY_ACCESS:
                    case AST_KEYWORD:
                        rpn_push(&solve_stack, node);
                        break;

                    case AST_EXPRESSION:
                        tmp = rpn_peek(&holding_stack);
                        while (tmp && rpn_compare(node, tmp)) {
                            rpn_push(&solve_stack, rpn_pop(&holding_stack));
                            tmp = rpn_peek(&holding_stack);
                        }

                        rpn_push(&holding_stack, node);
                        break;

                    default:
                        PARSER_LOG("Error: Unknown node type:");
                        print_node(node, 0);
                        exit(1);
                }
                break;

            case TOKEN_OPERATOR:
            case TOKEN_QUESTION_MARK:
            case TOKEN_COLON:
                parse_operator(parser, node);

                if (node->type == AST_EXPRESSION) {
                    tmp = rpn_peek(&holding_stack);
                    while (tmp && rpn_compare(node, tmp)) {
                        rpn_push(&solve_stack, rpn_pop(&holding_stack));
                        tmp = rpn_peek(&holding_stack);
                    }

                    rpn_push(&holding_stack, node);

                } else {
                    PARSER_LOG("Error: Unknown node type:");
                    print_node(node, 0);
                    exit(1);
                }
                break;

            case TOKEN_BRACKET_OPEN:
                parse_array(parser, node);
                rpn_push(&holding_stack, node);
                break;

            case TOKEN_PARENT_OPEN:
                consume(parser, TOKEN_PARENT_OPEN);
                parenthesis_balance++;
                node->type = AST_EXPRESSION;
                node->op = NULL;
                printf("Put NULL operator\n");
                rpn_push(&holding_stack, node);
                break;

            case TOKEN_PARENT_CLOSE:
                consume(parser, TOKEN_PARENT_CLOSE);
                parenthesis_balance--;
                // Drain the stack while not reach the expression with unknown operator
                ast_node_t* tmp1 = rpn_peek(&holding_stack);
                while (tmp1 && tmp1->op != NULL) {
                    rpn_push(&solve_stack, rpn_pop(&holding_stack));
                    tmp1 = rpn_peek(&holding_stack);
                }

                printf("Pull NULL operator\n");
                tmp1 = rpn_pop(&holding_stack);
                free(tmp1);
                break;

            case TOKEN_KEYWORD:
                parse_keyword(parser, node);
                rpn_push(&solve_stack, node);
                break;

            case TOKEN_END_OF_FILE:
                // Happens only if entire file are empty
                PARSER_LOG("End of file (file are empty)");
                return NULL;

            default:
                PARSER_LOG("Error: Unexpected token: ");
                print_token(token);
                exit(1);
        }

        // Not used, so we need to free it
        if (node->type == AST_UNKNOWN)
            free(node);

        token = current_token(parser);
    }

    if (parenthesis_balance != 0)
        PARSER_LOG("WARNING: Parenthesis balance is not zero");

    while (!rpn_is_empty(&solve_stack))
        rpn_push(&holding_stack, rpn_pop(&solve_stack));

    // SOLVE IT DAMN IT
    while (!rpn_is_empty(&holding_stack)) {
        ast_node_t* node = rpn_pop(&holding_stack);

        switch (node->type) {
            case AST_IDENTIFIER:
            case AST_ARRAY:
            case AST_CONSTANT:
            case AST_DECL_VARIABLE:
            case AST_DECL_FUNCTION:
            case AST_FUNCTION_CALL:
            case AST_ARRAY_ACCESS:
            case AST_KEYWORD:
            case AST_OBJECT_INIT:
                rpn_push(&solve_stack, node);
                break;

            case AST_EXPRESSION:
                switch (node->op->type) {
                    case OPERATOR_INC:
                    case OPERATOR_DEC:
                    case OPERATOR_NOT:
                    case OPERATOR_BITWISE_NOT:
                    case OPERATOR_CUSTOM_UNARY:
                        node->l = rpn_pop(&solve_stack);
                        // printf("Solved unary op: \n");
                        // print_node(node, 0);
                        rpn_push(&solve_stack, node);
                        break;

                    // We can evaluate some expressions here e.g. (1 + 2)
                    default:
                        node->r = rpn_pop(&solve_stack);
                        node->l = rpn_pop(&solve_stack);
                        // printf("Solved binary op: \n");
                        // print_node(node, 0);
                        rpn_push(&solve_stack, node);
                        break;

                    // case OPERATOR_TYPE_TERNARY:
                        // tmp = rpn_pop(&solve_stack);
                        // if (tmp->type == AST_EXPRESSION && tmp->op.op == OPERATOR_TERNARY_ELSE) {
                        //     node->op.if_true   = tmp->l;
                        //     node->op.if_false  = tmp->r;
                        //     node->op.cond      = rpn_pop(&solve_stack);
                        //     free(tmp); // Free the ternary else node, it not needed anymore
                        //     rpn_push(&solve_stack, node);
                        // } else {
                        //     PARSER_LOG("Error: Ternary if operator must be followed by ternary else");
                        //     exit(1);
                        // }
                        // break;

                }
                break;

            default:
                PARSER_LOG("Error: Unknown node type:");
                print_node(node, 0);
                exit(1);
        }
    }

    while (!rpn_is_empty(&solve_stack))
        rpn_push(&holding_stack, rpn_pop(&solve_stack));

    if (rpn_is_empty(&holding_stack))
        return NULL;

    ast_node_t* root = rpn_pop(&holding_stack);
    ast_node_t* current = root;

    while (!rpn_is_empty(&holding_stack)) {
        current->next = rpn_pop(&holding_stack);
        current = current->next;
    }

    return root;
}