# Documentation

This directory contains tools for generating documentation and visualizations for the MIevM project.

## Transfer Function Visualization

The `generate_soc2_plot.sh` script generates a visual representation of the `BatteryModel::voltageToSoC2()` transfer function.

### Requirements

- g++ compiler
- gnuplot

Install gnuplot:
```bash
# Ubuntu/Debian
sudo apt-get install gnuplot

# macOS
brew install gnuplot
```

**Note:** The C++ data generator will work without gnuplot and produce `soc2_data.csv`. 
Gnuplot is only needed for the final plot generation.

### Usage

Simply run the shell script:
```bash
cd documentation
./generate_soc2_plot.sh
```

This will:
1. Compile `generate_soc2_transfer_function.cpp` 
2. Execute it to generate voltage/SoC data points
3. Use gnuplot to create a plot
4. Save the image as `soc2_transfer_function.png` in this `documentation` directory

