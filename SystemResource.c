#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <string.h>
#include <sys/select.h>

//CPU breakdown struct
typedef struct {
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
} CPUStats;

//Memory breakdown struct
typedef struct {
    unsigned long total;
    unsigned long free;
    unsigned long used;
    unsigned long cached;
    unsigned long buffers;
} MemStats;

typedef struct {
    char interface[16];
    unsigned long long rx_bytes;      // Received bytes
    unsigned long long tx_bytes;      // Transmitted bytes
    unsigned long long rx_bytes_prev; // Previous reading for rate calculation
    unsigned long long tx_bytes_prev; // Previous reading for rate calculation
    double rx_rate;              // Receive rate in KB/s
    double tx_rate;              // Transmit rate in KB/s
} NetworkStats;

typedef struct {
    int simple_mode; // Simple mode flag shows only main usage bars
    int show_cpu_details; // Show CPU stats with or without Detailed breakdown
    int show_memory_details; // Show Memory stats with or without Detailed breakdown
    int show_disk_details; // Show Disk usage
    int show_network; // Show network stats
} DisplayFlags;

//Global display flags
DisplayFlags display_flags = {1,1,1,1,1}; // Default to simple mode with all details

// Function to print progress bar with color based on usage
void print_progress_bar(double usage) {
    int width = 50;  // Width of the progress bar
    int pos = usage * width;
    // Determine color: Green (low usage), Yellow (medium), Red (high)
    const char *color;
    if (usage < 0.5) {
        color = "\033[32m";  // Green
    } else if (usage < 0.75) {
        color = "\033[33m";  // Yellow
    } else {
        color = "\033[31m";  // Red
    }
    printf("%s[", color);  // Set color
    for (int i = 0; i < width; ++i) {
        if (i < pos) {
            printf("#");
        } else {
            printf(" ");
        }
    }
    printf("]\033[0m %.2f%%\n", usage * 100);  // Reset color
}

int input_available(){
    struct timeval tv = {0, 0}; // timeout of 0 seconds
    fd_set fds; // set of file descriptors to check

    FD_ZERO(&fds); // clear the set
    FD_SET(STDIN_FILENO, &fds); // add stdin (file descriptor 0) to the set

    // Check if input is available
    return select(1, &fds, NULL, NULL, &tv) > 0;
}


// Handle keyboard input
void handle_input() {
    if (input_available()) {
        char c = getchar();
        switch (c) {
            case 's':
            case 'S':
                // Toggle simple/advanced mode
                display_flags.simple_mode = !display_flags.simple_mode;
                if (display_flags.simple_mode) {
                    // In simple mode, hide all details
                    display_flags.show_cpu_details = 0;
                    display_flags.show_memory_details = 0;
                } else {
                    // In advanced mode, show all details
                    display_flags.show_cpu_details = 1;
                    display_flags.show_memory_details = 1;
                }
                break;
            case 'c':
            case 'C':
                // Toggle CPU details (only in advanced mode)
                if (!display_flags.simple_mode) {
                    display_flags.show_cpu_details = !display_flags.show_cpu_details;
                }
                break;
            case 'm':
            case 'M':
                // Toggle memory details (only in advanced mode)
                if (!display_flags.simple_mode) {
                    display_flags.show_memory_details = !display_flags.show_memory_details;
                }
                break;
            case 'd':
            case 'D':
                // Toggle disk display
                display_flags.show_disk_details = !display_flags.show_disk_details;
                break;
            case 'n':
            case 'N':
                // Toggle network display
                display_flags.show_network = !display_flags.show_network;
                break;
            case 'q':
            case 'Q':
                printf("\nExiting system monitor...\n");
                exit(0);
                break;
        }
    }
}

void print_controls() {
    printf("\nControls: ");
    printf("S(mode:%s) ", display_flags.simple_mode ? "SIMPLE" : "ADVANCED");
    if (!display_flags.simple_mode) {
        printf("C[%s] ", display_flags.show_cpu_details ? "ON" : "OFF");
        printf("M[%s] ", display_flags.show_memory_details ? "ON" : "OFF");
    }
    printf("D[%s] ", display_flags.show_disk_details ? "ON" : "OFF");
    printf("N[%s] ", display_flags.show_network ? "ON" : "OFF");
    printf("Q(quit)\n");
}

// Function to calculate CPU usage
double get_cpu_usage() {

    static CPUStats prev_stats = {0};
    CPUStats curr_stats = {0};

    FILE *file = fopen("/proc/stat", "r");
    if (!file) {
        perror("fopen");
        return 0.0;
    }
    char buffer[256];

    if (fgets(buffer, sizeof(buffer), file)) {
        sscanf(buffer, "cpu %lu %lu %lu %lu %lu %lu %lu %lu",
               &curr_stats.user, &curr_stats.nice, &curr_stats.system,
               &curr_stats.idle, &curr_stats.iowait, &curr_stats.irq,
               &curr_stats.softirq, &curr_stats.steal);
    }
    fclose(file);

    unsigned long prev_total = prev_stats.user + prev_stats.nice + prev_stats.system +
                                       prev_stats.idle + prev_stats.iowait +
                                       prev_stats.irq + prev_stats.softirq + prev_stats.steal;
    unsigned long curr_total = curr_stats.user + curr_stats.nice + curr_stats.system +
                                       curr_stats.idle + curr_stats.iowait +
                                       curr_stats.irq + curr_stats.softirq + curr_stats.steal;

    double usage = 0.0;
    if (prev_total >0){
        unsigned long total_diff = curr_total - prev_total;
        unsigned long idle_diff = curr_stats.idle - prev_stats.idle;
        unsigned long used_diff = total_diff - idle_diff;

        usage = (double)used_diff / total_diff;
    }
    prev_stats = curr_stats;
    return usage;

}

void print_detailed_cpu_stats(){
    CPUStats curr = {0};

    FILE *file = fopen("/proc/stat", "r");
    if (!file) {
        perror("fopen");
        return;}
    
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), file)) {
        sscanf(buffer, "cpu %lu %lu %lu %lu %lu %lu %lu %lu",
               &curr.user, &curr.nice, &curr.system,
               &curr.idle, &curr.iowait, &curr.irq,
               &curr.softirq, &curr.steal);
    }
    fclose(file);
    unsigned long total = curr.user + curr.nice + curr.system +
                          curr.idle + curr.iowait +
                          curr.irq + curr.softirq + curr.steal;
    if (total>0){
        printf("CPU Breakdown:\n");
        // Calculate percentages
        double user_percent = (double)curr.user / total;
        double nice_percent = (double)curr.nice / total;
        double system_percent = (double)curr.system / total;
        double idle_percent = (double)curr.idle / total;
        double iowait_percent = (double)curr.iowait / total;
        double irq_percent = (double)curr.irq / total;
        
        // Print with progress bars - using shorter labels for alignment
        printf("  User:   ");
        print_progress_bar(user_percent);
        
        printf("  Nice:   ");
        print_progress_bar(nice_percent);
        
        printf("  System: ");
        print_progress_bar(system_percent);
        
        printf("  Idle:   ");
        print_progress_bar(idle_percent);
        
        printf("  IOWait: ");
        print_progress_bar(iowait_percent);
        
        printf("  IRQ:    ");
        print_progress_bar(irq_percent);
    }
}

// Network monitoring functions
void get_network_stats(NetworkStats *stats, int *count) {
    FILE *file = fopen("/proc/net/dev", "r");
    if (!file) {
        perror("fopen");
        return;
    }
    
    char line[256];
    int i = 0;
    // Skip first two lines (header)
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file) && i < *count) {
        char interface[16];
        unsigned long long rx_bytes, tx_bytes;
        
        // Parse the network statistics line
        // Format: interface: rx_bytes rx_packets rx_errors rx_drop ... tx_bytes tx_packets ...
        if (sscanf(line, " %15[^:]: %llu %*u %*u %*u %*u %*u %*u %*u %llu",
                   interface, &rx_bytes, &tx_bytes) >= 2) {
            // Remove trailing colon from interface name
            char *colon = strchr(interface, ':');
            if (colon) *colon = '\0';
            
            // Skip loopback interface
            if (strcmp(interface, "lo") != 0) {
                strcpy(stats[i].interface, interface);
                stats[i].rx_bytes = rx_bytes;
                stats[i].tx_bytes = tx_bytes;
                i++;
            }
        }
    }
    *count = i;
    fclose(file);
}

void calculate_network_rates(NetworkStats *stats, int count) {
    static int first_run = 1;
    if (first_run) {
        // First run, just store current values
        for (int i = 0; i < count; i++) {
            stats[i].rx_bytes_prev = stats[i].rx_bytes;
            stats[i].tx_bytes_prev = stats[i].tx_bytes;
            stats[i].rx_rate = 0.0;
            stats[i].tx_rate = 0.0;
        }
        first_run = 0;
        return;
    }
    
    // Calculate rates based on difference from previous reading
    for (int i = 0; i < count; i++) {
        unsigned long rx_diff = stats[i].rx_bytes - stats[i].rx_bytes_prev;
        unsigned long tx_diff = stats[i].tx_bytes - stats[i].tx_bytes_prev;

        // Handle counter reset or wrap-around
        if (rx_diff < 0) rx_diff = 0;
        if (tx_diff < 0) tx_diff = 0;
        
        // Convert to KB/s (assuming 200ms refresh rate)
        stats[i].rx_rate = rx_diff / 1024.0 / 0.2;  // KB per second
        stats[i].tx_rate = tx_diff / 1024.0 / 0.2;  // KB per second
        
        // Store current values for next calculation
        stats[i].rx_bytes_prev = stats[i].rx_bytes;
        stats[i].tx_bytes_prev = stats[i].tx_bytes;
    }
}

void print_network_stats() {
    static NetworkStats stats[8];  // Support up to 8 interfaces
    int count = 8;
    
    get_network_stats(stats, &count);
    calculate_network_rates(stats, count);

    if (count > 0) {
        printf("Network:\n");

        double total_rx = 0.0, total_tx = 0.0;
        for (int i = 0; i < count; i++) {
            total_rx += stats[i].rx_rate;
            total_tx += stats[i].tx_rate;

            // Display only active or recently used interfaces
            if (stats[i].rx_rate > 0.1 || stats[i].tx_rate > 0.1) {
                printf("  %s:\n", stats[i].interface);
                printf("    RX: %10.2f KB/s | TX: %10.2f KB/s\n",
                       stats[i].rx_rate, stats[i].tx_rate);
            }
        }

        printf("  Total: RX: %10.2f KB/s | TX: %10.2f KB/s\n", total_rx, total_tx);
    }
}
    
// Function to calculate memory usage
double get_memory_usage() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        perror("sysinfo");
        return 0.0;
    }
    return (double)(info.totalram - info.freeram) / info.totalram;
}

void get_detailed_memory_stats(MemStats *mem){
    FILE *file = fopen("/proc/meminfo", "r");
    if (!file) {
        perror("fopen");
        return;
    }
    
    char line[256];
    int found_total = 0, found_free = 0, found_cached = 0, found_buffers = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line + 9, "%lu", &mem->total);
            found_total = 1;
        } else if (strncmp(line, "MemFree:", 8) == 0) {
            sscanf(line + 8, "%lu", &mem->free);
            found_free = 1;
        } else if (strncmp(line, "Cached:", 7) == 0) {
            sscanf(line + 7, "%lu", &mem->cached);
            found_cached = 1;
        } else if (strncmp(line, "Buffers:", 8) == 0) {
            sscanf(line + 8, "%lu", &mem->buffers);
            found_buffers = 1;
        }
        
        // If we found all values, we can break early
        if (found_total && found_free && found_cached && found_buffers) {
            break;
        }
    }
    fclose(file);
    
    // Only calculate used if we have all required values
    if (found_total && found_free && found_cached && found_buffers) {
        mem->used = mem->total - mem->free - mem->cached - mem->buffers;
    } else {
        // Set to 0 if we couldn't read all values
        mem->used = 0;
    }
}

void print_detailed_memory_stats(){
    MemStats mem = {0};
    get_detailed_memory_stats(&mem);
    if (mem.total>0){
        printf("Memory Breakdown:\n");
        // Calculate percentages
        double used_percent = (double)mem.used / mem.total;
        double free_percent = (double)mem.free / mem.total;
        double cached_percent = (double)mem.cached / mem.total;
        double buffers_percent = (double)mem.buffers / mem.total;
        
        double total_gb = mem.total / (1024.0 * 1024.0);
        double used_gb = mem.used / (1024.0 * 1024.0);
        double free_gb = mem.free / (1024.0 * 1024.0);
        double cached_gb = mem.cached / (1024.0 * 1024.0);
        double buffers_gb = mem.buffers / (1024.0 * 1024.0);
        
        // Print with progress bars
        printf("  Used:   ");
        print_progress_bar(used_percent);
        printf(" (%.1f/%.1f GB)\n", used_gb, total_gb);
        
        printf("  Free:   ");
        print_progress_bar(free_percent);
        printf(" (%.1f GB)\n", free_gb);
        
        printf("  Cached: ");
        print_progress_bar(cached_percent);
        printf(" (%.1f GB)\n", cached_gb);
        
        printf("  Buffers:");
        print_progress_bar(buffers_percent);
        printf(" (%.1f GB)\n", buffers_gb);
    }
}
// Function to calculate disk usage for a given path
double get_disk_usage(const char *path) {
    struct statvfs buf;
    if (statvfs(path, &buf) != 0) {
        perror("statvfs");
        return 0.0;
    }
    unsigned long total = buf.f_blocks * buf.f_frsize;
    unsigned long used = total - (buf.f_bfree * buf.f_frsize);
    return (double)used / total;
}
int main() {
    printf("Starting System Monitor...\n");
    printf("Controls: S(toggle simple/advanced) C(toggle CPU) M(toggle Memory) D(toggle Disk) N(toggle Network) Q(quit)\n");
    sleep(2);  // Show instructions briefly

    while (1) {
        system("clear");
        
        // Show current mode in header
        if (display_flags.simple_mode) {
            printf("--- System Monitor (SIMPLE MODE) ---\n");
        } else {
            printf("--- System Monitor (ADVANCED MODE) ---\n");
        }
        
        // Check for user input
        handle_input();
        
        // Always show CPU usage
        printf("CPU Usage: ");
        double cpu_usage = get_cpu_usage();
        print_progress_bar(cpu_usage);
        
        // Show CPU details only in advanced mode and if enabled
        if (!display_flags.simple_mode && display_flags.show_cpu_details) {
            print_detailed_cpu_stats();
        }
        
        // Always show memory usage  
        printf("\nMemory Usage: ");
        double memory_usage = get_memory_usage();
        print_progress_bar(memory_usage);
        
        // Show memory details only in advanced mode and if enabled
        if (!display_flags.simple_mode && display_flags.show_memory_details) {
            print_detailed_memory_stats();
        }
        
        // Show disk only if enabled
        if (display_flags.show_disk_details) {
            printf("\nDisk Usage: ");
            double disk_usage = get_disk_usage("/");
            print_progress_bar(disk_usage);
        }

        if (display_flags.show_network) {
            printf("\n");
            print_network_stats();
        }
        
        // Show controls
        print_controls();
        
        usleep(1000000);  // refresh every 1 second
    }
    return 0;
}