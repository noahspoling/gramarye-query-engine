#include "gramarye_query/parser.h"
#include "mem.h"
#include <string.h>
#include <strings.h>  // For strncasecmp
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Query parser structure
struct QueryParser {
    const char* input;
    size_t position;
    size_t length;
    size_t line;
    size_t column;
};

// ASTNodeType is now defined in parser.h

// Data structures are now defined in parser.h

// Query AST structure
struct QueryAST {
    ASTNodeType type;
    void* data;  // ComponentList* for HAS/HAS_ANY/NOT_HAS, EntityIdData* for SHOW, NULL for others
    struct QueryAST* left;
    struct QueryAST* right;
    struct QueryAST* children;
    size_t childCount;
};

QueryParser* QueryParser_new(const char* queryString) {
    if (!queryString) return NULL;
    
    QueryParser* parser = (QueryParser*)ALLOC(sizeof(QueryParser));
    if (!parser) return NULL;
    
    parser->input = queryString;
    parser->position = 0;
    parser->length = strlen(queryString);
    parser->line = 1;
    parser->column = 1;
    
    return parser;
}

void QueryParser_destroy(QueryParser* parser) {
    if (parser) {
        FREE(parser);
    }
}

static void skip_whitespace(QueryParser* parser) {
    while (parser->position < parser->length && isspace(parser->input[parser->position])) {
        if (parser->input[parser->position] == '\n') {
            parser->line++;
            parser->column = 1;
        } else {
            parser->column++;
        }
        parser->position++;
    }
}

static bool is_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

static Token read_identifier(QueryParser* parser) {
    Token token;
    token.type = TOKEN_IDENTIFIER;
    token.line = parser->line;
    token.column = parser->column;
    token.value = &parser->input[parser->position];
    
    size_t start = parser->position;
    while (parser->position < parser->length && is_identifier_char(parser->input[parser->position])) {
        parser->position++;
        parser->column++;
    }
    
    token.length = parser->position - start;
    
    // Check for keywords (case-insensitive, check longer ones first to avoid partial matches)
    // Helper macro for case-insensitive comparison
    #define MATCH_KEYWORD(str, len) (token.length == len && strncasecmp(token.value, str, len) == 0)
    
    if (MATCH_KEYWORD("ENTITIES", 8)) {
        token.type = TOKEN_ENTITIES;
    } else if (MATCH_KEYWORD("HAS_ANY", 7)) {
        token.type = TOKEN_HAS_ANY;
    } else if (MATCH_KEYWORD("NOT_HAS", 7)) {
        token.type = TOKEN_NOT_HAS;
    } else if (MATCH_KEYWORD("SELECT", 6)) {
        token.type = TOKEN_SELECT;
    } else if (MATCH_KEYWORD("ENTITY", 6)) {
        token.type = TOKEN_ENTITY;
    } else if (MATCH_KEYWORD("COUNT", 5)) {
        token.type = TOKEN_COUNT;
    } else if (MATCH_KEYWORD("WHERE", 5)) {
        token.type = TOKEN_WHERE;
    } else if (MATCH_KEYWORD("SHOW", 4)) {
        token.type = TOKEN_SHOW;
    } else if (MATCH_KEYWORD("HAS", 3)) {
        token.type = TOKEN_HAS;
    } else if (MATCH_KEYWORD("ALL", 3)) {
        token.type = TOKEN_ALL;
    } else if (MATCH_KEYWORD("AND", 3)) {
        token.type = TOKEN_AND;
    } else if (MATCH_KEYWORD("OF", 2)) {
        token.type = TOKEN_OF;
    } else if (MATCH_KEYWORD("OR", 2)) {
        token.type = TOKEN_OR;
    }
    
    #undef MATCH_KEYWORD
    
    return token;
}

static Token read_number(QueryParser* parser) {
    Token token;
    token.type = TOKEN_NUMBER;
    token.line = parser->line;
    token.column = parser->column;
    token.value = &parser->input[parser->position];
    
    size_t start = parser->position;
    while (parser->position < parser->length && isdigit(parser->input[parser->position])) {
        parser->position++;
        parser->column++;
    }
    
    token.length = parser->position - start;
    return token;
}

Token QueryParser_next_token(QueryParser* parser) {
    if (!parser) {
        Token error = {TOKEN_ERROR, NULL, 0, 0, 0};
        return error;
    }
    
    skip_whitespace(parser);
    
    if (parser->position >= parser->length) {
        Token eof = {TOKEN_EOF, NULL, 0, parser->line, parser->column};
        return eof;
    }
    
    char c = parser->input[parser->position];
    
    // Single character tokens
    if (c == '(') {
        parser->position++;
        parser->column++;
        Token token = {TOKEN_LPAREN, &parser->input[parser->position - 1], 1, parser->line, parser->column - 1};
        return token;
    }
    if (c == ')') {
        parser->position++;
        parser->column++;
        Token token = {TOKEN_RPAREN, &parser->input[parser->position - 1], 1, parser->line, parser->column - 1};
        return token;
    }
    if (c == '.') {
        parser->position++;
        parser->column++;
        Token token = {TOKEN_DOT, &parser->input[parser->position - 1], 1, parser->line, parser->column - 1};
        return token;
    }
    if (c == ',') {
        parser->position++;
        parser->column++;
        Token token = {TOKEN_COMMA, &parser->input[parser->position - 1], 1, parser->line, parser->column - 1};
        return token;
    }
    
    // Operators
    if (c == '>' || c == '<' || c == '=' || c == '!') {
        size_t start = parser->position;
        parser->position++;
        parser->column++;
        
        // Check for >=, <=, !=
        if (parser->position < parser->length && parser->input[parser->position] == '=') {
            parser->position++;
            parser->column++;
        }
        
        Token token = {TOKEN_OPERATOR, &parser->input[start], parser->position - start, parser->line, parser->column - (parser->position - start)};
        return token;
    }
    
    // Numbers
    if (isdigit(c)) {
        return read_number(parser);
    }
    
    // Identifiers and keywords
    if (isalpha(c) || c == '_') {
        return read_identifier(parser);
    }
    
    // Unknown character
    Token error = {TOKEN_ERROR, &parser->input[parser->position], 1, parser->line, parser->column};
    parser->position++;
    parser->column++;
    return error;
}

Token QueryParser_peek_token(QueryParser* parser) {
    if (!parser) {
        Token error = {TOKEN_ERROR, NULL, 0, 0, 0};
        return error;
    }
    
    size_t savedPos = parser->position;
    size_t savedLine = parser->line;
    size_t savedCol = parser->column;
    
    Token token = QueryParser_next_token(parser);
    
    parser->position = savedPos;
    parser->line = savedLine;
    parser->column = savedCol;
    
    return token;
}

// Helper: Parse a component name list (e.g., "Position, Health, Sprite")
static ComponentList* parse_component_list(QueryParser* parser) {
    ComponentList* list = (ComponentList*)ALLOC(sizeof(ComponentList));
    if (!list) return NULL;
    
    list->componentNames = NULL;
    list->count = 0;
    
    // Expect opening parenthesis
    Token token = QueryParser_next_token(parser);
    if (token.type != TOKEN_LPAREN) {
        FREE(list);
        return NULL;
    }
    
    // Allocate initial array
    size_t capacity = 4;
    list->componentNames = (char**)ALLOC(sizeof(char*) * capacity);
    if (!list->componentNames) {
        FREE(list);
        return NULL;
    }
    
    // Parse component names
    bool first = true;
    while (1) {
        token = QueryParser_next_token(parser);
        
        if (token.type == TOKEN_RPAREN) {
            break;
        }
        
        if (!first) {
            // Expect comma between names
            if (token.type != TOKEN_COMMA) {
                // Free allocated names
                for (size_t i = 0; i < list->count; i++) {
                    FREE(list->componentNames[i]);
                }
                FREE(list->componentNames);
                FREE(list);
                return NULL;
            }
            // Consume comma and get next token
            token = QueryParser_next_token(parser);
        }
        
        if (token.type == TOKEN_IDENTIFIER) {
            // Grow array if needed
            if (list->count >= capacity) {
                capacity *= 2;
                char** newNames = (char**)ALLOC(sizeof(char*) * capacity);
                if (!newNames) {
                    for (size_t i = 0; i < list->count; i++) {
                        FREE(list->componentNames[i]);
                    }
                    FREE(list->componentNames);
                    FREE(list);
                    return NULL;
                }
                memcpy(newNames, list->componentNames, sizeof(char*) * list->count);
                FREE(list->componentNames);
                list->componentNames = newNames;
            }
            
            // Copy component name
            list->componentNames[list->count] = (char*)ALLOC(token.length + 1);
            if (!list->componentNames[list->count]) {
                for (size_t i = 0; i < list->count; i++) {
                    FREE(list->componentNames[i]);
                }
                FREE(list->componentNames);
                FREE(list);
                return NULL;
            }
            strncpy(list->componentNames[list->count], token.value, token.length);
            list->componentNames[list->count][token.length] = '\0';
            list->count++;
            
            first = false;
        } else {
            // Free allocated names
            for (size_t i = 0; i < list->count; i++) {
                FREE(list->componentNames[i]);
            }
            FREE(list->componentNames);
            FREE(list);
            return NULL;
        }
    }
    
    // Reject empty component lists
    if (list->count == 0) {
        FREE(list->componentNames);
        FREE(list);
        return NULL;
    }
    
    return list;
}

// Helper: Parse entity ID (format: "high:low")
static EntityIdData* parse_entity_id(QueryParser* parser) {
    // Entity ID format: "high:low" where both are uint64_t
    // Parse manually since colon isn't handled by tokenizer
    
    skip_whitespace(parser);
    
    if (parser->position >= parser->length) {
        return NULL;
    }
    
    size_t startPos = parser->position;
    
    // Find the colon
    size_t pos = startPos;
    while (pos < parser->length && isdigit(parser->input[pos])) {
        pos++;
    }
    
    if (pos >= parser->length || parser->input[pos] != ':') {
        return NULL;
    }
    
    // Parse high part
    char* endptr;
    uint64_t high = strtoull(&parser->input[startPos], &endptr, 10);
    if (endptr != &parser->input[pos]) {
        return NULL;
    }
    
    // Skip colon
    pos++;
    if (pos >= parser->length) {
        return NULL;
    }
    
    size_t lowStart = pos;
    
    // Parse low part
    while (pos < parser->length && isdigit(parser->input[pos])) {
        pos++;
    }
    
    if (pos == lowStart) {
        return NULL; // No digits after colon
    }
    
    uint64_t low = strtoull(&parser->input[lowStart], &endptr, 10);
    if (endptr != &parser->input[pos]) {
        return NULL;
    }
    
    // Update parser position
    parser->position = pos;
    parser->column += (pos - parser->position); // Will be recalculated on next token
    
    EntityIdData* idData = (EntityIdData*)ALLOC(sizeof(EntityIdData));
    if (!idData) return NULL;
    
    idData->high = high;
    idData->low = low;
    
    return idData;
}

// Helper: Parse predicate (has, has_any, not_has)
static QueryAST* parse_predicate(QueryParser* parser) {
    Token token = QueryParser_next_token(parser);
    
    QueryAST* predicate = (QueryAST*)ALLOC(sizeof(QueryAST));
    if (!predicate) return NULL;
    
    predicate->left = NULL;
    predicate->right = NULL;
    predicate->children = NULL;
    predicate->childCount = 0;
    
    if (token.type == TOKEN_HAS) {
        predicate->type = AST_HAS;
        ComponentList* list = parse_component_list(parser);
        if (!list) {
            FREE(predicate);
            return NULL;
        }
        predicate->data = list;
    } else if (token.type == TOKEN_HAS_ANY) {
        predicate->type = AST_HAS_ANY;
        ComponentList* list = parse_component_list(parser);
        if (!list) {
            FREE(predicate);
            return NULL;
        }
        predicate->data = list;
    } else if (token.type == TOKEN_NOT_HAS) {
        predicate->type = AST_NOT_HAS;
        ComponentList* list = parse_component_list(parser);
        if (!list) {
            FREE(predicate);
            return NULL;
        }
        predicate->data = list;
    } else {
        FREE(predicate);
        return NULL;
    }
    
    return predicate;
}

QueryAST* QueryParser_parse(QueryParser* parser) {
    if (!parser) return NULL;
    
    Token token = QueryParser_next_token(parser);
    
    QueryAST* ast = (QueryAST*)ALLOC(sizeof(QueryAST));
    if (!ast) return NULL;
    
    ast->left = NULL;
    ast->right = NULL;
    ast->children = NULL;
    ast->childCount = 0;
    ast->data = NULL;
    
    // Parse query type: SELECT, COUNT, or SHOW
    if (token.type == TOKEN_SELECT) {
        ast->type = AST_SELECT;
        
        // Expect "entities"
        token = QueryParser_next_token(parser);
        if (token.type != TOKEN_ENTITIES) {
            FREE(ast);
            return NULL;
        }
        
        // Optional WHERE clause
        token = QueryParser_peek_token(parser);
        if (token.type == TOKEN_WHERE) {
            QueryParser_next_token(parser); // Consume WHERE
            
            QueryAST* predicate = parse_predicate(parser);
            if (!predicate) {
                FREE(ast);
                return NULL;
            }
            
            ast->left = predicate;
        }
        
        // Should be EOF now
        token = QueryParser_next_token(parser);
        if (token.type != TOKEN_EOF) {
            QueryAST_destroy(ast);
            return NULL;
        }
        
    } else if (token.type == TOKEN_COUNT) {
        ast->type = AST_COUNT;
        
        // Expect "entities"
        token = QueryParser_next_token(parser);
        if (token.type != TOKEN_ENTITIES) {
            FREE(ast);
            return NULL;
        }
        
        // Optional WHERE clause
        token = QueryParser_peek_token(parser);
        if (token.type == TOKEN_WHERE) {
            QueryParser_next_token(parser); // Consume WHERE
            
            QueryAST* predicate = parse_predicate(parser);
            if (!predicate) {
                FREE(ast);
                return NULL;
            }
            
            ast->left = predicate;
        }
        
        // Should be EOF now
        token = QueryParser_next_token(parser);
        if (token.type != TOKEN_EOF) {
            QueryAST_destroy(ast);
            return NULL;
        }
        
    } else if (token.type == TOKEN_SHOW) {
        ast->type = AST_SHOW;
        
        // Parse component name or ALL
        token = QueryParser_next_token(parser);
        
        ShowQueryData* showData = (ShowQueryData*)ALLOC(sizeof(ShowQueryData));
        if (!showData) {
            FREE(ast);
            return NULL;
        }
        showData->componentName = NULL;
        showData->entityId = NULL;
        
        if (token.type == TOKEN_ALL) {
            // SHOW ALL OF entity <id>
            showData->componentName = NULL; // NULL means ALL
            
        } else if (token.type == TOKEN_IDENTIFIER) {
            // SHOW ComponentName OF entity <id>
            showData->componentName = (char*)ALLOC(token.length + 1);
            if (!showData->componentName) {
                FREE(showData);
                FREE(ast);
                return NULL;
            }
            strncpy(showData->componentName, token.value, token.length);
            showData->componentName[token.length] = '\0';
            
        } else {
            FREE(showData);
            FREE(ast);
            return NULL;
        }
        
        // Expect "OF"
        token = QueryParser_next_token(parser);
        if (token.type != TOKEN_OF) {
            if (showData->componentName) FREE(showData->componentName);
            FREE(showData);
            FREE(ast);
            return NULL;
        }
        
        // Expect "entity"
        token = QueryParser_next_token(parser);
        if (token.type != TOKEN_ENTITY) {
            if (showData->componentName) FREE(showData->componentName);
            FREE(showData);
            FREE(ast);
            return NULL;
        }
        
        // Parse entity ID
        showData->entityId = parse_entity_id(parser);
        if (!showData->entityId) {
            if (showData->componentName) FREE(showData->componentName);
            FREE(showData);
            FREE(ast);
            return NULL;
        }
        
        ast->data = showData;
        
        // Should be EOF now
        token = QueryParser_next_token(parser);
        if (token.type != TOKEN_EOF) {
            QueryAST_destroy(ast);
            return NULL;
        }
        
    } else {
        FREE(ast);
        return NULL;
    }
    
    return ast;
}

void QueryAST_destroy(QueryAST* ast) {
    if (!ast) return;
    
    // Recursively destroy children
    if (ast->children) {
        for (size_t i = 0; i < ast->childCount; i++) {
            QueryAST_destroy(&ast->children[i]);
        }
        FREE(ast->children);
    }
    
    // Recursively destroy left and right subtrees
    if (ast->left) {
        QueryAST_destroy(ast->left);
    }
    if (ast->right) {
        QueryAST_destroy(ast->right);
    }
    
    // Free data based on AST type
    if (ast->data) {
        if (ast->type == AST_HAS || ast->type == AST_HAS_ANY || ast->type == AST_NOT_HAS) {
            // ComponentList
            ComponentList* list = (ComponentList*)ast->data;
            if (list->componentNames) {
                for (size_t i = 0; i < list->count; i++) {
                    FREE(list->componentNames[i]);
                }
                FREE(list->componentNames);
            }
            FREE(list);
        } else if (ast->type == AST_SHOW) {
            // ShowQueryData
            ShowQueryData* showData = (ShowQueryData*)ast->data;
            if (showData->componentName) {
                FREE(showData->componentName);
            }
            if (showData->entityId) {
                FREE(showData->entityId);
            }
            FREE(showData);
        } else {
            // Generic data (shouldn't happen, but be safe)
            FREE(ast->data);
        }
    }
    
    FREE(ast);
}

// Accessor functions for AST
ASTNodeType QueryAST_get_type(QueryAST* ast) {
    if (!ast) return AST_SELECT; // Default
    return ast->type;
}

void* QueryAST_get_data(QueryAST* ast) {
    if (!ast) return NULL;
    return ast->data;
}

QueryAST* QueryAST_get_left(QueryAST* ast) {
    if (!ast) return NULL;
    return ast->left;
}

QueryAST* QueryAST_get_right(QueryAST* ast) {
    if (!ast) return NULL;
    return ast->right;
}

