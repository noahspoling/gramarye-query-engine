cmake_minimum_required(VERSION 3.27)
project(gramarye-query-engine C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# WebAssembly toggle
option(BUILD_WEB "Build with Emscripten (WebAssembly)" OFF)

if(EMSCRIPTEN OR BUILD_WEB)
    set(PLATFORM Web CACHE STRING "" FORCE)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Option to build as shared or static (default static)
option(BUILD_SHARED_LIBS "Build shared library" OFF)

# Option to build standalone query shell executable
option(BUILD_QUERY_SHELL "Build standalone query shell executable" OFF)

# Fetch dependencies
include(FetchContent)

FetchContent_Declare(
    gramarye_libcore
    GIT_REPOSITORY "https://github.com/noahspoling/gramarye-libcore.git"
    GIT_TAG "main"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(gramarye_libcore)

FetchContent_Declare(
    gramarye_ecs
    GIT_REPOSITORY "https://github.com/noahspoling/gramarye-ecs.git"
    GIT_TAG "main"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(gramarye_ecs)

# Source files
file(GLOB SRC_FILES
    "src/*.c"
)

message(STATUS "SRC_FILES: ${SRC_FILES}")

# Create library
add_library(gramarye-query-engine 
    ${SRC_FILES}
)

# Add include directory
target_include_directories(gramarye-query-engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(gramarye-query-engine PUBLIC
    gramarye-ecs
    gramarye-libcore
)

# Export for CMake
include(GNUInstallDirs)
install(TARGETS gramarye-query-engine
    EXPORT queryTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT queryTargets
    FILE queryTargets.cmake
    NAMESPACE gramarye-query-engine::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/query
)

# For FetchContent compatibility
if(NOT TARGET gramarye-query-engine::gramarye-query-engine)
    add_library(gramarye-query-engine::gramarye-query-engine ALIAS gramarye-query-engine)
endif()

# Option to build tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    file(GLOB TEST_SRC_FILES "tests/*.c")
    list(FILTER TEST_SRC_FILES EXCLUDE REGEX ".*test_runner\\.c$")
    add_executable(query_tests tests/test_runner.c ${TEST_SRC_FILES})
    target_link_libraries(query_tests PRIVATE
        gramarye-query-engine
        gramarye-ecs
        gramarye-libcore
    )
    target_include_directories(query_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
endif()

# Optional standalone query shell executable
if(BUILD_QUERY_SHELL)
    add_executable(query_shell
        examples/shell_main.c
    )
    
    target_link_libraries(query_shell PRIVATE
        gramarye-query-engine
        gramarye-ecs
        gramarye-libcore
    )
    
    target_include_directories(query_shell PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
endif()

# Test shell with mock data (always build if tests are enabled)
if(BUILD_TESTS)
    add_executable(test_shell
        test_shell.c
    )
    
    target_link_libraries(test_shell PRIVATE
        gramarye-query-engine
        gramarye-ecs
        gramarye-libcore
    )
    
    target_include_directories(test_shell PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
endif()

