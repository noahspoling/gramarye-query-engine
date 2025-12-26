#include "gramarye_query/shell.h"
#include "gramarye_query/query.h"
#include "gramarye_ecs/entity.h"  // Get EntityId type
#include "mem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Query shell structure
struct QueryShell {
    ECS* ecs;
    char* prompt;
    bool historyEnabled;
};

QueryShell* QueryShell_new(ECS* ecs) {
    if (!ecs) return NULL;
    
    QueryShell* shell = (QueryShell*)ALLOC(sizeof(QueryShell));
    if (!shell) return NULL;
    
    shell->ecs = ecs;
    shell->prompt = (char*)ALLOC(32);
    if (shell->prompt) {
        strcpy(shell->prompt, "query> ");
    }
    shell->historyEnabled = false;
    
    return shell;
}

void QueryShell_destroy(QueryShell* shell) {
    if (!shell) return;
    
    if (shell->prompt) {
        FREE(shell->prompt);
    }
    
    FREE(shell);
}

void QueryShell_set_prompt(QueryShell* shell, const char* prompt) {
    if (!shell || !prompt) return;
    
    if (shell->prompt) {
        FREE(shell->prompt);
    }
    
    shell->prompt = (char*)ALLOC(strlen(prompt) + 1);
    if (shell->prompt) {
        strcpy(shell->prompt, prompt);
    }
}

void QueryShell_set_history_enabled(QueryShell* shell, bool enabled) {
    if (shell) {
        shell->historyEnabled = enabled;
    }
}

void QueryShell_process_command(QueryShell* shell, const char* command) {
    if (!shell || !command) return;
    
    // Skip empty commands
    if (strlen(command) == 0) return;
    
    // Handle built-in commands
    if (strcmp(command, "HELP") == 0 || strcmp(command, "help") == 0) {
        printf("Query Language Commands:\n");
        printf("  SELECT entities WHERE has(ComponentName1, ComponentName2)\n");
        printf("  SELECT entities WHERE has_any(ComponentName1, ComponentName2)\n");
        printf("  SELECT entities WHERE not_has(ComponentName)\n");
        printf("  COUNT entities WHERE has(ComponentName)\n");
        printf("  SHOW ComponentName OF entity <high>:<low>\n");
        printf("  SHOW ALL OF entity <high>:<low>\n");
        printf("  HELP - Show this help\n");
        printf("  EXIT - Exit shell\n");
        printf("\n");
        printf("Note: Replace ComponentName with actual component names\n");
        printf("      (e.g., Position, Health, Sprite, Velocity)\n");
        return;
    }
    
    if (strcmp(command, "EXIT") == 0 || strcmp(command, "exit") == 0) {
        return;  // Caller should handle exit
    }
    
    // Execute query
    QueryEngineResult result;
    QueryStatus status = Query_execute(shell->ecs, command, &result);
    
    if (status == QUERY_SUCCESS) {
        // Check if this is a SHOW query (has data but no entities)
        if (result.data != NULL) {
            // SHOW query result - component data is in result->data
            printf("Component data retrieved (size: %zu bytes)\n", result.count);
            // Note: Component data structure is type-specific, so we can't generically print it
            // In a real implementation, you'd want to format based on component type
            printf("  (Use SHOW queries programmatically to access typed data)\n");
        } else if (result.count > 0) {
            // SELECT or COUNT query result
            printf("Found %zu entities\n", result.count);
            // Get entities using the accessor function
            size_t entityCount = 0;
            EntityId* entities = (EntityId*)QueryEngineResult_get_entities(&result, &entityCount);
            if (entities) {
                for (size_t i = 0; i < entityCount && i < 10; i++) {
                    printf("  Entity: %llu:%llu\n", 
                           (unsigned long long)entities[i].high,
                           (unsigned long long)entities[i].low);
                }
                if (entityCount > 10) {
                    printf("  ... and %zu more\n", entityCount - 10);
                }
            }
        } else {
            printf("No entities found\n");
        }
        QueryEngineResult_free(&result);
    } else {
        const char* errorMsg = "Unknown error";
        switch (status) {
            case QUERY_ERROR_PARSE:
                errorMsg = "Parse error - check your query syntax";
                break;
            case QUERY_ERROR_EXECUTION:
                errorMsg = "Execution error - component/entity may not exist";
                break;
            case QUERY_ERROR_INVALID_SYNTAX:
                errorMsg = "Invalid syntax";
                break;
        }
        printf("Query error (%d): %s\n", status, errorMsg);
    }
}

void QueryShell_run(QueryShell* shell) {
    if (!shell) return;
    
    printf("Gramarye Query Shell\n");
    printf("Type 'HELP' for commands, 'EXIT' to quit\n");
    
    char* line = NULL;
    size_t lineSize = 0;
    
    while (1) {
        if (shell->prompt) {
            printf("%s", shell->prompt);
        } else {
            printf("query> ");
        }
        fflush(stdout);
        
        ssize_t read = getline(&line, &lineSize, stdin);
        if (read <= 0) {
            break;
        }
        
        // Remove newline
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        
        // Check for exit
        if (strcmp(line, "EXIT") == 0 || strcmp(line, "exit") == 0) {
            break;
        }
        
        QueryShell_process_command(shell, line);
    }
    
    if (line) {
        free(line);
    }
}

