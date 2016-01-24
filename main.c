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

pid_t  pid;
int status;
int minimum;
int maximum;

int main(int argc, char **argv) {
    maximum = 1800;
    minimum = 300;
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
    
    while (1) {
        char hwaddr[1000];
        memset(hwaddr, 0, strlen(hwaddr));
        sprintf(hwaddr, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x", rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16);
        pid = fork();
        if (pid == 0) {
            char *execArgs[] = {"sudo", "ifconfig", argv[1], "ether", hwaddr, NULL};
            printf("\n");
            for (int i=0; i<5; i++) {
                printf("%s ",execArgs[i]);
            }
            printf("\n");
            int ifconfig_spawn = execvp(execArgs[0], execArgs);
        } else {
            waitpid(-1, &status, 0);
            int r = rand()%(maximum+1-minimum);
            sleep(minimum+r);
        }
    }
    return EXIT_SUCCESS;
}
