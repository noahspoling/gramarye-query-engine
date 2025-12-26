#ifndef GRAMARYE_QUERY_QUERY_H
#define GRAMARYE_QUERY_QUERY_H

#include <stddef.h>
#include <stdint.h>

// Forward declarations
// Note: EntityId and ECS QueryResult are defined in gramarye_ecs headers
// This header should be included AFTER ECS headers in implementation files
typedef struct ECS ECS;

// Forward declare EntityId (actual type is uuid_key_t from gramarye_ecs)
// We use void* to avoid conflicts - will be cast in implementation
typedef void QueryEntityId;

// Query result structure for query engine (extends ECS QueryResult with data field)
// Always use QueryEngineResult as the struct name to avoid conflict
struct QueryEngineResult {
    QueryEntityId* entities;  // Opaque - cast to EntityId* in .c files
    size_t count;
    size_t capacity;
    void* data;  // Additional result data (for component values, etc.)
};

// Typedef - always use QueryEngineResult to avoid conflict with ECS QueryResult
// When ECS headers are included, QueryResult refers to ECS QueryResult
// So we use QueryEngineResult for the query engine's extended version
typedef struct QueryEngineResult QueryEngineResult;

// Query execution result
typedef enum {
    QUERY_SUCCESS,
    QUERY_ERROR_PARSE,
    QUERY_ERROR_EXECUTION,
    QUERY_ERROR_INVALID_SYNTAX
} QueryStatus;

// Forward declarations for EntityId (actual type from ECS)
typedef void EntityId_forward;

// Execute a query string
QueryStatus Query_execute(ECS* ecs, const char* queryString, QueryEngineResult* outResult);

// Free query result (query engine's QueryEngineResult, not ECS QueryResult)
void QueryEngineResult_free(QueryEngineResult* result);

// Get query result entities (returns EntityId* from ECS)
EntityId_forward* QueryEngineResult_get_entities(QueryEngineResult* result, size_t* outCount);

#endif // GRAMARYE_QUERY_QUERY_H

