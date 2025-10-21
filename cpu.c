#include "monitor.h"
#include <stdio.h>

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
    if (prev_total > 0) {
        unsigned long total_diff = curr_total - prev_total;
        unsigned long idle_diff = curr_stats.idle - prev_stats.idle;
        unsigned long used_diff = total_diff - idle_diff;
        usage = (double)used_diff / total_diff;
    }
    
    prev_stats = curr_stats;
    return usage;
}

void print_detailed_cpu_stats() {
    CPUStats curr = {0};
    FILE *file = fopen("/proc/stat", "r");
    
    if (!file) {
        perror("fopen");
        return;
    }
    
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
    
    if (total > 0) {
        printf("CPU Breakdown:\n");
        printf("  User:   "); print_progress_bar((double)curr.user / total);
        printf("  Nice:   "); print_progress_bar((double)curr.nice / total);
        printf("  System: "); print_progress_bar((double)curr.system / total);
        printf("  Idle:   "); print_progress_bar((double)curr.idle / total);
        printf("  IOWait: "); print_progress_bar((double)curr.iowait / total);
        printf("  IRQ:    "); print_progress_bar((double)curr.irq / total);
    }
}