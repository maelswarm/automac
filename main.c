//
//  main.c
//  autoMACtic
//
//  Created by fairy-slipper on 1/24/16.
//  Copyright © 2016 fairy-slipper. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>

pid_t  pid;
int status;
int minimum;
int maximum;


void safe_printf(const char *format, ...)
{
    char buf[1000];
    va_list args;
    
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    write(1, buf, strlen(buf)); /* write is async-signal-safe */
}


int main(int argc, char **argv) {
    maximum = 3600;
    minimum = 1800;
    srand(time(NULL));
    
    uid_t euid=geteuid();
    if (0!=euid) {
        printf("\nPlease run as root\n");
        exit(0);
    }
    
    if (argc<2) {
        printf("\nPlease specify a network device\n");
        printf("./autoMACtic <device>\n");
        printf("\n\nYou can also choose refresh bounds.");
        printf("\n./autoMACtic <device> <minumum timeout in minutes> <maximum timeout in minutes>\n\n");
        exit(0);
    }
    if (argv[2] != NULL) {
        minimum = atoi(argv[2])*60;
    }
    if (argv[3] != NULL) {
        maximum = atoi(argv[3])*60;
    }
    if (maximum<minimum) {
        printf("\nmaximum must be eault to or greater than minimum\n");
        exit(0);
    }
    if (maximum<60 || minimum<60) {
        printf("\ntimeout must be equal or greater than 1 minute\n");
    }
    
    printf("\nDevice: %s", argv[1]);
    printf("\nMin Refresh: %.01fmin", (float)minimum/60.0f);
    printf("\nMax Refresh: %.01fmin\n", (float)maximum/60.0f);
    
    while (1) {
        char hwaddr[1000];
        memset(hwaddr, 0, strlen(hwaddr));
        sprintf(hwaddr, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x", rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16);
        pid = fork();
        if (pid == 0) {
            
            char *execArgs[] = {"sudo", "ifconfig", argv[1], "ether", hwaddr, NULL};
            printf("\nSetting new MAC address: %s on device: %s\n", hwaddr, argv[1]);
            int ifconfig_refresh = execvp(execArgs[0], execArgs);
        } else {
            waitpid(-1, &status, 0);
            sleep(10);
            pid = fork();
            if (pid == 0) {
                char *execArgs[] = {"sudo", "ifconfig", argv[1], "down", NULL};
                int ifconfig_down = execvp(execArgs[0], execArgs);
            } else {
                waitpid(-1, &status, 0);
                sleep(10);
                pid = fork();
                if (pid == 0) {
                    char *execArgs[] = {"sudo", "ifconfig", argv[1], "up", NULL};
                    int ifconfig_up = execvp(execArgs[0], execArgs);
                } else {
                    waitpid(-1, &status, 0);
                    printf("\nCompleted\n");
                    int r = rand()%(maximum+1-minimum);
                    sleep(minimum+r);
                }
            }
        }
    }
    return EXIT_SUCCESS;
}
