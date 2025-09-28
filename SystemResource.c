#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <string.h>

typedef struct {
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
} CPUStats;

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

void print_detailed_stats(){
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

    
// Function to calculate memory usage
double get_memory_usage() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        perror("sysinfo");
        return 0.0;
    }
    return (double)(info.totalram - info.freeram) / info.totalram;
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
    while (1) {
        printf("\033[H\033[J");  // Clear the screen and move the cursor to the top
        printf("--- System Performance Monitor ---\n");
        // CPU usage
        printf("CPU Usage: ");
        double cpu_usage = get_cpu_usage();
        print_progress_bar(cpu_usage);
        //CPU breakdown
        print_detailed_stats();

        // Memory usage
        printf("Memory Usage: ");
        double memory_usage = get_memory_usage();
        print_progress_bar(memory_usage);
        // Disk usage
        printf("Disk Usage: ");
        double disk_usage = get_disk_usage("/");
        print_progress_bar(disk_usage);
        sleep(1);  // Refresh every second 
    }
    return 0;
}