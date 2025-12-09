#!/bin/bash

# Build script for MIevM STM32 project using CMake

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Debug"
CLEAN_BUILD=0
BUILD_DIR="build"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=1
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -r, --release    Build in Release mode (default: Debug)"
            echo "  -c, --clean      Clean build directory before building"
            echo "  -h, --help       Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Check if arm-none-eabi-g++ is installed
if ! command -v arm-none-eabi-g++ &> /dev/null; then
    echo -e "${RED}Error: arm-none-eabi-g++ not found!${NC}"
    echo "Please install the ARM GNU Toolchain:"
    echo "  Ubuntu/Debian: sudo apt-get install gcc-arm-none-eabi"
    echo "  macOS: brew install arm-none-eabi-gcc"
    echo "  Or download from: https://developer.arm.com/downloads/-/gnu-rm"
    exit 1
fi

echo -e "${GREEN}Found ARM toolchain:${NC}"
arm-none-eabi-g++ --version | head -n 1

# Clean build directory if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf $BUILD_DIR
fi

# Create build directory
mkdir -p $BUILD_DIR

# Run CMake
echo -e "${GREEN}Configuring CMake (${BUILD_TYPE})...${NC}"
cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# Build
echo -e "${GREEN}Building project...${NC}"
cmake --build . -- -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Check if build was successful
if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}================================${NC}"
    echo -e "${GREEN}Build completed successfully!${NC}"
    echo -e "${GREEN}================================${NC}"
    echo ""
    echo "Output files:"
    ls -lh *.elf *.hex *.bin *.srec 2>/dev/null || true
    echo ""
    echo "Build artifacts are in: $BUILD_DIR/"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi
