#ifndef GRAMARYE_QUERY_EXECUTOR_H
#define GRAMARYE_QUERY_EXECUTOR_H

#include "gramarye_ecs/ecs.h"
#include "gramarye_ecs/entity.h"
#include "parser.h"
// Include query.h to get QueryStatus and QueryEngineResult
// This is safe because we include it AFTER ECS headers, so ECS QueryResult is already defined
#include "query.h"

// Forward declarations
typedef struct QueryAST QueryAST;

// Execute a parsed query AST (uses QueryEngineResult to avoid conflict with ECS QueryResult)
QueryStatus QueryExecutor_execute(ECS* ecs, QueryAST* ast, QueryEngineResult* outResult);

// Execute a simple entity query by component types
QueryStatus QueryExecutor_query_entities(ECS* ecs, 
                                        const char* componentNames[], 
                                        size_t componentCount,
                                        QueryEngineResult* outResult);

// Execute component inspection query
QueryStatus QueryExecutor_inspect_component(ECS* ecs, 
                                           EntityId entity,
                                           const char* componentName,
                                           void** outData,
                                           size_t* outSize);

#endif // GRAMARYE_QUERY_EXECUTOR_H

