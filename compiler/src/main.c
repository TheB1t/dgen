#include <stdlib.h>
#include <stdio.h>
#include <lexer.h>
#include <parser.h>
#include <semanter.h>
#include <lang/dgen.h>
#include <lang/sqf.h>
#include <utils.h>

#define OUTPUT_BUFFER_SIZE 4096

char output_buffer[OUTPUT_BUFFER_SIZE] = { 0 };

int main() {
    language_t* lang = get_dgen_language();

    char* source = read_file("test.dgen");

    token_list_t* tokens = lexer_analyze(lang, source);

    printf("Successfully lexed %d tokens (max %d) from test.dgen\n", tokens->count, TOKEN_LIST_SIZE);

    parser_t* parser = create_parser(lang, tokens);
    ast_node_t* root = parser_analyze(parser);

    printf("Successfully parsed %d tokens to AST\n", tokens->count);

    DELETE(parser);
    DELETE(tokens);
    DELETE(source);

    printf("AST root:\n");
    print_node(root, 0);

    sem_validate_node(lang, root);

    printf("Successfully analyzed AST\n");

    generator_t* gen = generator_create(&sqf_generator, output_buffer, OUTPUT_BUFFER_SIZE);

    generator_generate(gen, root);

    printf("Generated code:\n%s\n", output_buffer);

    parser_free_ast(&root);
    lang_free(lang);

    printf("\n\n\n");

    return 0;
}