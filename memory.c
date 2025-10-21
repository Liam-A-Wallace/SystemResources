#include "monitor.h"
#include <stdio.h>
#include <string.h>

double get_memory_usage() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        perror("sysinfo");
        return 0.0;
    }
    return (double)(info.totalram - info.freeram) / info.totalram;
}

void get_detailed_memory_stats(MemStats *mem) {
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
        
        if (found_total && found_free && found_cached && found_buffers) {
            break;
        }
    }
    fclose(file);
    
    if (found_total && found_free && found_cached && found_buffers) {
        mem->used = mem->total - mem->free - mem->cached - mem->buffers;
    } else {
        mem->used = 0;
    }
}

void print_detailed_memory_stats() {
    MemStats mem = {0};
    get_detailed_memory_stats(&mem);
    
    if (mem.total > 0) {
        printf("Memory Breakdown:\n");
        
        double used_percent = (double)mem.used / mem.total;
        double free_percent = (double)mem.free / mem.total;
        double cached_percent = (double)mem.cached / mem.total;
        double buffers_percent = (double)mem.buffers / mem.total;
        
        double total_gb = mem.total / (1024.0 * 1024.0);
        double used_gb = mem.used / (1024.0 * 1024.0);
        double free_gb = mem.free / (1024.0 * 1024.0);
        double cached_gb = mem.cached / (1024.0 * 1024.0);
        double buffers_gb = mem.buffers / (1024.0 * 1024.0);
        
        printf("  Used:   "); print_progress_bar(used_percent);
        printf(" (%.1f/%.1f GB)\n", used_gb, total_gb);
        printf("  Free:   "); print_progress_bar(free_percent);
        printf(" (%.1f GB)\n", free_gb);
        printf("  Cached: "); print_progress_bar(cached_percent);
        printf(" (%.1f GB)\n", cached_gb);
        printf("  Buffers:"); print_progress_bar(buffers_percent);
        printf(" (%.1f GB)\n", buffers_gb);
    }
}