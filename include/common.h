#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_NAME_SIZE       256

#define DECL_STRUCT(name)   typedef struct name##_t name##_t
#define DECL_ENUM(name)     typedef enum name##_e name##_e
#define DECL_UNION(name)    typedef union name##_u name##_u

#define STRUCT(name)    struct name##_t
#define ENUM(name)      enum name##_e
#define UNION(name)     union name##_u

#define NEW(name, type) type* name = (type*)malloc(sizeof(type)); memset(name, 0, sizeof(type))
#define DELETE(name)    if (name) free(name)