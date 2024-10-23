#include <lexer.h>
#include <utils.h>

const char* TOKEN_STR[] = {
    TOKEN_ENUM_STR(UNKNOWN)
    FOREACH_TOKEN(TOKEN_ENUM_STR)
    TOKEN_ENUM_STR(MAX)
};

char WHITESPACE_DIGITS[]    = " \t\n\r";
char NUMERIC_DIGITS[]       = "0123456789";
char REAL_DIGITS[]          = ".0123456789";
char OPERATORS_DIGITS[]     = "!$%^&*+-=#@|`/\\<>~.";
char IDENTIFIER_DIGITS[]    = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
char HEX_DIGITS[]           = "abcdefABCDEF0123456789";
char BINARY_DIGITS[]        = "01";

typedef enum {
    FSM_STATE_NEW_TOKEN,
    FSM_STATE_COLON,
    FSM_STATE_SEMICOLON,
    FSM_STATE_STRING_LITERAL,
    FSM_STATE_NUMERIC_LITERAL,
    FSM_STATE_IDENTIFIER,
    FSM_STATE_PARENT_OPEN,
    FSM_STATE_PARENT_CLOSE,
    FSM_STATE_SCOPE_OPEN,
    FSM_STATE_SCOPE_CLOSE,
    FSM_STATE_BRACKET_OPEN,
    FSM_STATE_BRACKET_CLOSE,
    FSM_STATE_COMMA,
    FSM_STATE_OPERATOR,
    FSM_STATE_COMMENT_SINGLELINE,
    FSM_STATE_COMMENT_MULTILINE,
    FSM_STATE_COMMENT_MULTILINE_END,
    FSM_STATE_QUESTION_MARK,
    FSM_STATE_COMPLETE_TOKEN,
} fsm_state_e;

token_list_t* lexer_analyze(language_t* lang, char* source) {
    token_list_t* list = malloc(sizeof(token_list_t));

    operator_t* op;
    operator_t* op_next;
    keyword_t* kw;

    list->count = 0;
    list->cur = list->tokens;

    fsm_state_e state       = FSM_STATE_NEW_TOKEN;
    fsm_state_e next_state  = FSM_STATE_NEW_TOKEN;
    token_t token = {
        .type = TOKEN_UNKNOWN,
        .line = 1,
        .column = 1,
    };

    uint32_t line = 1;
    uint32_t column = 1;

    char* cur = source;
    char* end = source + strlen(source) + 1;

    char* tok = token.value;

    while (cur < end) {
        switch (state) {
            case FSM_STATE_NEW_TOKEN:
                token.type = TOKEN_UNKNOWN;

                tok = token.value;
                token.line = line;
                token.column = column;

                if (strchr(WHITESPACE_DIGITS, *cur)) {
                    if (*cur == '\n') {
                        line++;
                        column = 1;
                    } else {
                        column++;
                    }
                    cur++;
                    next_state = FSM_STATE_NEW_TOKEN;
                } else if (strchr(NUMERIC_DIGITS, *cur)) {
                    *tok++ = *cur++;
                    column++;
                    next_state = FSM_STATE_NUMERIC_LITERAL;
                } else if (strchr(OPERATORS_DIGITS, *cur)) {
                    next_state = FSM_STATE_OPERATOR;

                    if (*cur == '/') {
                        cur++;
                        column++;

                        if (*cur == '*') {
                            cur++;
                            column++;
                            // printf("/*");
                            next_state = FSM_STATE_COMMENT_MULTILINE;
                        } else if (*cur == '/') {
                            // printf("//");
                            next_state = FSM_STATE_COMMENT_SINGLELINE;
                        } else {
                            cur--;
                            column--;
                        }
                    }
                } else if (*cur == '(') {
                    next_state = FSM_STATE_PARENT_OPEN;
                } else if (*cur == ')') {
                    next_state = FSM_STATE_PARENT_CLOSE;
                } else if (*cur == '{') {
                    next_state = FSM_STATE_SCOPE_OPEN;
                } else if (*cur == '}') {
                    next_state = FSM_STATE_SCOPE_CLOSE;
                } else if (*cur == '[') {
                    next_state = FSM_STATE_BRACKET_OPEN;
                } else if (*cur == ']') {
                    next_state = FSM_STATE_BRACKET_CLOSE;
                } else if (*cur == ',') {
                    next_state = FSM_STATE_COMMA;
                } else if (*cur == ':') {
                    next_state = FSM_STATE_COLON;
                } else if (*cur == ';') {
                    next_state = FSM_STATE_SEMICOLON;
                } else if (*cur == '?') {
                    next_state = FSM_STATE_QUESTION_MARK;
                } else if (*cur == '"') {
                    column++;
                    cur++;
                    next_state = FSM_STATE_STRING_LITERAL;
                } else {
                    *tok++ = *cur++;
                    column++;
                    next_state = FSM_STATE_IDENTIFIER;
                }

                break;

            case FSM_STATE_STRING_LITERAL:
                if (*cur == '"') {
                    token.type = TOKEN_LITERAL_STRING;
                    next_state = FSM_STATE_COMPLETE_TOKEN;
                } else {
                    *tok++ = *cur;
                    next_state = FSM_STATE_STRING_LITERAL;
                }

                column++;
                cur++;
                break;

            case FSM_STATE_NUMERIC_LITERAL:
                if (strchr(NUMERIC_DIGITS, *cur)) {
                    *tok++ = *cur++;
                    column++;
                    next_state = FSM_STATE_NUMERIC_LITERAL;
                } else {
                    if (strchr(IDENTIFIER_DIGITS, *cur)) {
                        printf("Error: Invalid numeric literal: %s (line %d, column %d)\n", token.value, line, column);
                        exit(1);
                    }

                    token.type = TOKEN_LITERAL_NUMERIC;
                    next_state = FSM_STATE_COMPLETE_TOKEN;
                }
                break;

            case FSM_STATE_OPERATOR:
                op = lang_find_operator(lang, token.value);

                if (strchr(OPERATORS_DIGITS, *cur)) {
                    char tmp[1024] = { 0 };
                    snprintf(tmp, 1024, "%s%c", token.value, *cur);

                    op_next = lang_find_operator(lang, token.value);

                    if (op_next) {
                        *tok++ = *cur++;
                        *tok = '\0';
                        column++;
                        next_state = FSM_STATE_OPERATOR;
                    } else if (op) {
                        token.type = TOKEN_OPERATOR;
                        next_state = FSM_STATE_COMPLETE_TOKEN;
                    } else {
                        *tok++ = *cur++;
                        *tok = '\0';
                        column++;
                        next_state = FSM_STATE_OPERATOR;
                    }
                } else {
                    if (op) {
                        token.type = TOKEN_OPERATOR;
                        next_state = FSM_STATE_COMPLETE_TOKEN;
                    } else {
                        printf("Error: Invalid operator: %s (line %d, column %d)\n", token.value, line, column);
                        exit(1);
                    }
                }
                break;

            case FSM_STATE_PARENT_OPEN:
                *tok++ = *cur++;
                token.type = TOKEN_PARENT_OPEN;
                column++;
                next_state = FSM_STATE_COMPLETE_TOKEN;
                break;

            case FSM_STATE_PARENT_CLOSE:
                *tok++ = *cur++;
                token.type = TOKEN_PARENT_CLOSE;
                column++;
                next_state = FSM_STATE_COMPLETE_TOKEN;
                break;

            case FSM_STATE_SCOPE_OPEN:
                *tok++ = *cur++;
                token.type = TOKEN_SCOPE_OPEN;
                column++;
                next_state = FSM_STATE_COMPLETE_TOKEN;
                break;

            case FSM_STATE_SCOPE_CLOSE:
                *tok++ = *cur++;
                token.type = TOKEN_SCOPE_CLOSE;
                column++;
                next_state = FSM_STATE_COMPLETE_TOKEN;
                break;

            case FSM_STATE_BRACKET_OPEN:
                *tok++ = *cur++;
                token.type = TOKEN_BRACKET_OPEN;
                column++;
                next_state = FSM_STATE_COMPLETE_TOKEN;
                break;

            case FSM_STATE_BRACKET_CLOSE:
                *tok++ = *cur++;
                token.type = TOKEN_BRACKET_CLOSE;
                column++;
                next_state = FSM_STATE_COMPLETE_TOKEN;
                break;

            case FSM_STATE_COMMA:
                *tok++ = *cur++;
                token.type = TOKEN_COMMA;
                column++;
                next_state = FSM_STATE_COMPLETE_TOKEN;
                break;

            case FSM_STATE_COLON:
                *tok++ = *cur++;
                token.type = TOKEN_COLON;
                column++;
                next_state = FSM_STATE_COMPLETE_TOKEN;
                break;

            case FSM_STATE_SEMICOLON:
                *tok++ = *cur++;
                token.type = TOKEN_SEMICOLON;
                column++;
                next_state = FSM_STATE_COMPLETE_TOKEN;
                break;

            case FSM_STATE_QUESTION_MARK:
                *tok++ = *cur++;
                token.type = TOKEN_QUESTION_MARK;
                column++;
                next_state = FSM_STATE_COMPLETE_TOKEN;
                break;

            case FSM_STATE_COMMENT_SINGLELINE:
                if (*cur == '\n') {
                    next_state = FSM_STATE_NEW_TOKEN;
                } else {
                    cur++;
                    column++;
                    // printf("%c", *cur);
                    next_state = FSM_STATE_COMMENT_SINGLELINE;
                }
                break;

            case FSM_STATE_COMMENT_MULTILINE:
                if (*cur == '*') {
                    next_state = FSM_STATE_COMMENT_MULTILINE_END;
                } else {
                    // printf("%c", *cur);
                    next_state = FSM_STATE_COMMENT_MULTILINE;
                }

                if (*cur == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }

                cur++;
                break;

            case FSM_STATE_COMMENT_MULTILINE_END:
                if (*cur == '/') {
                    // printf("*/\n");
                    next_state = FSM_STATE_NEW_TOKEN;
                } else {
                    // printf("*");
                    next_state = FSM_STATE_COMMENT_MULTILINE;
                }
                cur++;
                column++;
                break;

            case FSM_STATE_IDENTIFIER:
                op = lang_find_operator(lang, token.value);
                kw = lang_find_keyword(lang, token.value);

                if (strchr(IDENTIFIER_DIGITS, *cur)) {
                    *tok++ = *cur++;
                    *tok = '\0';
                    column++;
                    next_state = FSM_STATE_IDENTIFIER;
                } else {
                    if (kw) {
                        token.type = TOKEN_KEYWORD;
                    } else if (op) {
                        token.type = TOKEN_OPERATOR;
                    } else {
                        token.type = TOKEN_IDENTIFIER;
                    }
                    next_state = FSM_STATE_COMPLETE_TOKEN;
                }
                break;

            case FSM_STATE_COMPLETE_TOKEN:
                *tok = '\0';
                memcpy(list->cur++, &token, sizeof(token_t));
                memset(&token.value, 0, MAX_NAME_SIZE);
                list->count++;
                next_state = FSM_STATE_NEW_TOKEN;
                break;
        }

        state = next_state;
    }

    token.type = TOKEN_END_OF_FILE;
    *tok = '\0';
    memcpy(list->cur++, &token, sizeof(token_t));
    list->count++;

    return list;
}

void lexer_free_token_list(token_list_t* list) {
    free(list);
}