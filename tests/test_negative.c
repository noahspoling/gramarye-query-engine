#include "test_common.h"
#include "gramarye_query/query.h"
#include "gramarye_ecs/ecs.h"
#include "arena.h"
#include "except.h"

static void test_negative_null_ecs(void) {
    printf("  Testing NULL ECS pointer...\n");
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(NULL, "SELECT entities WHERE has(Position)", &result);
    
    TEST_ASSERT_NE(status, QUERY_SUCCESS, "Should return error for NULL ECS");
}

static void test_negative_null_query_string(void) {
    printf("  Testing NULL query string...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, NULL, &result);
    
    TEST_ASSERT_NE(status, QUERY_SUCCESS, "Should return error for NULL query string");
}

static void test_negative_empty_query_string(void) {
    printf("  Testing empty query string...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "", &result);
    
    TEST_ASSERT_NE(status, QUERY_SUCCESS, "Should return error for empty query");
}

static void test_negative_malformed_entity_id(void) {
    printf("  Testing malformed entity ID...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(int) * 2);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "SHOW Position OF entity invalid", &result);
    
    TEST_ASSERT_NE(status, QUERY_SUCCESS, "Should return error for malformed entity ID");
}

static void test_negative_nonexistent_component_name(void) {
    printf("  Testing non-existent component name...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Nonexistent)", &result);
    
    // This should succeed but return empty result
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 0, "Should return empty result");
    QueryEngineResult_free(&result);
}

static void test_negative_nonexistent_entity(void) {
    printf("  Testing non-existent entity...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(int) * 2);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "SHOW Position OF entity 999999:999999", &result);
    
    TEST_ASSERT_NE(status, QUERY_SUCCESS, "Should return error for non-existent entity");
}

static void test_negative_invalid_query_syntax(void) {
    printf("  Testing invalid query syntax...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "INVALID QUERY SYNTAX", &result);
    
    TEST_ASSERT_NE(status, QUERY_SUCCESS, "Should return error for invalid syntax");
}

static void test_negative_empty_ecs(void) {
    printf("  Testing queries on empty ECS...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(int) * 2);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Position)", &result);
    
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 0, "Should return empty result");
    QueryEngineResult_free(&result);
}

static void test_negative_component_type_no_entities(void) {
    printf("  Testing component type with no entities...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(int) * 2);
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(int) * 2);
    
    // Create entity with Position but not Health
    EntityId entity = Entity_create(ECS_get_entity_registry(ecs));
    int pos[2] = {10, 20};
    ECS_add_component(ecs, entity, positionType, pos);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Health)", &result);
    
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 0, "Should return empty result");
    QueryEngineResult_free(&result);
}

bool test_negative(void) {
    printf("Running negative tests...\n");
    
    TRY
        test_negative_null_ecs();
        test_negative_null_query_string();
        test_negative_empty_query_string();
        test_negative_malformed_entity_id();
        test_negative_nonexistent_component_name();
        test_negative_nonexistent_entity();
        test_negative_invalid_query_syntax();
        test_negative_empty_ecs();
        test_negative_component_type_no_entities();
        
        printf("  ✓ All negative tests passed\n");
        return true;
    EXCEPT(Test_Failed)
        printf("  ✗ Negative test failed\n");
        return false;
    END_TRY;
}

