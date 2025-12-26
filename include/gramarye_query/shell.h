#ifndef GRAMARYE_QUERY_SHELL_H
#define GRAMARYE_QUERY_SHELL_H

#include "gramarye_ecs/ecs.h"

// Forward declaration
typedef struct QueryShell QueryShell;

// Create a new query shell
QueryShell* QueryShell_new(ECS* ecs);

// Destroy query shell
void QueryShell_destroy(QueryShell* shell);

// Run interactive shell (blocks until exit)
void QueryShell_run(QueryShell* shell);

// Process a single command (non-interactive)
void QueryShell_process_command(QueryShell* shell, const char* command);

// Set prompt string
void QueryShell_set_prompt(QueryShell* shell, const char* prompt);

// Enable/disable command history
void QueryShell_set_history_enabled(QueryShell* shell, bool enabled);

#endif // GRAMARYE_QUERY_SHELL_H

