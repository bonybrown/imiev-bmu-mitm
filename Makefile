# Makefile for MIevM Project
# Provides convenient targets for building firmware and tests

.PHONY: all firmware tests clean clean-all help

# Default target
all: firmware

# Build firmware for STM32
firmware:
	@echo "Building firmware for STM32F105xC..."
	@./build.sh --release

# Build firmware in debug mode
firmware-debug:
	@echo "Building firmware in debug mode..."
	@./build.sh

# Build and run unit tests
tests:
	@echo "Building and running unit tests..."
	@./test.sh

# Build tests with verbose output
tests-verbose:
	@echo "Building and running tests with verbose output..."
	@./test.sh --verbose

# Build tests with coverage
tests-coverage:
	@echo "Building tests with coverage..."
	@./test.sh --coverage

# Clean firmware build
clean:
	@echo "Cleaning firmware build..."
	@rm -rf build

# Clean test build
clean-tests:
	@echo "Cleaning test build..."
	@rm -rf test/build

# Clean everything
clean-all: clean clean-tests
	@echo "Cleaned all build artifacts"

# Flash firmware using st-flash
flash: firmware
	@echo "Flashing firmware to STM32..."
	@st-flash write build/MIevM.bin 0x8000000

# Show help
help:
	@echo "MIevM Makefile Targets:"
	@echo ""
	@echo "  make                  - Build firmware (release mode)"
	@echo "  make firmware         - Build firmware (release mode)"
	@echo "  make firmware-debug   - Build firmware (debug mode)"
	@echo "  make tests            - Build and run unit tests"
	@echo "  make tests-verbose    - Run tests with verbose output"
	@echo "  make tests-coverage   - Run tests with coverage report"
	@echo "  make clean            - Clean firmware build"
	@echo "  make clean-tests      - Clean test build"
	@echo "  make clean-all        - Clean all builds"
	@echo "  make flash            - Flash firmware to STM32"
	@echo "  make help             - Show this help message"
	@echo ""
	@echo "For more details, see CMAKE_BUILD.md and test/README.md"
