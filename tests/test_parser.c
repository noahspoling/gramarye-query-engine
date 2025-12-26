#include "test_common.h"
#include "gramarye_query/parser.h"
#include "except.h"
#include <string.h>

static void test_parser_basic_select(void) {
    printf("  Testing basic SELECT query parsing...\n");
    
    QueryParser* parser = QueryParser_new("SELECT entities WHERE has(Position)");
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");
    
    QueryAST* ast = QueryParser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "AST should be created");
    TEST_ASSERT_EQ(QueryAST_get_type(ast), AST_SELECT, "Query type should be SELECT");
    
    QueryAST* predicate = QueryAST_get_left(ast);
    TEST_ASSERT_NOT_NULL(predicate, "Predicate should exist");
    TEST_ASSERT_EQ(QueryAST_get_type(predicate), AST_HAS, "Predicate type should be HAS");
    
    ComponentList* list = (ComponentList*)QueryAST_get_data(predicate);
    TEST_ASSERT_NOT_NULL(list, "Component list should exist");
    TEST_ASSERT_EQ(list->count, 1, "Should have one component");
    TEST_ASSERT(strcmp(list->componentNames[0], "Position") == 0, "Component name should be Position");
    
    QueryAST_destroy(ast);
    QueryParser_destroy(parser);
}

static void test_parser_select_multiple_components(void) {
    printf("  Testing SELECT with multiple components...\n");
    
    QueryParser* parser = QueryParser_new("SELECT entities WHERE has(Position, Health, Sprite)");
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");
    
    QueryAST* ast = QueryParser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "AST should be created");
    
    QueryAST* predicate = QueryAST_get_left(ast);
    TEST_ASSERT_NOT_NULL(predicate, "Predicate should exist");
    
    ComponentList* list = (ComponentList*)QueryAST_get_data(predicate);
    TEST_ASSERT_NOT_NULL(list, "Component list should exist");
    TEST_ASSERT_EQ(list->count, 3, "Should have three components");
    TEST_ASSERT(strcmp(list->componentNames[0], "Position") == 0, "First component should be Position");
    TEST_ASSERT(strcmp(list->componentNames[1], "Health") == 0, "Second component should be Health");
    TEST_ASSERT(strcmp(list->componentNames[2], "Sprite") == 0, "Third component should be Sprite");
    
    QueryAST_destroy(ast);
    QueryParser_destroy(parser);
}

static void test_parser_count_query(void) {
    printf("  Testing COUNT query parsing...\n");
    
    QueryParser* parser = QueryParser_new("COUNT entities WHERE has(Sprite)");
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");
    
    QueryAST* ast = QueryParser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "AST should be created");
    TEST_ASSERT_EQ(QueryAST_get_type(ast), AST_COUNT, "Query type should be COUNT");
    
    QueryAST_destroy(ast);
    QueryParser_destroy(parser);
}

static void test_parser_has_any(void) {
    printf("  Testing has_any predicate...\n");
    
    QueryParser* parser = QueryParser_new("SELECT entities WHERE has_any(Position, Sprite)");
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");
    
    QueryAST* ast = QueryParser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "AST should be created");
    
    QueryAST* predicate = QueryAST_get_left(ast);
    TEST_ASSERT_NOT_NULL(predicate, "Predicate should exist");
    TEST_ASSERT_EQ(QueryAST_get_type(predicate), AST_HAS_ANY, "Predicate type should be HAS_ANY");
    
    QueryAST_destroy(ast);
    QueryParser_destroy(parser);
}

static void test_parser_not_has(void) {
    printf("  Testing not_has predicate...\n");
    
    QueryParser* parser = QueryParser_new("SELECT entities WHERE not_has(Health)");
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");
    
    QueryAST* ast = QueryParser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "AST should be created");
    
    QueryAST* predicate = QueryAST_get_left(ast);
    TEST_ASSERT_NOT_NULL(predicate, "Predicate should exist");
    TEST_ASSERT_EQ(QueryAST_get_type(predicate), AST_NOT_HAS, "Predicate type should be NOT_HAS");
    
    QueryAST_destroy(ast);
    QueryParser_destroy(parser);
}

static void test_parser_show_component(void) {
    printf("  Testing SHOW component query...\n");
    
    QueryParser* parser = QueryParser_new("SHOW Position OF entity 1:2");
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");
    
    QueryAST* ast = QueryParser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "AST should be created");
    TEST_ASSERT_EQ(QueryAST_get_type(ast), AST_SHOW, "Query type should be SHOW");
    
    ShowQueryData* showData = (ShowQueryData*)QueryAST_get_data(ast);
    TEST_ASSERT_NOT_NULL(showData, "Show data should exist");
    TEST_ASSERT_NOT_NULL(showData->componentName, "Component name should exist");
    TEST_ASSERT(strcmp(showData->componentName, "Position") == 0, "Component name should be Position");
    TEST_ASSERT_NOT_NULL(showData->entityId, "Entity ID should exist");
    TEST_ASSERT_EQ(showData->entityId->high, 1ULL, "Entity ID high should be 1");
    TEST_ASSERT_EQ(showData->entityId->low, 2ULL, "Entity ID low should be 2");
    
    QueryAST_destroy(ast);
    QueryParser_destroy(parser);
}

static void test_parser_show_all(void) {
    printf("  Testing SHOW ALL query...\n");
    
    QueryParser* parser = QueryParser_new("SHOW ALL OF entity 123:456");
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");
    
    QueryAST* ast = QueryParser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "AST should be created");
    TEST_ASSERT_EQ(QueryAST_get_type(ast), AST_SHOW, "Query type should be SHOW");
    
    ShowQueryData* showData = (ShowQueryData*)QueryAST_get_data(ast);
    TEST_ASSERT_NOT_NULL(showData, "Show data should exist");
    TEST_ASSERT_NULL(showData->componentName, "Component name should be NULL for ALL");
    TEST_ASSERT_NOT_NULL(showData->entityId, "Entity ID should exist");
    TEST_ASSERT_EQ(showData->entityId->high, 123ULL, "Entity ID high should be 123");
    TEST_ASSERT_EQ(showData->entityId->low, 456ULL, "Entity ID low should be 456");
    
    QueryAST_destroy(ast);
    QueryParser_destroy(parser);
}

static void test_parser_invalid_syntax(void) {
    printf("  Testing invalid syntax handling...\n");
    
    // Missing WHERE
    QueryParser* parser = QueryParser_new("SELECT entities has(Position)");
    QueryAST* ast = QueryParser_parse(parser);
    TEST_ASSERT_NULL(ast, "Should fail to parse invalid syntax");
    QueryParser_destroy(parser);
    
    // Missing entities
    parser = QueryParser_new("SELECT WHERE has(Position)");
    ast = QueryParser_parse(parser);
    TEST_ASSERT_NULL(ast, "Should fail to parse missing entities");
    QueryParser_destroy(parser);
    
    // Missing component name
    parser = QueryParser_new("SELECT entities WHERE has()");
    ast = QueryParser_parse(parser);
    TEST_ASSERT_NULL(ast, "Should fail to parse empty component list");
    QueryParser_destroy(parser);
    
    // Invalid entity ID format
    parser = QueryParser_new("SHOW Position OF entity abc");
    ast = QueryParser_parse(parser);
    TEST_ASSERT_NULL(ast, "Should fail to parse invalid entity ID");
    QueryParser_destroy(parser);
}

static void test_parser_whitespace_handling(void) {
    printf("  Testing whitespace handling...\n");
    
    // Extra whitespace
    QueryParser* parser = QueryParser_new("  SELECT   entities   WHERE   has(  Position  )  ");
    QueryAST* ast = QueryParser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "Should handle extra whitespace");
    if (ast) {
        QueryAST_destroy(ast);
    }
    QueryParser_destroy(parser);
}

bool test_parser(void) {
    printf("Running parser tests...\n");
    
    TRY
        test_parser_basic_select();
        test_parser_select_multiple_components();
        test_parser_count_query();
        test_parser_has_any();
        test_parser_not_has();
        test_parser_show_component();
        test_parser_show_all();
        test_parser_invalid_syntax();
        test_parser_whitespace_handling();
        
        printf("  ✓ All parser tests passed\n");
        return true;
    EXCEPT(Test_Failed)
        printf("  ✗ Parser test failed\n");
        return false;
    END_TRY;
}

