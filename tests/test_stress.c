#include "test_common.h"
#include "gramarye_query/query.h"
#include "gramarye_ecs/ecs.h"
#include "arena.h"
#include "mem.h"
#include "except.h"
#include <string.h>

// Test component structure
typedef struct {
    int x;
    int y;
} Position;

static void test_stress_large_ecs(void) {
    printf("  Testing with 1000+ entities...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(int) * 2);
    
    // Create 1000 entities
    const int ENTITY_COUNT = 1000;
    EntityId* entities = (EntityId*)ALLOC(sizeof(EntityId) * ENTITY_COUNT);
    
    for (int i = 0; i < ENTITY_COUNT; i++) {
        entities[i] = Entity_create(ECS_get_entity_registry(ecs));
        
        Position pos = {i, i * 2};
        ECS_add_component(ecs, entities[i], positionType, &pos);
        
        // Add Health to every other entity
        if (i % 2 == 0) {
            int health[2] = {100, 100};
            ECS_add_component(ecs, entities[i], healthType, health);
        }
    }
    
    QueryEngineResult result;
    
    // Query all with Position
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Position)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, ENTITY_COUNT, "Should find all entities");
    QueryEngineResult_free(&result);
    
    // Query with Position AND Health
    status = Query_execute(ecs, "SELECT entities WHERE has(Position, Health)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, ENTITY_COUNT / 2, "Should find half the entities");
    QueryEngineResult_free(&result);
    
    FREE(entities);
}

static void test_stress_many_component_types(void) {
    printf("  Testing with 50+ component types...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    // Register 50 component types
    const int COMPONENT_COUNT = 50;
    ComponentTypeId types[COMPONENT_COUNT];
    char componentNames[COMPONENT_COUNT][32];
    
    for (int i = 0; i < COMPONENT_COUNT; i++) {
        snprintf(componentNames[i], sizeof(componentNames[i]), "Component%d", i);
        types[i] = ECS_register_component_type(ecs, componentNames[i], sizeof(int));
        TEST_ASSERT_NE(types[i], COMPONENT_TYPE_INVALID, "Component registration should succeed");
    }
    
    // Create entities with various component combinations
    for (int i = 0; i < 100; i++) {
        EntityId entity = Entity_create(ECS_get_entity_registry(ecs));
        
        // Add first 10 components to each entity
        for (int j = 0; j < 10 && j < COMPONENT_COUNT; j++) {
            int data = i * 10 + j;
            ECS_add_component(ecs, entity, types[j], &data);
        }
    }
    
    QueryEngineResult result;
    
    // Query with first component
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Component0)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    TEST_ASSERT_EQ(result.count, 100, "Should find all 100 entities");
    QueryEngineResult_free(&result);
}

static void test_stress_complex_queries(void) {
    printf("  Testing complex queries with many conditions...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(int) * 2);
    ComponentTypeId spriteType = ECS_register_component_type(ecs, "Sprite", sizeof(int) * 2);
    ComponentTypeId velocityType = ECS_register_component_type(ecs, "Velocity", sizeof(int) * 2);
    ComponentTypeId damageType = ECS_register_component_type(ecs, "Damage", sizeof(int));
    
    // Create entities with various combinations
    for (int i = 0; i < 200; i++) {
        EntityId entity = Entity_create(ECS_get_entity_registry(ecs));
        
        Position pos = {i, i * 2};
        ECS_add_component(ecs, entity, positionType, &pos);
        
        if (i % 2 == 0) {
            int health[2] = {100, 100};
            ECS_add_component(ecs, entity, healthType, health);
        }
        
        if (i % 3 == 0) {
            int sprite[2] = {32, 32};
            ECS_add_component(ecs, entity, spriteType, sprite);
        }
        
        if (i % 5 == 0) {
            int velocity[2] = {1, 1};
            ECS_add_component(ecs, entity, velocityType, velocity);
        }
    }
    
    QueryEngineResult result;
    
    // Complex query: entities with Position AND Health AND Sprite
    QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Position, Health, Sprite)", &result);
    TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
    // Should find entities where i % 2 == 0 AND i % 3 == 0, i.e., i % 6 == 0
    TEST_ASSERT_EQ(result.count, 200 / 6 + 1, "Should find correct number of entities");
    QueryEngineResult_free(&result);
}

static void test_stress_memory_cleanup(void) {
    printf("  Testing memory cleanup...\n");
    
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    
    // Create many entities and run many queries
    for (int i = 0; i < 100; i++) {
        EntityId entity = Entity_create(ECS_get_entity_registry(ecs));
        Position pos = {i, i * 2};
        ECS_add_component(ecs, entity, positionType, &pos);
    }
    
    // Run many queries and ensure cleanup
    for (int i = 0; i < 100; i++) {
        QueryEngineResult result;
        QueryStatus status = Query_execute(ecs, "SELECT entities WHERE has(Position)", &result);
        TEST_ASSERT_EQ(status, QUERY_SUCCESS, "Query should succeed");
        TEST_ASSERT_EQ(result.count, 100, "Should find all entities");
        QueryEngineResult_free(&result);
    }
}

bool test_stress(void) {
    printf("Running stress tests...\n");
    
    TRY
        test_stress_large_ecs();
        test_stress_many_component_types();
        test_stress_complex_queries();
        test_stress_memory_cleanup();
        
        printf("  ✓ All stress tests passed\n");
        return true;
    EXCEPT(Test_Failed)
        printf("  ✗ Stress test failed\n");
        return false;
    END_TRY;
}

