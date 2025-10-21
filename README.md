# System Resource Monitor

A real-time system monitoring tool written in C that displays CPU, memory, disk, and network usage with an interactive terminal interface.

## Features
- **Real-time Monitoring**: Live updates of system resources every second
- **Interactive Controls**: Toggle between simple and advanced modes
- **Color-coded Display**: Progress bars change color based on usage levels
- **Multiple Resource Types**: CPU, memory, disk, and network monitoring
- **Detailed Breakdowns**: Advanced mode shows detailed component statistics

## Quick Start
1. Compile the project: `make`
2. Run the monitor: `./SystemResources`
3. Use keyboard controls to toggle displays hitting enter after input:
   - `S` - Switch between simple/advanced modes
   - `C` - Toggle CPU details (advanced mode)
   - `M` - Toggle memory details (advanced mode)
   - `D` - Toggle disk usage display
   - `N` - Toggle network statistics
   - `Q` - Quit the application

## Modules
- `main.c` - Program controller and main loop
- `monitor.h` - Common headers and struct definitions
- `display.c` - Progress bars, input handling, and UI controls
- `cpu.c` - CPU usage calculation and detailed breakdown
- `memory.c` - Memory statistics and detailed analysis
- `disk.c` - Disk usage monitoring
- `network.c` - Network interface statistics and rate calculation

## Monitoring Includes
- **CPU Usage**: Overall usage and detailed breakdown (user, system, idle, IOWait, IRQ)
- **Memory Statistics**: Total, used, free, cached, and buffers with GB measurements
- **Disk Usage**: Root partition usage percentage
- **Network Statistics**: Real-time upload/download rates per interface in KB/s

Run the monitor and use the interactive controls to customize which system resources are displayed in real-time.