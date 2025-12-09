# MIevM Unit Tests

This directory contains unit tests for the MIevM CAN bridge firmware.

## Prerequisites

- CMake 3.14 or higher
- GCC compiler (native, not ARM cross-compiler)
- CppUTest (included as local source in `/cpputest`)

## Building and Running Tests

### Quick Start

Use the test script from the project root:

```bash
./test.sh
```

Or with options:

```bash
./test.sh -c          # Clean build
./test.sh -v          # Verbose test output
./test.sh --coverage  # Build with coverage
```

### Using Make

From the project root:

```bash
make test            # Build and run tests
make test-clean      # Clean test build directory
```

### Manual Build

```bash
cd test
mkdir -p build
cd build
cmake ..
make
./MIevM_Tests
```

## Test Structure

- `test_main.cpp` - Test runner entry point
- `test_can_bridge.cpp` - Unit tests for CAN bridge functions

## Writing New Tests

1. Create a new test file (e.g., `test_new_feature.cpp`)
2. Add it to `CMakeLists.txt` in the `add_executable()` section
3. Include CppUTest headers: `#include "CppUTest/TestHarness.h"`
4. Write your tests using CppUTest macros

Example:

```cpp
TEST_GROUP(MyFeature)
{
    void setup() {
        // Setup before each test
    }
    
    void teardown() {
        // Cleanup after each test
    }
};

TEST(MyFeature, BasicTest)
{
    CHECK_EQUAL(1, 1);
}
```

## Test Coverage

To generate coverage reports:

```bash
./test.sh --coverage
```

Coverage report will be in `test/build/coverage_report/index.html`

### Coverage Notes

The coverage report shows code coverage **only for your project source files**, excluding:
- Test files (`test_*.cpp`, `test_main.cpp`)
- CppUTest library code
- System headers

Currently, the example tests use `test_functions.c` as a demonstration. To get coverage of your actual firmware:

1. **Add your source files to the test build** in `test/CMakeLists.txt`:
   ```cmake
   add_executable(MIevM_Tests
       test_main.cpp
       test_can_bridge.cpp
       test_functions.c
       # Add your firmware source files here:
       # ../Src/can-bridge-firmware.c
       # ../Src/can.c
   )
   ```

2. **Mock hardware dependencies** (GPIO, CAN peripherals, etc.) or use conditional compilation to replace hardware calls with test stubs.

3. **Write tests** that exercise your actual firmware functions.

### Interpreting Coverage Results

- **Line coverage**: Percentage of code lines executed during tests
- **Function coverage**: Percentage of functions called during tests
- **Branch coverage**: Percentage of conditional branches tested (if available)

Aim for high coverage (>80%) on critical business logic, though 100% coverage doesn't guarantee bug-free code.

## CppUTest Documentation

For more information on writing tests with CppUTest, see:
- https://cpputest.github.io/
