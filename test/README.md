# MIevM Unit Tests

This directory contains unit tests for the MIevM CAN bridge firmware.

## Prerequisites

- CMake 3.14 or higher
- GCC compiler (native, not ARM cross-compiler)
- CppUTest (included as local source in `/cpputest`)
- `lcov` for generating test coverage report with the `--coverage` option.

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

## Test Structure

- `test_main.cpp` - Test runner entry point
- `test_[module_name].cpp` - Unit tests for `[module_name]`

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

Because `main.cpp` is mainly concerned with hardware initialisation
and passing CAN frames between the hardware and the `App` instance,
there are no tests for that code. It is kept intentionally simple - _Change with care!_

## CppUTest Documentation

For more information on writing tests with CppUTest, see:
- https://cpputest.github.io/
