#!/bin/bash
# Generate the SoC2 transfer function plot using gnuplot

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"

pushd "$SCRIPT_DIR" > /dev/null

echo "Compiling SoC2 data generator..."
g++ -std=c++11 -I"$ROOT_DIR/Inc" -o "$SCRIPT_DIR/generate_soc2_transfer_function" \
    "$SCRIPT_DIR/generate_soc2_transfer_function.cpp" \
    "$ROOT_DIR/Src/BatteryModel.cpp" \
    "$ROOT_DIR/Src/VoltageByte.cpp"

echo "Generating data..."
"$SCRIPT_DIR/generate_soc2_transfer_function" > "$SCRIPT_DIR/soc2_data.csv"

echo "Creating plot with gnuplot..."
gnuplot "$SCRIPT_DIR/plot_soc2_transfer_function.gnuplot"

echo "Plot saved to: $SCRIPT_DIR/soc2_transfer_function.png"
echo "Done!"

popd > /dev/null