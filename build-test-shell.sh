#!/bin/bash
# build-test-shell.sh - Build and run the test shell with mock data
#
# Usage:
#   ./build-test-shell.sh           # Build and run test shell
#   ./build-test-shell.sh --build   # Just build, don't run
#   ./build-test-shell.sh --clean   # Clean build artifacts

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BUILD_ONLY=false
CLEAN_ONLY=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build)
            BUILD_ONLY=true
            shift
            ;;
        --clean)
            CLEAN_ONLY=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [--build] [--clean] [--help]"
            echo ""
            echo "Options:"
            echo "  --build    Build only, don't run"
            echo "  --clean    Clean build artifacts"
            echo "  --help     Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Clean function
clean_build_artifacts() {
    echo "Cleaning build artifacts..."
    
    if [ -d "build" ]; then
        rm -rf build
        echo "  Removed build/ directory"
    fi
    
    if [ -d "bin" ]; then
        rm -rf bin
        echo "  Removed bin/ directory"
    fi
    
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
    
    if [ -d "_deps" ]; then
        rm -rf _deps
        echo "  Removed _deps/ directory"
    fi
    
    echo "Clean complete!"
}

if [ "$CLEAN_ONLY" = true ]; then
    clean_build_artifacts
    exit 0
fi

# Create build directory (temporary, will be cleaned up)
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure CMake
echo "Configuring CMake..."
cmake .. -DBUILD_TESTS=ON -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=../bin

# Build
echo ""
echo "Building test shell..."
cmake --build . --target test_shell

# Create bin directory if it doesn't exist
if [ ! -d "../bin" ]; then
    mkdir -p ../bin
fi

# Copy executable to bin (if it's not already there)
if [ -f "./test_shell" ] && [ ! -f "../bin/test_shell" ]; then
    cp ./test_shell ../bin/test_shell
fi

# Clean up build directory and _deps
echo ""
echo "Cleaning up build artifacts..."
cd ..
rm -rf build
if [ -d "_deps" ]; then
    rm -rf _deps
fi

if [ "$BUILD_ONLY" = true ]; then
    echo ""
    echo "Build complete! Executable is in bin/test_shell"
    echo "Run with:"
    echo "  ./bin/test_shell"
    exit 0
fi

# Run the test shell
echo ""
echo "=========================================="
echo "Running test shell..."
echo "=========================================="
echo ""

if [ -f "./bin/test_shell" ]; then
    ./bin/test_shell
else
    echo "Error: test_shell executable not found in bin/!"
    echo "Build may have failed. Check the output above."
    exit 1
fi

