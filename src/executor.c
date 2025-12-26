#include "gramarye_query/executor.h"
#include "gramarye_query/parser.h"
#include "gramarye_query/query.h"  // Include after executor.h to get full QueryResult definition
#include "gramarye_ecs/ecs.h"
#include "gramarye_ecs/query.h"  // Include ECS query.h for ECS QueryResult
#include "gramarye_ecs/entity.h"
#include "gramarye_ecs/component.h"
#include "mem.h"
#include <string.h>
#include <stdint.h>

// Now we have both:
// - ECS QueryResult (from gramarye_ecs/query.h) - used for ECS query functions
// - QueryResult (from gramarye_query/query.h) - query engine's extended version with data field

QueryStatus QueryExecutor_execute(ECS* ecs, QueryAST* ast, QueryEngineResult* outResult) {
    if (!ecs || !ast || !outResult) {
        return QUERY_ERROR_EXECUTION;
    }
    
    // Initialize result
    outResult->entities = NULL;
    outResult->count = 0;
    outResult->capacity = 0;
    outResult->data = NULL;
    
    ASTNodeType queryType = QueryAST_get_type(ast);
    
    if (queryType == AST_SELECT || queryType == AST_COUNT) {
        // SELECT or COUNT entities WHERE ...
        QueryAST* predicate = QueryAST_get_left(ast);
        
        if (!predicate) {
            // No WHERE clause - return all entities (not typical, but handle it)
            // For now, return empty result
            return QUERY_SUCCESS;
        }
        
        ASTNodeType predicateType = QueryAST_get_type(predicate);
        ComponentList* componentList = (ComponentList*)QueryAST_get_data(predicate);
        
        if (!componentList || componentList->count == 0) {
            return QUERY_SUCCESS; // Empty result
        }
        
        // Convert component names to ComponentTypeIds
        ComponentTypeId* typeIds = (ComponentTypeId*)ALLOC(sizeof(ComponentTypeId) * componentList->count);
        if (!typeIds) {
            return QUERY_ERROR_EXECUTION;
        }
        
        size_t validCount = 0;
        for (size_t i = 0; i < componentList->count; i++) {
            ComponentTypeId typeId = ECS_get_component_type_by_name(ecs, componentList->componentNames[i]);
            if (typeId != COMPONENT_TYPE_INVALID) {
                typeIds[validCount++] = typeId;
            }
        }
        
        if (validCount == 0) {
            FREE(typeIds);
            return QUERY_SUCCESS; // No valid components, return empty result
        }
        
        // Query entities based on predicate type
        struct QueryResult ecsResult;
        if (predicateType == AST_HAS) {
            ecsResult = ECS_query_entities(ecs, typeIds, validCount);
        } else if (predicateType == AST_HAS_ANY) {
            ecsResult = ECS_query_entities_any(ecs, typeIds, validCount);
        } else if (predicateType == AST_NOT_HAS) {
            ecsResult = ECS_query_entities_excluding(ecs, typeIds, validCount);
        } else {
            FREE(typeIds);
            return QUERY_ERROR_EXECUTION;
        }
        
        if (queryType == AST_COUNT) {
            // For COUNT, just store the count
            outResult->count = ecsResult.count;
            outResult->entities = NULL;
            outResult->capacity = 0;
            outResult->data = NULL;
            // Use ECS QueryResult_free for ECS QueryResult
            QueryResult_free(&ecsResult);  // This is ECS QueryResult_free from gramarye_ecs/query.h
        } else {
            // For SELECT, copy entities
            if (ecsResult.count > 0) {
                outResult->entities = (QueryEntityId*)ALLOC(sizeof(EntityId) * ecsResult.count);
                if (outResult->entities) {
                    memcpy(outResult->entities, ecsResult.entities, sizeof(EntityId) * ecsResult.count);
                    outResult->count = ecsResult.count;
                    outResult->capacity = ecsResult.count;
                    outResult->data = NULL;
                }
            }
            // Use ECS QueryResult_free for ECS QueryResult (from gramarye_ecs/query.h)
            QueryResult_free(&ecsResult);
        }
        
        FREE(typeIds);
        
    } else if (queryType == AST_SHOW) {
        // SHOW ComponentName OF entity <id> or SHOW ALL OF entity <id>
        ShowQueryData* showData = (ShowQueryData*)QueryAST_get_data(ast);
        
        if (!showData || !showData->entityId) {
            return QUERY_ERROR_EXECUTION;
        }
        
        // Convert EntityIdData to EntityId (uuid_key_t from ECS)
        EntityId entity;
        entity.high = showData->entityId->high;
        entity.low = showData->entityId->low;
        
        // Check if entity exists
        if (!Entity_exists(ECS_get_entity_registry(ecs), entity)) {
            return QUERY_ERROR_EXECUTION;
        }
        
        if (showData->componentName == NULL) {
            // SHOW ALL - get all components for entity
            const size_t MAX_COMPONENTS = 64;
            ComponentTypeId componentTypes[MAX_COMPONENTS];
            size_t componentCount = 0;
            
            ECS_get_entity_components(ecs, entity, componentTypes, &componentCount, MAX_COMPONENTS);
            
            // Store component info in result data
            // For now, just store the count in result->count
            // In a full implementation, we'd store component data
            outResult->count = componentCount;
            outResult->entities = NULL;
            outResult->capacity = 0;
            // TODO: Store component data in outResult->data
        } else {
            // SHOW single component
            ComponentTypeId typeId = ECS_get_component_type_by_name(ecs, showData->componentName);
            if (typeId == COMPONENT_TYPE_INVALID) {
                return QUERY_ERROR_EXECUTION;
            }
            
            void* componentData = ECS_get_component(ecs, entity, typeId);
            if (!componentData) {
                return QUERY_ERROR_EXECUTION;
            }
            
            ComponentType* type = ECS_get_component_type(ecs, typeId);
            if (!type) {
                return QUERY_ERROR_EXECUTION;
            }
            
            // Copy component data (ECS data is internal and shouldn't be freed by query engine)
            outResult->data = ALLOC(type->size);
            if (!outResult->data) {
                return QUERY_ERROR_EXECUTION;
            }
            memcpy(outResult->data, componentData, type->size);
            
            // Store component data
            outResult->count = 1; // One component
            outResult->entities = NULL;
            outResult->capacity = 0;
        }
        
    } else {
        return QUERY_ERROR_EXECUTION;
    }
    
    return QUERY_SUCCESS;
}

QueryStatus QueryExecutor_query_entities(ECS* ecs, 
                                        const char* componentNames[], 
                                        size_t componentCount,
                                        QueryEngineResult* outResult) {
    if (!ecs || !componentNames || !outResult) {
        return QUERY_ERROR_EXECUTION;
    }
    
    // Initialize result
    outResult->entities = NULL;
    outResult->count = 0;
    outResult->capacity = 0;
    outResult->data = NULL;
    
    // Convert component names to IDs
    ComponentTypeId* typeIds = (ComponentTypeId*)ALLOC(sizeof(ComponentTypeId) * componentCount);
    if (!typeIds) {
        return QUERY_ERROR_EXECUTION;
    }
    
    size_t validCount = 0;
    for (size_t i = 0; i < componentCount; i++) {
        ComponentTypeId typeId = ECS_get_component_type_by_name(ecs, componentNames[i]);
        if (typeId != COMPONENT_TYPE_INVALID) {
            typeIds[validCount++] = typeId;
        }
    }
    
    if (validCount == 0) {
        FREE(typeIds);
        return QUERY_SUCCESS;  // No valid components, return empty result
    }
    
    // Query entities
    struct QueryResult ecsResult = ECS_query_entities(ecs, typeIds, validCount);
    
    // Copy results
    outResult->entities = (QueryEntityId*)ALLOC(sizeof(EntityId) * ecsResult.count);
    if (outResult->entities) {
        memcpy(outResult->entities, ecsResult.entities, sizeof(EntityId) * ecsResult.count);
        outResult->count = ecsResult.count;
        outResult->capacity = ecsResult.count;
        outResult->data = NULL;
    }
    
    QueryResult_free(&ecsResult);
    FREE(typeIds);
    
    return QUERY_SUCCESS;
}

QueryStatus QueryExecutor_inspect_component(ECS* ecs, 
                                           EntityId entity,
                                           const char* componentName,
                                           void** outData,
                                           size_t* outSize) {
    if (!ecs || !componentName || !outData || !outSize) {
        return QUERY_ERROR_EXECUTION;
    }
    
    ComponentTypeId typeId = ECS_get_component_type_by_name(ecs, componentName);
    if (typeId == COMPONENT_TYPE_INVALID) {
        return QUERY_ERROR_EXECUTION;
    }
    
    void* data = ECS_get_component(ecs, entity, typeId);
    if (!data) {
        return QUERY_ERROR_EXECUTION;
    }
    
    ComponentType* type = ECS_get_component_type(ecs, typeId);
    if (!type) {
        return QUERY_ERROR_EXECUTION;
    }
    
    *outData = data;
    *outSize = type->size;
    
    return QUERY_SUCCESS;
}

