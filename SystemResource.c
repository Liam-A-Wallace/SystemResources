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
    int simple_mode; // Simple mode flag shows only main usage bars
    int show_cpu_details; // Show CPU stats with or without Detailed breakdown
    int show_memory_details; // Show Memory stats with or without Detailed breakdown
    int show_disk_details; // Show Disk usage
} DisplayFlags;

//Global display flags
DisplayFlags display_flags = {1,1,1,1}; // Default to simple mode with all details

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


void handle_input(){
    if (input_available()){
        char c = getchar();
        switch (c)
        {
        case 's':
        case 'S':
            /* code */
            break;
        
        case 'q':
        case 'Q':
            printf("Exiting...\n");
            exit(0);
            break;
        }
    }
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
    while (1) {
        system("clear");  // Clear the screen and move the cursor to the top
        printf("--- System Performance Monitor ---\n");
        // CPU usage
        printf("CPU Usage: ");
        double cpu_usage = get_cpu_usage();
        print_progress_bar(cpu_usage);
        //CPU breakdown
        print_detailed_cpu_stats();
        printf("\n");

        // Memory usage
        printf("Memory Usage: ");
        double memory_usage = get_memory_usage();
        print_progress_bar(memory_usage);
        //Memory breakdown
        print_detailed_memory_stats();
        printf("\n");

        // Disk usage
        printf("Disk Usage: ");
        double disk_usage = get_disk_usage("/");
        print_progress_bar(disk_usage);
        sleep(1);  // Refresh every second 
    }
    return 0;
}