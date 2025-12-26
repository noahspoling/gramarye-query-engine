#ifndef GRAMARYE_QUERY_PARSER_H
#define GRAMARYE_QUERY_PARSER_H

#include <stddef.h>
#include <stdint.h>

// Forward declarations
typedef struct QueryAST QueryAST;
typedef struct QueryParser QueryParser;

// Query AST node types (exposed for executor)
typedef enum {
    AST_SELECT,
    AST_COUNT,
    AST_SHOW,
    AST_WHERE,
    AST_HAS,
    AST_HAS_ANY,
    AST_NOT_HAS,
    AST_FILTER,
    AST_AND,
    AST_OR
} ASTNodeType;

// Data structures for AST nodes (exposed for executor)
typedef struct {
    char** componentNames;
    size_t count;
} ComponentList;

typedef struct {
    uint64_t high;
    uint64_t low;
} EntityIdData;

typedef struct {
    char* componentName;  // NULL for "ALL"
    EntityIdData* entityId;
} ShowQueryData;

// Query token types
typedef enum {
    TOKEN_SELECT,
    TOKEN_COUNT,
    TOKEN_SHOW,
    TOKEN_WHERE,
    TOKEN_HAS,
    TOKEN_HAS_ANY,
    TOKEN_NOT_HAS,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_OF,
    TOKEN_ENTITY,
    TOKEN_ENTITIES,
    TOKEN_ALL,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,  // >, <, =, >=, <=, !=
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

// Token structure
typedef struct Token {
    TokenType type;
    const char* value;
    size_t length;
    size_t line;
    size_t column;
} Token;

// Create a new parser
QueryParser* QueryParser_new(const char* queryString);

// Destroy parser
void QueryParser_destroy(QueryParser* parser);

// Get next token
Token QueryParser_next_token(QueryParser* parser);

// Peek at next token without consuming it
Token QueryParser_peek_token(QueryParser* parser);

// Parse query into AST
QueryAST* QueryParser_parse(QueryParser* parser);

// Destroy AST
void QueryAST_destroy(QueryAST* ast);

// Accessor functions for AST (for executor)
ASTNodeType QueryAST_get_type(QueryAST* ast);
void* QueryAST_get_data(QueryAST* ast);
QueryAST* QueryAST_get_left(QueryAST* ast);
QueryAST* QueryAST_get_right(QueryAST* ast);

#endif // GRAMARYE_QUERY_PARSER_H

