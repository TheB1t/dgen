#pragma once

#include <common.h>
#include <lang.h>

#if defined(LOGS_WITH_FILE_AND_LINE)
#define SEMANTER_LOG(fmt, ...) printf("[SEMANTER] %s:%-4d | " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define SEMANTER_LOG(fmt, ...) printf("[SEMANTER] " fmt "\n", ##__VA_ARGS__)
#endif

extern void     sem_validate_expression(language_t* lang, ast_node_t* node);
extern void     sem_validate_node_list(language_t* lang, ast_node_t* node);
extern void     sem_validate_node(language_t* lang, ast_node_t* node);