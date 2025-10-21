#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Initialize global display flags
DisplayFlags display_flags = {1,1,1,1,1};

int main() {
    printf("Starting System Monitor...\n");
    printf("Controls: S(toggle simple/advanced) C(toggle CPU) M(toggle Memory) D(toggle Disk) N(toggle Network) Q(quit)\n");
    sleep(2);

    while (1) {
        system("clear");
        
        if (display_flags.simple_mode) {
            printf("--- System Monitor (SIMPLE MODE) ---\n");
        } else {
            printf("--- System Monitor (ADVANCED MODE) ---\n");
        }
        
        handle_input();
        
        printf("CPU Usage: ");
        double cpu_usage = get_cpu_usage();
        print_progress_bar(cpu_usage);
        
        if (!display_flags.simple_mode && display_flags.show_cpu_details) {
            print_detailed_cpu_stats();
        }
        
        printf("\nMemory Usage: ");
        double memory_usage = get_memory_usage();
        print_progress_bar(memory_usage);
        
        if (!display_flags.simple_mode && display_flags.show_memory_details) {
            print_detailed_memory_stats();
        }
        
        if (display_flags.show_disk_details) {
            printf("\nDisk Usage: ");
            double disk_usage = get_disk_usage("/");
            print_progress_bar(disk_usage);
        }

        if (display_flags.show_network) {
            printf("\n");
            print_network_stats();
        }
        
        print_controls();
        usleep(1000000);
    }
    
    return 0;
}