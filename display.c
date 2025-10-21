#include "monitor.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

// Function to print progress bar with color based on usage
void print_progress_bar(double usage) {
    int width = 50;
    int pos = usage * width;
    const char *color;
    
    if (usage < 0.5) {
        color = "\033[32m";  // Green
    } else if (usage < 0.75) {
        color = "\033[33m";  // Yellow
    } else {
        color = "\033[31m";  // Red
    }
    
    printf("%s[", color);
    for (int i = 0; i < width; ++i) {
        if (i < pos) printf("#");
        else printf(" ");
    }
    printf("]\033[0m %.2f%%\n", usage * 100);
}

int input_available() {
    struct timeval tv = {0, 0};
    fd_set fds;
    
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

void handle_input() {
    if (input_available()) {
        char c = getchar();
        switch (c) {
            case 's':
            case 'S':
                display_flags.simple_mode = !display_flags.simple_mode;
                if (display_flags.simple_mode) {
                    display_flags.show_cpu_details = 0;
                    display_flags.show_memory_details = 0;
                } else {
                    display_flags.show_cpu_details = 1;
                    display_flags.show_memory_details = 1;
                }
                break;
            case 'c':
            case 'C':
                if (!display_flags.simple_mode) {
                    display_flags.show_cpu_details = !display_flags.show_cpu_details;
                }
                break;
            case 'm':
            case 'M':
                if (!display_flags.simple_mode) {
                    display_flags.show_memory_details = !display_flags.show_memory_details;
                }
                break;
            case 'd':
            case 'D':
                display_flags.show_disk_details = !display_flags.show_disk_details;
                break;
            case 'n':
            case 'N':
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