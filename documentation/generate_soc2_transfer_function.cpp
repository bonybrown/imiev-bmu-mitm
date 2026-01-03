/**
 * @file generate_transfer_function.cpp
 * @brief Generate CSV data for BatteryModel::voltageToSoC2() transfer function
 * 
 * This program samples the voltageToSoC2() function across the full voltage
 * range and outputs CSV data that can be plotted.
 * 
 * Compile with:
 *   g++ -std=c++11 -I../Inc -o generate_transfer_function \
 *       generate_transfer_function.cpp \
 *       ../Src/BatteryModel.cpp \
 *       ../Src/VoltageByte.cpp
 * 
 * Run with:
 *   ./generate_transfer_function > soc2_data.csv
 */

#include <iostream>
#include <iomanip>
#include "BatteryModel.h"
#include "VoltageByte.h"

int main() {
    // Output CSV header
    std::cout << "voltage,soc2" << std::endl;
    
    // Sample voltage from 2.0V to 4.5V in 0.01V steps
    for (float voltage = 2.0f; voltage <= 4.5f; voltage += 0.01f) {
        VoltageByte vb = VoltageByte::fromVoltage(voltage);
        float soc2 = BatteryModel::voltageToSoC2(vb);
        
        // Output with high precision
        std::cout << std::fixed << std::setprecision(3) << voltage << ","
                  << std::setprecision(2) << soc2 << std::endl;
    }
    
    return 0;
}
