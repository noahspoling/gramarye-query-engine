# gramarye-query-engine

Query language for debugging and inspecting Entity Component System data in the Gramarye game engine.

## Overview

gramarye-query-engine provides a SQL-like query language for querying entities, inspecting components, and filtering data in an ECS. It's designed as an optional dependency for gramarye-ecs, useful for debugging and development tools.

## Features

- Entity queries by component types
- Component value inspection
- Filtering by component field values
- Aggregation operations (COUNT, etc.)
- Interactive REPL shell
- Programmatic query API

## Query Language Syntax

### Entity Queries

```sql
-- Find entities with specific components
SELECT entities WHERE has(Position, Health)

-- Find entities with any of the components
SELECT entities WHERE has_any(Position, Sprite)

-- Count entities
COUNT entities WHERE has(Sprite)

-- Find entities excluding certain components
SELECT entities WHERE not_has(Health)
```

### Component Inspection

```sql
-- Show component data for an entity
SHOW Position OF entity <id>

-- Show all components for an entity
SHOW ALL OF entity <id>
```

### Filtering

```sql
-- Filter by component field values
SELECT entities WHERE Position.x > 100 AND Health.hp < 50

-- Complex filters
SELECT entities WHERE Position.x > 100 OR Position.y < 0
```

### Interactive Commands

```
HELP          -- Show help
EXIT          -- Exit shell
CLEAR         -- Clear screen
```

## Usage

### Programmatic API

```c
#include "gramarye_query/query.h"
#include "gramarye_ecs/ecs.h"

ECS* ecs = ...;  // Your ECS instance

// Parse and execute a query
QueryResult result = Query_execute(ecs, "SELECT entities WHERE has(Position, Health)");

// Process results
for (size_t i = 0; i < result.count; i++) {
    EntityId entity = result.entities[i];
    // Process entity...
}

QueryResult_free(&result);
```

### Interactive Shell

```c
#include "gramarye_query/shell.h"

ECS* ecs = ...;  // Your ECS instance

// Start interactive shell
QueryShell* shell = QueryShell_new(ecs);
QueryShell_run(shell);
QueryShell_destroy(shell);
```

### Test Shell with Mock Data

A test shell executable with pre-populated mock data is available for testing:

```bash
# Build and run the test shell
./build-test-shell.sh

# Or just build it
./build-test-shell.sh --build

# Then run it manually
./build/test_shell
```

The test shell includes:
- Player entity (Position, Health, Sprite, Velocity)
- Enemy entities (Position, Health, Sprite)
- Projectile entities (Position, Velocity)
- NPC entities (Position, Sprite)
- Item entity (Position only)

Try example queries like:
- `SELECT entities WHERE has(Position)`
- `SELECT entities WHERE has(Position, Health)`
- `COUNT entities WHERE has(Sprite)`
- `SHOW Position OF entity <id>`

## Dependencies

- gramarye-ecs (required)
- gramarye-libcore (required)

## Building

```bash
mkdir build
cd build
cmake ..
make

# Optional: Build standalone query shell
cmake -DBUILD_QUERY_SHELL=ON ..
make
```

## Integration

This library is designed to be used as a submodule in game projects. It can be conditionally compiled for debug builds or integrated into development tools.

