#include "test_common.h"
#include "gramarye_query/query.h"
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

typedef struct {
    int width;
    int height;
} Sprite;

static void test_integration_complex_scenario(void) {
    printf("  Testing complex ECS scenario...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    // Register multiple component types
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(Health));
    ComponentTypeId spriteType = ECS_register_component_type(ecs, "Sprite", sizeof(Sprite));
    
    // Create many entities with various component combinations
    EntityId entities[10];
    for (int i = 0; i < 10; i++) {
        entities[i] = Entity_create(ECS_get_entity_registry(ecs));
    }
    
    // Entity 0: Position only
    Position pos0 = {0, 0};
    ECS_add_component(ecs, entities[0], positionType, &pos0);
    
    // Entities 1-3: Position + Health
    for (int i = 1; i <= 3; i++) {
        Position pos = {i * 10, i * 20};
        Health health = {100 - i * 10, 100};
        ECS_add_component(ecs, entities[i], positionType, &pos);
        ECS_add_component(ecs, entities[i], healthType, &health);
    }
    
    // Entities 4-6: Position + Sprite
    for (int i = 4; i <= 6; i++) {
        Position pos = {i * 10, i * 20};
        Sprite sprite = {32, 32};
        ECS_add_component(ecs, entities[i], positionType, &pos);
        ECS_add_component(ecs, entities[i], spriteType, &sprite);
    }
    
    // Entities 7-9: All three components
    for (int i = 7; i <= 9; i++) {
        Position pos = {i * 10, i * 20};
        Health health = {100 - i * 10, 100};
        Sprite sprite = {32, 32};
        ECS_add_component(ecs, entities[i], positionType, &pos);
        ECS_add_component(ecs, entities[i], healthType, &health);
        ECS_add_component(ecs, entities[i], spriteType, &sprite);
    }
    
    // Test queries
    QueryEngineResult result;
    
    // Query: entities with Position only
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Position)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 10, "Should find all 10 entities with Position");
    QueryEngineResult_free(&result);
    
    // Query: entities with Position AND Health
    status = Query_execute(ecs, "SELECT entities WHERE has(Position, Health)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 6, "Should find 6 entities (1-3 and 7-9)");
    QueryEngineResult_free(&result);
    
    // Query: entities with Position OR Sprite
    status = Query_execute(ecs, "SELECT entities WHERE has_any(Position, Sprite)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 10, "Should find all 10 entities");
    QueryEngineResult_free(&result);
    
    // Query: entities without Health
    status = Query_execute(ecs, "SELECT entities WHERE not_has(Health)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 4, "Should find 4 entities (0 and 4-6)");
    QueryEngineResult_free(&result);
    
    // COUNT query
    status = Query_execute(ecs, "COUNT entities WHERE has(Position, Health, Sprite)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 3, "Count should be 3");
    QueryEngineResult_free(&result);
}

static void test_integration_large_ecs(void) {
    printf("  Testing with large ECS (100+ entities)...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(Health));
    
    // Create 100 entities
    EntityId entities[100];
    for (int i = 0; i < 100; i++) {
        entities[i] = Entity_create(ECS_get_entity_registry(ecs));
        
        // Add Position to all
        Position pos = {i, i * 2};
        ECS_add_component(ecs, entities[i], positionType, &pos);
        
        // Add Health to half
        if (i % 2 == 0) {
            Health health = {100, 100};
            ECS_add_component(ecs, entities[i], healthType, &health);
        }
    }
    
    QueryEngineResult result;
    
    // Query all with Position
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Position)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 100, "Should find all 100 entities");
    QueryEngineResult_free(&result);
    
    // Query with Position AND Health
    status = Query_execute(ecs, "SELECT entities WHERE has(Position, Health)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 50, "Should find 50 entities");
    QueryEngineResult_free(&result);
}

static void test_integration_empty_results(void) {
    printf("  Testing queries with empty results...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(Health));
    
    EntityId entity = Entity_create(ECS_get_entity_registry(ecs));
    Position pos = {10, 20};
    ECS_add_component(ecs, entity, positionType, &pos);
    
    QueryEngineResult result;
    
    // Query for component that doesn't exist on any entity
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Health)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 0, "Should return empty result");
    QueryEngineResult_free(&result);
}

static void test_integration_show_all_components(void) {
    printf("  Testing SHOW ALL query...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(Health));
    
    EntityId entity = Entity_create(ECS_get_entity_registry(ecs));
    Position pos = {42, 84};
    Health health = {75, 100};
    ECS_add_component(ecs, entity, positionType, &pos);
    ECS_add_component(ecs, entity, healthType, &health);
    
    char query[256];
    snprintf(query, sizeof(query), "SHOW ALL OF entity %llu:%llu", 
             (unsigned long long)entity.high, (unsigned long long)entity.low);
    
    QueryEngineResult result;
    QueryStatus status = Query_execute(ecs, query, &result);
    
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 2, "Should find 2 components");
    QueryEngineResult_free(&result);
}

bool test_integration(void) {
    printf("Running integration tests...\n");
    
    TRY
        test_integration_complex_scenario();
        test_integration_large_ecs();
        test_integration_empty_results();
        test_integration_show_all_components();
        
        printf("  ✓ All integration tests passed\n");
        return true;
    EXCEPT(Test_Failed)
        printf("  ✗ Integration test failed\n");
        return false;
    END_TRY;
}

