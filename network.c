#include "monitor.h"
#include <stdio.h>
#include <string.h>

void get_network_stats(NetworkStats *stats, int *count) {
    FILE *file = fopen("/proc/net/dev", "r");
    if (!file) {
        perror("fopen");
        return;
    }
    
    char line[256];
    int i = 0;
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file) && i < *count) {
        char interface[16];
        unsigned long long rx_bytes, tx_bytes;
        
        if (sscanf(line, " %15[^:]: %llu %*u %*u %*u %*u %*u %*u %*u %llu",
                   interface, &rx_bytes, &tx_bytes) >= 2) {
            char *colon = strchr(interface, ':');
            if (colon) *colon = '\0';
            
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
        for (int i = 0; i < count; i++) {
            stats[i].rx_bytes_prev = stats[i].rx_bytes;
            stats[i].tx_bytes_prev = stats[i].tx_bytes;
            stats[i].rx_rate = 0.0;
            stats[i].tx_rate = 0.0;
        }
        first_run = 0;
        return;
    }
    
    for (int i = 0; i < count; i++) {
        unsigned long rx_diff = stats[i].rx_bytes - stats[i].rx_bytes_prev;
        unsigned long tx_diff = stats[i].tx_bytes - stats[i].tx_bytes_prev;

        if (rx_diff < 0) rx_diff = 0;
        if (tx_diff < 0) tx_diff = 0;
        
        stats[i].rx_rate = rx_diff / 1024.0 / 0.2;
        stats[i].tx_rate = tx_diff / 1024.0 / 0.2;
        
        stats[i].rx_bytes_prev = stats[i].rx_bytes;
        stats[i].tx_bytes_prev = stats[i].tx_bytes;
    }
}

void print_network_stats() {
    static NetworkStats stats[8];
    int count = 8;
    
    get_network_stats(stats, &count);
    calculate_network_rates(stats, count);

    if (count > 0) {
        printf("Network:\n");
        double total_rx = 0.0, total_tx = 0.0;
        
        for (int i = 0; i < count; i++) {
            total_rx += stats[i].rx_rate;
            total_tx += stats[i].tx_rate;

            if (stats[i].rx_rate > 0.1 || stats[i].tx_rate > 0.1) {
                printf("  %s:\n", stats[i].interface);
                printf("    RX: %10.2f KB/s | TX: %10.2f KB/s\n",
                       stats[i].rx_rate, stats[i].tx_rate);
            }
        }
        printf("  Total: RX: %10.2f KB/s | TX: %10.2f KB/s\n", total_rx, total_tx);
    }
}