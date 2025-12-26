# Query Language Examples

## Basic Entity Queries

### Find entities with specific components
```sql
SELECT entities WHERE has(Position, Health)
```

### Find entities with any component
```sql
SELECT entities WHERE has_any(Position, Sprite)
```

### Count entities
```sql
COUNT entities WHERE has(Sprite)
```

## Component Inspection

### Show component data
```sql
SHOW Position OF entity 1234567890123456789:9876543210987654321
```

### Show all components for an entity
```sql
SHOW ALL OF entity 1234567890123456789:9876543210987654321
```

## Filtering

### Filter by component field values
```sql
SELECT entities WHERE Position.x > 100 AND Health.hp < 50
```

### Complex filters
```sql
SELECT entities WHERE Position.x > 100 OR Position.y < 0
```

### Multiple conditions
```sql
SELECT entities WHERE has(Position, Health) AND Position.x > 50 AND Health.hp > 0
```

## Interactive Shell Usage

```
query> SELECT entities WHERE has(Position)
Found 42 entities
  Entity: 1:1
  Entity: 1:2
  ...

query> SHOW Position OF entity 1:1
Position { x: 100, y: 200 }

query> COUNT entities WHERE has(Sprite)
42

query> EXIT
```

