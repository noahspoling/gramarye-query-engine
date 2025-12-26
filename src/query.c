#include "gramarye_query/query.h"
#include "gramarye_query/parser.h"
#include "gramarye_query/executor.h"
#include "gramarye_ecs/ecs.h"
#include "gramarye_ecs/entity.h"  // Get actual EntityId type
#include "mem.h"
#include <string.h>

// Cast macro for QueryEntityId* to EntityId*
#define QUERY_ENTITY_ID_PTR(ptr) ((EntityId*)(ptr))

// Compatibility wrapper - QueryResult maps to QueryEngineResult when ECS QueryResult is defined
QueryStatus Query_execute(ECS* ecs, const char* queryString, QueryEngineResult* outResult) {
    if (!ecs || !queryString || !outResult) {
        return QUERY_ERROR_INVALID_SYNTAX;
    }
    
    // Initialize result
    outResult->entities = NULL;
    outResult->count = 0;
    outResult->capacity = 0;
    outResult->data = NULL;
    
    // Parse query
    QueryParser* parser = QueryParser_new(queryString);
    if (!parser) {
        return QUERY_ERROR_PARSE;
    }
    
    QueryAST* ast = QueryParser_parse(parser);
    if (!ast) {
        QueryParser_destroy(parser);
        return QUERY_ERROR_PARSE;
    }
    
    // Execute query
    QueryStatus status = QueryExecutor_execute(ecs, ast, outResult);
    
    // Cleanup
    QueryAST_destroy(ast);
    QueryParser_destroy(parser);
    
    return status;
}

void QueryEngineResult_free(QueryEngineResult* result) {
    if (!result) return;
    
    if (result->entities) {
        FREE(result->entities);
    }
    
    if (result->data) {
        FREE(result->data);
    }
    
    result->count = 0;
    result->capacity = 0;
}

EntityId_forward* QueryEngineResult_get_entities(QueryEngineResult* result, size_t* outCount) {
    if (!result || !outCount) return NULL;
    
    *outCount = result->count;
    // Cast to EntityId_forward* (void*) to match declaration
    // Caller will cast to EntityId* when ECS headers are included
    return (EntityId_forward*)QUERY_ENTITY_ID_PTR(result->entities);
}

