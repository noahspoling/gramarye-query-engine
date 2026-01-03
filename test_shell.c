#include "gramarye_query/shell.h"
#include "gramarye_ecs/ecs.h"
#include "arena.h"
#include <stdio.h>

// Mock component structures
typedef struct {
    float x;
    float y;
} Position;

typedef struct {
    int hp;
    int maxHp;
} Health;

typedef struct {
    int width;
    int height;
    const char* texturePath;
} Sprite;

typedef struct {
    float speed;
    float direction;
} Velocity;

int main(void) {
    printf("Initializing ECS with mock data...\n");
    
    // Create ECS
    Arena_T arena = Arena_new();
    ECS* ecs = ECS_new(arena);
    
    if (!ecs) {
        fprintf(stderr, "Failed to create ECS\n");
        return 1;
    }
    
    // Register component types
    ComponentTypeId positionType = ECS_register_component_type(ecs, "Position", sizeof(Position));
    ComponentTypeId healthType = ECS_register_component_type(ecs, "Health", sizeof(Health));
    ComponentTypeId spriteType = ECS_register_component_type(ecs, "Sprite", sizeof(Sprite));
    ComponentTypeId velocityType = ECS_register_component_type(ecs, "Velocity", sizeof(Velocity));
    
    printf("Registered component types:\n");
    printf("  Position: %u\n", positionType);
    printf("  Health: %u\n", healthType);
    printf("  Sprite: %u\n", spriteType);
    printf("  Velocity: %u\n", velocityType);
    
    // Create some mock entities
    EntityId player = Entity_create(ECS_get_entity_registry(ecs));
    EntityId enemy1 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId enemy2 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId projectile1 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId projectile2 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId npc1 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId npc2 = Entity_create(ECS_get_entity_registry(ecs));
    EntityId item1 = Entity_create(ECS_get_entity_registry(ecs));
    
    // Player: Position + Health + Sprite + Velocity
    Position playerPos = {100.0f, 200.0f};
    Health playerHealth = {100, 100};
    Sprite playerSprite = {32, 32, "player.png"};
    Velocity playerVel = {5.0f, 0.0f};
    ECS_add_component(ecs, player, positionType, &playerPos);
    ECS_add_component(ecs, player, healthType, &playerHealth);
    ECS_add_component(ecs, player, spriteType, &playerSprite);
    ECS_add_component(ecs, player, velocityType, &playerVel);
    
    // Enemy1: Position + Health + Sprite
    Position enemy1Pos = {300.0f, 150.0f};
    Health enemy1Health = {50, 50};
    Sprite enemy1Sprite = {24, 24, "enemy.png"};
    ECS_add_component(ecs, enemy1, positionType, &enemy1Pos);
    ECS_add_component(ecs, enemy1, healthType, &enemy1Health);
    ECS_add_component(ecs, enemy1, spriteType, &enemy1Sprite);
    
    // Enemy2: Position + Health + Sprite
    Position enemy2Pos = {400.0f, 250.0f};
    Health enemy2Health = {75, 75};
    Sprite enemy2Sprite = {24, 24, "enemy.png"};
    ECS_add_component(ecs, enemy2, positionType, &enemy2Pos);
    ECS_add_component(ecs, enemy2, healthType, &enemy2Health);
    ECS_add_component(ecs, enemy2, spriteType, &enemy2Sprite);
    
    // Projectile1: Position + Velocity
    Position proj1Pos = {120.0f, 210.0f};
    Velocity proj1Vel = {10.0f, 0.0f};
    ECS_add_component(ecs, projectile1, positionType, &proj1Pos);
    ECS_add_component(ecs, projectile1, velocityType, &proj1Vel);
    
    // Projectile2: Position + Velocity
    Position proj2Pos = {130.0f, 220.0f};
    Velocity proj2Vel = {12.0f, -2.0f};
    ECS_add_component(ecs, projectile2, positionType, &proj2Pos);
    ECS_add_component(ecs, projectile2, velocityType, &proj2Vel);
    
    // NPC1: Position + Sprite
    Position npc1Pos = {500.0f, 300.0f};
    Sprite npc1Sprite = {16, 16, "npc.png"};
    ECS_add_component(ecs, npc1, positionType, &npc1Pos);
    ECS_add_component(ecs, npc1, spriteType, &npc1Sprite);
    
    // NPC2: Position + Sprite
    Position npc2Pos = {550.0f, 350.0f};
    Sprite npc2Sprite = {16, 16, "npc.png"};
    ECS_add_component(ecs, npc2, positionType, &npc2Pos);
    ECS_add_component(ecs, npc2, spriteType, &npc2Sprite);
    
    // Item1: Position only
    Position item1Pos = {200.0f, 100.0f};
    ECS_add_component(ecs, item1, positionType, &item1Pos);
    
    printf("\nCreated mock entities:\n");
    printf("  Player: %llu:%llu (Position, Health, Sprite, Velocity)\n", 
           (unsigned long long)player.high, (unsigned long long)player.low);
    printf("  Enemy1: %llu:%llu (Position, Health, Sprite)\n", 
           (unsigned long long)enemy1.high, (unsigned long long)enemy1.low);
    printf("  Enemy2: %llu:%llu (Position, Health, Sprite)\n", 
           (unsigned long long)enemy2.high, (unsigned long long)enemy2.low);
    printf("  Projectile1: %llu:%llu (Position, Velocity)\n", 
           (unsigned long long)projectile1.high, (unsigned long long)projectile1.low);
    printf("  Projectile2: %llu:%llu (Position, Velocity)\n", 
           (unsigned long long)projectile2.high, (unsigned long long)projectile2.low);
    printf("  NPC1: %llu:%llu (Position, Sprite)\n", 
           (unsigned long long)npc1.high, (unsigned long long)npc1.low);
    printf("  NPC2: %llu:%llu (Position, Sprite)\n", 
           (unsigned long long)npc2.high, (unsigned long long)npc2.low);
    printf("  Item1: %llu:%llu (Position only)\n", 
           (unsigned long long)item1.high, (unsigned long long)item1.low);
    
    printf("\n");
    printf("========================================\n");
    printf("Starting Query Shell\n");
    printf("========================================\n");
    printf("\n");
    printf("Available component types: Position, Health, Sprite, Velocity\n");
    printf("\n");
    printf("Try these example queries:\n");
    printf("  SELECT entities WHERE has(Position)\n");
    printf("  SELECT entities WHERE has(Position, Health)\n");
    printf("  SELECT entities WHERE has_any(Position, Sprite)\n");
    printf("  SELECT entities WHERE not_has(Health)\n");
    printf("  COUNT entities WHERE has(Sprite)\n");
    printf("  SHOW Position OF entity %llu:%llu\n", 
           (unsigned long long)player.high, (unsigned long long)player.low);
    printf("  SHOW Health OF entity %llu:%llu\n", 
           (unsigned long long)player.high, (unsigned long long)player.low);
    printf("  SHOW ALL OF entity %llu:%llu\n", 
           (unsigned long long)player.high, (unsigned long long)player.low);
    printf("\n");
    printf("Note: For SHOW queries, use actual component names (Position, Health, etc.)\n");
    printf("      not the word 'Component'\n");
    printf("\n");
    
    // Create and run the shell
    QueryShell* shell = QueryShell_new(ecs);
    if (!shell) {
        fprintf(stderr, "Failed to create query shell\n");
        ECS_destroy(ecs);
        Arena_free(arena);
        return 1;
    }
    
    // Customize prompt
    QueryShell_set_prompt(shell, "query> ");
    
    // Run interactive shell (blocks until EXIT)
    QueryShell_run(shell);
    
    // Cleanup
    QueryShell_destroy(shell);
    ECS_destroy(ecs);
    Arena_free(arena);
    
    printf("\nQuery shell exited. Goodbye!\n");
    return 0;
}

