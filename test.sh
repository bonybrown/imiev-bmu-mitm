#!/bin/bash

# Test build and run script for MIevM unit tests

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
CLEAN_BUILD=0
VERBOSE=0
COVERAGE=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--clean)
            CLEAN_BUILD=1
            shift
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        --coverage)
            COVERAGE=1
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -c, --clean      Clean build directory before building"
            echo "  -v, --verbose    Run tests with verbose output"
            echo "  --coverage       Build with coverage instrumentation"
            echo "  -h, --help       Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Check if CppUTest source exists in the project
CPPUTEST_DIR="$(dirname "$0")/cpputest"
if [ ! -d "$CPPUTEST_DIR" ]; then
    echo -e "${RED}Error: CppUTest source directory not found!${NC}"
    echo "Expected location: $CPPUTEST_DIR"
    echo "Please clone CppUTest:"
    echo "  cd $(dirname "$0")"
    echo "  git clone --depth 1 --branch v4.0 https://github.com/cpputest/cpputest.git"
    exit 1
fi

echo -e "${GREEN}Found CppUTest source in project${NC}"

# Navigate to test directory
cd "$(dirname "$0")/test"

TEST_BUILD_DIR="build"

# Clean build directory if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    echo -e "${YELLOW}Cleaning test build directory...${NC}"
    rm -rf $TEST_BUILD_DIR
fi

# Create build directory
mkdir -p $TEST_BUILD_DIR
cd $TEST_BUILD_DIR

# Configure CMake
echo -e "${GREEN}Configuring tests...${NC}"
if [ $COVERAGE -eq 1 ]; then
    echo -e "${BLUE}Enabling coverage instrumentation${NC}"
    cmake -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_CXX_FLAGS="--coverage" \
          -DCMAKE_C_FLAGS="--coverage" \
          ..
else
    cmake -DCMAKE_BUILD_TYPE=Debug ..
fi

# Build tests
echo -e "${GREEN}Building tests...${NC}"
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Check if build was successful
if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}Running tests...${NC}"
echo -e "${GREEN}================================${NC}"
echo ""

# Run tests
if [ $VERBOSE -eq 1 ]; then
    ./MIevM_Tests -v -c
else
    ./MIevM_Tests
fi

TEST_RESULT=$?

echo ""
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}================================${NC}"
    echo -e "${GREEN}All tests passed!${NC}"
    echo -e "${GREEN}================================${NC}"
    
    if [ $COVERAGE -eq 1 ]; then
        echo ""
        echo -e "${BLUE}Generating coverage report...${NC}"
        
        if command -v lcov &> /dev/null; then
            echo -e "${BLUE}Capturing coverage data...${NC}"
            lcov --capture --directory . --output-file coverage.info 2>/dev/null
            
            echo -e "${BLUE}Filtering coverage data...${NC}"
            # Remove system headers
            lcov --remove coverage.info '/usr/*' --output-file coverage.info 2>/dev/null
            # Remove CppUTest library code
            lcov --remove coverage.info '*/cpputest/*' --output-file coverage.info 2>/dev/null
            lcov --remove coverage.info '*/cpputest-build/*' --output-file coverage.info 2>/dev/null
            # Remove build directory artifacts
            lcov --remove coverage.info '*/build/*' --output-file coverage.info 2>/dev/null
            # Remove test runner and test case files, but keep source files being tested
            lcov --remove coverage.info '*/test_main.cpp' --output-file coverage.info 2>/dev/null
            lcov --remove coverage.info '*/test_*.cpp' --output-file coverage.info 2>/dev/null
            
            if command -v genhtml &> /dev/null; then
                echo -e "${BLUE}Generating HTML report...${NC}"
                genhtml coverage.info --output-directory coverage_report 2>/dev/null
                echo -e "${GREEN}Coverage report generated in: test/build/coverage_report/index.html${NC}"
                echo ""
                echo -e "${BLUE}Coverage summary:${NC}"
                lcov --list coverage.info
            else
                echo -e "${YELLOW}genhtml not found. Install with: sudo apt-get install lcov${NC}"
            fi
        else
            echo -e "${YELLOW}lcov not found. Install with: sudo apt-get install lcov${NC}"
        fi
    fi
else
    echo -e "${RED}================================${NC}"
    echo -e "${RED}Some tests failed!${NC}"
    echo -e "${RED}================================${NC}"
    exit 1
fi
