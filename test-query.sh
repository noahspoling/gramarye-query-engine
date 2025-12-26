#!/bin/bash
# test-query.sh - Build and run query engine tests
#
# Usage:
#   ./test-query.sh           # Build and run all tests
#   ./test-query.sh --clean   # Clean build artifacts and exit
#   ./test-query.sh --clean --build  # Clean, then build and run tests

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Clean function - removes all build artifacts
clean_build_artifacts() {
    echo "Cleaning build artifacts..."
    
    # Remove build directory
    if [ -d "build" ]; then
        rm -rf build
        echo "  Removed build/ directory"
    fi
    
    # Remove bin directory
    if [ -d "bin" ]; then
        rm -rf bin
        echo "  Removed bin/ directory"
    fi
    
    # Remove CMake artifacts in root
    if [ -d "CMakeFiles" ]; then
        rm -rf CMakeFiles
        echo "  Removed CMakeFiles/ directory"
    fi
    
    if [ -f "CMakeCache.txt" ]; then
        rm -f CMakeCache.txt
        echo "  Removed CMakeCache.txt"
    fi
    
    if [ -f "cmake_install.cmake" ]; then
        rm -f cmake_install.cmake
        echo "  Removed cmake_install.cmake"
    fi
    
    if [ -f "Makefile" ]; then
        rm -f Makefile
        echo "  Removed Makefile"
    fi
    
    # Remove CMake FetchContent dependencies
    if [ -d "_deps" ]; then
        rm -rf _deps
        echo "  Removed _deps/ directory"
    fi
    
    # Remove built libraries and executables in root
    if [ -f "libgramarye-query-engine.a" ]; then
        rm -f libgramarye-query-engine.a
        echo "  Removed libgramarye-query-engine.a"
    fi
    
    if [ -f "query_tests" ]; then
        rm -f query_tests
        echo "  Removed query_tests executable"
    fi
    
    echo "Clean complete!"
}

# Parse arguments
CLEAN_ONLY=false
BUILD_AND_RUN=true

for arg in "$@"; do
    case "$arg" in
        --clean)
            CLEAN_ONLY=true
            ;;
        --build)
            BUILD_AND_RUN=true
            ;;
        -h|--help)
            echo "Usage: $0 [--clean] [--build]"
            echo ""
            echo "Options:"
            echo "  --clean    Clean all build artifacts and exit"
            echo "  --build    Build and run tests (default if --clean not specified)"
            echo "  -h, --help Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0              # Build and run all tests"
            echo "  $0 --clean     # Clean build artifacts and exit"
            echo "  $0 --clean --build  # Clean, then build and run tests"
            exit 0
            ;;
        *)
            echo "Unknown option: $arg"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Clean if requested
if [ "$CLEAN_ONLY" = true ]; then
    clean_build_artifacts
    if [ "$BUILD_AND_RUN" = false ]; then
        exit 0
    fi
    echo ""
fi

# Build and run tests
if [ "$BUILD_AND_RUN" = true ]; then
    echo "Building tests..."
    
    # Create build directory (temporary, will be cleaned up)
    if [ ! -d "build" ]; then
        mkdir build
    fi
    
    cd build
    
    # Configure CMake to output executables to bin/
    cmake -DBUILD_TESTS=ON -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=../bin ..
    make
    
    # Create bin directory if it doesn't exist
    if [ ! -d "../bin" ]; then
        mkdir -p ../bin
    fi
    
    # Copy executable to bin (if it's not already there)
    if [ -f "./query_tests" ] && [ ! -f "../bin/query_tests" ]; then
        cp ./query_tests ../bin/query_tests
    fi
    
    # Clean up build directory and _deps
    echo ""
    echo "Cleaning up build artifacts..."
    cd ..
    rm -rf build
    if [ -d "_deps" ]; then
        rm -rf _deps
    fi
    
    # Run all tests
    echo ""
    echo "Running tests..."
    if [ -f "./bin/query_tests" ]; then
        ./bin/query_tests
    else
        echo "Error: query_tests executable not found in bin/!"
        echo "Build may have failed. Check the output above."
        exit 1
    fi
fi

