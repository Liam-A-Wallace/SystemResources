#ifndef MONITOR_H
#define MONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <string.h>
#include <sys/select.h>

// CPU breakdown struct
typedef struct {
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
} CPUStats;

// Memory breakdown struct
typedef struct {
    unsigned long total;
    unsigned long free;
    unsigned long used;
    unsigned long cached;
    unsigned long buffers;
} MemStats;

typedef struct {
    char interface[16];
    unsigned long long rx_bytes;
    unsigned long long tx_bytes;
    unsigned long long rx_bytes_prev;
    unsigned long long tx_bytes_prev;
    double rx_rate;
    double tx_rate;
} NetworkStats;

typedef struct {
    int simple_mode;
    int show_cpu_details;
    int show_memory_details;
    int show_disk_details;
    int show_network;
} DisplayFlags;

// Global display flags
extern DisplayFlags display_flags;

// Display functions
void print_progress_bar(double usage);
int input_available();
void handle_input();
void print_controls();

// CPU functions
double get_cpu_usage();
void print_detailed_cpu_stats();

// Memory functions
double get_memory_usage();
void get_detailed_memory_stats(MemStats *mem);
void print_detailed_memory_stats();

// Disk functions
double get_disk_usage(const char *path);

// Network functions
void get_network_stats(NetworkStats *stats, int *count);
void calculate_network_rates(NetworkStats *stats, int count);
void print_network_stats();

#endif