#include <lang.h>

const char* AST_NODE_STR[] = {
    AST_NODE_ENUM(UNKNOWN)
    FOREACH_AST_NODE(AST_NODE_ENUM_STR)
    AST_NODE_ENUM_STR(MAX)
};

void lang_insert_scope(scope_t* parent, scope_t* scope) {
    if (parent->childs)
        scope->next = parent->childs;

    parent->childs = scope;
}

void lang_scope_enter(language_t* lang) {
    NEW(scope, scope_t);

    scope->parent   = lang->current;
    scope->symbols  = NULL;

    if (lang->current)
        lang_insert_scope(lang->current, scope);

    if (!lang->root)
        lang->root = scope;

    lang->current = scope;
}

void lang_scope_leave(language_t* lang) {
    if (!lang->current->parent)
        return;

    lang->current = lang->current->parent;
}

void lang_insert_type(language_t* lang, type_t* type) {
    if (lang->types)
        type->next = lang->types;

    lang->types = type;
}

void lang_insert_symbol(language_t* lang, symbol_t* sym) {
    if (lang->current->symbols)
        sym->next = lang->current->symbols;

    lang->current->symbols = sym;
}

void lang_insert_param(symbol_t* sym, symbol_t* arg) {
    if (sym->params)
        arg->next = sym->params;

    sym->params = arg;
}

void lang_insert_field(type_t* type, symbol_t* field) {
    if (type->fields)
        field->next = type->fields;

    type->fields = field;
}

type_t* lang_find_type(language_t* lang, char* name) {
    type_t* type = lang->types;
    while (type) {
        if (strcmp(type->name, name) == 0)
            return type;

        type = type->next;
    }

    return NULL;
}

type_t* lang_find_struct(language_t* lang, char* name) {
    type_t* type = lang->types;
    while (type) {
        if (type->type == TYPE_TYPE_STRUCT && strcmp(type->name, name) == 0)
            return type;

        type = type->next;
    }

    return NULL;
}

type_t* lang_find_primitive_type(language_t* lang, primitive_type_e primitive) {
    type_t* type = lang->types;
    while (type) {
        if (type->type == TYPE_TYPE_PRIMITIVE && type->primitive == primitive)
            return type;

        type = type->next;
    }

    return NULL;
}

operator_t* lang_find_operator(language_t* lang, char* name) {
    for (uint32_t i = 0; i < lang->operator_size; i++) {
        if (strcmp(lang->operators[i].name, name) == 0)
            return &lang->operators[i];
    }

    return NULL;
}

keyword_t* lang_find_keyword(language_t* lang, char* name) {
    for (uint32_t i = 0; i < lang->keyword_size; i++) {
        if (strcmp(lang->keywords[i].name, name) == 0)
            return &lang->keywords[i];
    }

    return NULL;
}

symbol_t* lang_find_symbol(language_t* lang, char* name) {
    symbol_t* sym = lang->current->symbols;
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return sym;

        sym = sym->next;
    }

    if (lang->current->parent) {
        scope_t* scope = lang->current;
        lang->current = lang->current->parent;
        sym = lang_find_symbol(lang, name);
        lang->current = scope;
        return sym;
    }

    return NULL;
}

symbol_t* lang_find_field_in_struct(type_t* type, char* name) {
    symbol_t* sym = type->fields;
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return sym;

        sym = sym->next;
    }

    return NULL;
}

type_t* lang_add_type(language_t* lang, char* name, char* parent, uint32_t quantity) {
    type_t* type = lang_find_type(lang, name);

    if (type) {
        printf("Type %s already exists\n", name);
        return NULL;
    }

    type_t* parent_type = lang_find_type(lang, parent);

    if (!parent_type) {
        printf("Parent type %s does not exist\n", parent);
        return NULL;
    }

    NEW(new_type, type_t);

    strcpy(new_type->name, name);
    new_type->quantity = quantity;

    new_type->type = TYPE_TYPE_ALIAS;
    new_type->parent = parent_type;
    new_type->quantity = quantity;

    lang_insert_type(lang, new_type);

    return new_type;
}

type_t* lang_add_struct(language_t* lang, char* name) {
    type_t* type = lang_find_type(lang, name);

    if (type) {
        printf("Type %s already exists\n", name);
        return NULL;
    }

    NEW(new_type, type_t);

    strcpy(new_type->name, name);
    new_type->type = TYPE_TYPE_STRUCT;
    new_type->quantity = 0;
    new_type->fields = NULL;

    lang_insert_type(lang, new_type);

    return new_type;
}

type_t* lang_add_primitive_type(language_t* lang, char* name, primitive_type_e primitive) {
    type_t* type = lang_find_primitive_type(lang, primitive);

    if (type) {
        printf("Primitive type %s already exists\n", name);
        return NULL;
    }

    NEW(new_type, type_t);

    strcpy(new_type->name, name);
    new_type->type = TYPE_TYPE_PRIMITIVE;
    new_type->primitive = primitive;
    new_type->quantity = 0;

    lang_insert_type(lang, new_type);

    return new_type;
}

symbol_t* lang_add_symbol(language_t* lang, char* name, symbol_type_e type, type_t* type_info) {
    symbol_t* sym = lang_find_symbol(lang, name);

    if (sym) {
        printf("Symbol %s already exists\n", name);
        return NULL;
    }

    NEW(new_sym, symbol_t);

    strcpy(new_sym->name, name);
    new_sym->type = type;
    new_sym->type_info = type_info;

    lang_insert_symbol(lang, new_sym);

    return new_sym;
}

symbol_t* lang_symbol_add_argument(symbol_t* sym, char* name, type_t* type_info) {
    if (sym->type != SYMBOL_TYPE_FUNCTION) {
        printf("Symbol %s is not a function\n", sym->name);
        return NULL;
    }

    NEW(new_sym, symbol_t);

    strcpy(new_sym->name, name);
    new_sym->type = SYMBOL_TYPE_VARIABLE;
    new_sym->type_info = type_info;

    lang_insert_param(sym, new_sym);

    return new_sym;
}

symbol_t* lang_struct_add_field(type_t* type, char* name) {
    if (type->type != TYPE_TYPE_STRUCT) {
        printf("Type %s is not a struct\n", type->name);
        return NULL;
    }

    NEW(new_sym, symbol_t);

    strcpy(new_sym->name, name);

    lang_insert_field(type, new_sym);

    return new_sym;
}

void lang_free_scope(scope_t* root) {
    symbol_t* sym = root->symbols;
    while (sym) {
        symbol_t* next = sym->next;
        free(sym);
        sym = next;
    }

    scope_t* scope = root->childs;
    while (scope) {
        scope_t* next = scope->next;
        lang_free_scope(scope);
        scope = next;
    }

    free(root);
}

language_t* lang_create() {
    NEW(lang, language_t);

    lang_scope_enter(lang); // Create global scope

    return lang;
}

void lang_free(language_t* lang) {
    type_t* type = lang->types;
    while (type) {
        type_t* next = type->next;
        free(type);
        type = next;
    }

    lang_free_scope(lang->root);

    free(lang);
}