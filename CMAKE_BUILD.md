# MIevM - STM32F105xC CAN Bridge Firmware

## CMake Build System

This project now includes a CMake build system that can compile the firmware using any installed ARM GCC toolchain.

### Prerequisites

You need to have the ARM GNU Toolchain installed:

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi cmake build-essential
```

#### Linux (Fedora/RHEL)
```bash
sudo dnf install arm-none-eabi-gcc-cs arm-none-eabi-gcc-cs-c++ cmake
```

#### macOS
```bash
brew install --cask gcc-arm-embedded
brew install cmake
```

#### Windows
Download and install:
- ARM GNU Toolchain: https://developer.arm.com/downloads/-/gnu-rm
- CMake: https://cmake.org/download/

Make sure `arm-none-eabi-gcc` is in your PATH.

### Building the Project

#### Using the build script (Linux/macOS)

The easiest way to build:

```bash
# Debug build
./build.sh

# Release build
./build.sh --release

# Clean build
./build.sh --clean
```

#### Using Make (convenience wrapper)

```bash
# Build firmware (release)
make

# Build firmware (debug)
make firmware-debug

# Build and run tests
make tests

# Flash to device
make flash

# Show all targets
make help
```

#### Manual CMake build

```bash
# Create build directory
mkdir build
cd build

# Configure (Debug)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Or configure (Release)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build .

# Or use make directly
make -j$(nproc)
```

### Build Output

After a successful build, you'll find these files in the `build/` directory:

- `MIevM.elf.elf` - ELF executable with debug symbols
- `MIevM.hex` - Intel HEX format (for most programmers)
- `MIevM.bin` - Raw binary format
- `MIevM.srec` - Motorola S-record format (like original canbridge.srec)
- `MIevM.map` - Memory map file
- `MIevM.lst` - Assembly listing with source code

## Unit Testing

The project includes CppUTest-based unit tests that run on your native platform (x86/x64) using native GCC.

### Prerequisites for Testing

**CppUTest is included!** The project includes CppUTest v4.0 source in the `cpputest/` directory. No additional installation is needed.

If you want to use a system-installed version instead, you can install it:

```bash
# Ubuntu/Debian
sudo apt-get install cpputest
```

### Running Tests

#### Using the test script (recommended)
```bash
# Run tests
./test.sh

# Run with verbose output
./test.sh --verbose

# Run with code coverage
./test.sh --coverage

# Clean build before running
./test.sh --clean
```

#### Using Make
```bash
# Run tests
make tests

# Run with verbose output
make tests-verbose

# Run with coverage
make tests-coverage

# Clean test builds
make clean-tests
```

#### Manual test build
```bash
cd test
mkdir -p build && cd build
cmake ..
make
./MIevM_Tests -v
```

### Test Structure

Tests are located in the `test/` directory:
- `test_main.cpp` - Test runner entry point
- `test_can_bridge.cpp` - Example tests for CAN bridge functions
- `CMakeLists.txt` - Test-specific CMake configuration

### Writing New Tests

Example test:

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

TEST(MyFeature, TestCase)
{
    // Your test code
    CHECK_EQUAL(expected, actual);
}
```

See `test/README.md` for detailed testing documentation.

### Flashing the Firmware

#### Using ST-Link
```bash
st-flash write build/MIevM.bin 0x8000000
```

#### Using OpenOCD
```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
    -c "program build/MIevM.elf verify reset exit"
```

#### Using J-Link
```bash
JLinkExe -device STM32F105RC -if SWD -speed 4000 \
    -CommanderScript flash.jlink
```

Create `flash.jlink`:
```
r
h
loadfile build/MIevM.hex
r
g
q
```

### Project Structure

```
MIevM/
├── CMakeLists.txt              # CMake configuration
├── STM32F105XC_FLASH.ld        # Linker script
├── startup_stm32f105xc.s       # Startup assembly
├── build.sh                    # Build helper script
├── Inc/                        # Header files
├── Src/                        # Source files
│   ├── main.c
│   ├── can.c
│   ├── can-bridge-firmware.c
│   └── ...
└── Drivers/                    # STM32 HAL drivers
    ├── CMSIS/
    └── STM32F1xx_HAL_Driver/
```

### Target MCU

- **Device**: STM32F105xC (Connectivity Line)
- **Core**: ARM Cortex-M3
- **Flash**: 256 KB
- **RAM**: 64 KB
- **Max Frequency**: 72 MHz

### Customization

Edit `CMakeLists.txt` to:
- Change optimization levels
- Add/remove source files
- Modify compiler flags
- Change linker script

### Troubleshooting

**Problem**: `arm-none-eabi-gcc not found`
- **Solution**: Install ARM GNU Toolchain and ensure it's in your PATH

**Problem**: Linker errors about missing functions
- **Solution**: Make sure all required HAL drivers are enabled in `stm32f1xx_hal_conf.h`

**Problem**: Code doesn't fit in flash
- **Solution**: Use Release build (`-DCMAKE_BUILD_TYPE=Release`) or optimize code

**Problem**: Hard fault at startup
- **Solution**: Check that the correct linker script is used and memory addresses match your MCU

### Build Types

- **Debug**: `-Og -g3` - Optimized for debugging
- **Release**: `-O2 -g0` - Optimized for size and speed

### Memory Usage

After building, the size tool output shows:
```
   text    data     bss     dec     hex filename
  xxxxx    yyyy    zzzz   total   total MIevM.elf
```

- `text`: Code in flash
- `data`: Initialized data in flash (copied to RAM)
- `bss`: Uninitialized data in RAM
- `dec/hex`: Total memory footprint

### IDE Integration

The CMake project can be imported into:
- **CLion**: Open the project folder
- **VS Code**: Install CMake Tools extension
- **Eclipse**: Import as CMake project
- **Qt Creator**: Open CMakeLists.txt

### Original STM32CubeIDE Project

This CMake setup is compatible with the original STM32CubeIDE project. You can continue using STM32CubeIDE for development and use CMake for automated builds or CI/CD.

The `.mxproject` file is preserved for STM32CubeMX code generation.

## License

See LICENSE file for details.
