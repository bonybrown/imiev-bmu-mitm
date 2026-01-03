# Gnuplot script to generate SoC2 transfer function plot

# Output settings
set terminal pngcairo size 1200,800 enhanced font 'Arial,10'
set output 'soc2_transfer_function.png'

# Title and labels
set title "BatteryModel::voltageToSoC2() Transfer Function\nPiecewise Linear Interpolation for Battery SoC Estimation" font 'Arial,14'
set xlabel "Cell Voltage (V)" font 'Arial,12'
set ylabel "State of Charge (%)" font 'Arial,12'

# Grid
set grid xtics ytics mxtics mytics
set grid linewidth 0.5 linetype -1 linecolor rgb "#cccccc"
set grid mxtics mytics linewidth 0.3 linetype -1 linecolor rgb "#e0e0e0"

# Axis ranges
set xrange [2.5:4.3]
set yrange [-5:105]

# Minor tics
set mxtics 5
set mytics 5

# Data format
set datafile separator ","

# Key voltage markers (vertical lines)
set arrow from 2.75, graph 0 to 2.75, graph 1 nohead linetype 0 linewidth 0.5 linecolor rgb "#999999"
set arrow from 3.00, graph 0 to 3.00, graph 1 nohead linetype 0 linewidth 0.5 linecolor rgb "#999999"
set arrow from 3.47, graph 0 to 3.47, graph 1 nohead linetype 0 linewidth 0.5 linecolor rgb "#999999"
set arrow from 3.60, graph 0 to 3.60, graph 1 nohead linetype 0 linewidth 0.5 linecolor rgb "#999999"
set arrow from 3.72, graph 0 to 3.72, graph 1 nohead linetype 0 linewidth 0.5 linecolor rgb "#999999"
set arrow from 3.81, graph 0 to 3.81, graph 1 nohead linetype 0 linewidth 0.5 linecolor rgb "#999999"
set arrow from 3.92, graph 0 to 3.92, graph 1 nohead linetype 0 linewidth 0.5 linecolor rgb "#999999"
set arrow from 4.00, graph 0 to 4.00, graph 1 nohead linetype 0 linewidth 0.5 linecolor rgb "#999999"
set arrow from 4.20, graph 0 to 4.20, graph 1 nohead linetype 0 linewidth 0.5 linecolor rgb "#999999"

# Legend
set key bottom right

# Plot the data
plot 'soc2_data.csv' using 1:2 skip 1 with lines linewidth 2 linecolor rgb "#0000ff" title "SoC2 Transfer Function", \
     'soc2_data.csv' using 1:($1==2.75||$1==3.00||$1==3.47||$1==3.60||$1==3.72||$1==3.81||$1==3.92||$1==4.00||$1==4.20?$2:1/0) skip 1 with points pointtype 7 pointsize 1.5 linecolor rgb "#ff0000" notitle
