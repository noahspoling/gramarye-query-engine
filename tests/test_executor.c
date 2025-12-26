#include "test_common.h"
#include "gramarye_query/query.h"
#include "gramarye_query/parser.h"
#include "gramarye_ecs/ecs.h"
#include "arena.h"
#include "except.h"
#include <string.h>

// Test component structures
typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    int hp;
    int maxHp;
} Health;

static void test_executor_select_has(void) {
    printf("  Testing SELECT with has() predicate...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    // Register components
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(Health));
    
    // Create entities
    EntityId entity1 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId entity2 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId entity3 = Entity_create(ECS_get_entity_registry(ecs));
    
    // Add components
    Position pos1 = {10, 20};
    ECS_add_component(ecs, entity1, positionType, &pos1);
    
    Position pos2 = {30, 40};
    Health health2 = {100, 100};
    ECS_add_component(ecs, entity2, positionType, &pos2);
    ECS_add_component(ecs, entity2, healthType, &health2);
    
    // Query
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Position)", &result);
    
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 2, "Should find 2 entities with Position");
    
    QueryEngineResult_free(&result);
}

static void test_executor_select_has_multiple(void) {
    printf("  Testing SELECT with has() multiple components...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(Health));
    
    EntityId entity1 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId entity2 = Entity_create(ECS_get_entity_registry(ecs));
    
    Position pos1 = {10, 20};
    ECS_add_component(ecs, entity1, positionType, &pos1);
    
    Position pos2 = {30, 40};
    Health health2 = {100, 100};
    ECS_add_component(ecs, entity2, positionType, &pos2);
    ECS_add_component(ecs, entity2, healthType, &health2);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Position, Health)", &result);
    
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 1, "Should find 1 entity with both Position and Health");
    
    QueryEngineResult_free(&result);
}

static void test_executor_select_has_any(void) {
    printf("  Testing SELECT with has_any() predicate...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(Health));
    
    EntityId entity1 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId entity2 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId entity3 = Entity_create(ECS_get_entity_registry(ecs));
    
    Position pos1 = {10, 20};
    ECS_add_component(ecs, entity1, positionType, &pos1);
    
    Health health2 = {100, 100};
    ECS_add_component(ecs, entity2, healthType, &health2);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has_any(Position, Health)", &result);
    
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 2, "Should find 2 entities with Position or Health");
    
    QueryEngineResult_free(&result);
}

static void test_executor_select_not_has(void) {
    printf("  Testing SELECT with not_has() predicate...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(Health));
    
    EntityId entity1 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId entity2 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId entity3 = Entity_create(ECS_get_entity_registry(ecs));
    
    Position pos1 = {10, 20};
    ECS_add_component(ecs, entity1, positionType, &pos1);
    
    Health health2 = {100, 100};
    ECS_add_component(ecs, entity2, healthType, &health2);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE not_has(Health)", &result);
    
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 2, "Should find 2 entities without Health");
    
    QueryEngineResult_free(&result);
}

static void test_executor_count(void) {
    printf("  Testing COUNT query...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    
    EntityId entity1 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId entity2 = Entity_create(ECS_get_entity_registry(ecs));
    
    Position pos1 = {10, 20};
    Position pos2 = {30, 40};
    ECS_add_component(ecs, entity1, positionType, &pos1);
    ECS_add_component(ecs, entity2, positionType, &pos2);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "COUNT entities WHERE has(Position)", &result);
    
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 2, "Count should be 2");
    
    QueryEngineResult_free(&result);
}

static void test_executor_show_component(void) {
    printf("  Testing SHOW component query...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    
    EntityId entity = Entity_create(ECS_get_entity_registry(ecs));
    Position pos = {42, 84};
    ECS_add_component(ecs, entity, positionType, &pos);
    
    // Format entity ID for query
    char query[256];
    snprintf(query, sizeof(query), "SHOW Position OF entity %llu:%llu", 
             (unsigned long long)entity.high, (unsigned long long)entity.low);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, query, &result);
    
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 1, "Should find one component");
    TEST_ASSERT_NOT_NULL(result.data, "Component data should exist");
    
    Position* retrievedPos = (Position*)result.data;
    TEST_ASSERT_EQ(retrievedPos->x, 42, "X coordinate should match");
    TEST_ASSERT_EQ(retrievedPos->y, 84, "Y coordinate should match");
    
    QueryEngineResult_free(&result);
}

static void test_executor_invalid_component_name(void) {
    printf("  Testing query with invalid component name...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Nonexistent)", &result);
    
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed (empty result)");
    TEST_ASSERT_EQ(result.count, 0, "Should return empty result");
    
    QueryEngineResult_free(&result);
}

bool test_executor(void) {
    printf("Running executor tests...\n");
    
    TRY
        test_executor_select_has();
        test_executor_select_has_multiple();
        test_executor_select_has_any();
        test_executor_select_not_has();
        test_executor_count();
        test_executor_show_component();
        test_executor_invalid_component_name();
        
        printf("  ✓ All executor tests passed\n");
        return true;
    EXCEPT(Test_Failed)
        printf("  ✗ Executor test failed\n");
        return false;
    END_TRY;
}

