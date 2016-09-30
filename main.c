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

#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

int isValidNDevice(char *name) {
    
    char data[4096];
    memset(data, 0, 4096);
    struct ifconf ifc;
    struct ifreq *ifr;
    int sk, length;
    
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if(sk < 0)
    {
        perror("socket");
        return 0;
    }
    
    ifc.ifc_len = sizeof(data);
    ifc.ifc_buf = (caddr_t)data;
    if(ioctl(sk, SIOCGIFCONF, &ifc) < 0)
    {
        perror("ioctl(SIOCGIFCONF)");
        return 0;
    }
    
    ifr = (struct ifreq*)data;
    for(int i=0;i<ifc.ifc_len;)
    {
        length=IFNAMSIZ + ifr->ifr_addr.sa_len;
        printf("%s\n", ifr->ifr_name);
        if (!strcmp(ifr->ifr_name,name)) {
            printf("Interface Found!\n");
            return 1;
        }
        ifr=(struct ifreq*)((char*)ifr+length);
        i+=length;
    }
    
    return 0;
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
    
    if (!isValidNDevice(argv[1])) {
        printf("\nPlease choose a valid network device.\nFor example: en0 or p2p0\n");
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
        exit(0);
    }
    
    printf("\nDevice: %s", argv[1]);
    printf("\nMin Refresh: %.01fmin", (float)minimum/60.0f);
    printf("\nMax Refresh: %.01fmin\n", (float)maximum/60.0f);
    
    sleep(3);
    while (1) {
        char hwaddr[1000];
        memset(hwaddr, 0, strlen(hwaddr));
        sprintf(hwaddr, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x", rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16, rand()%16);
        pid = fork();
        if (pid == 0) {
            char *execArgs[] = {"sudo", "ifconfig", argv[1], "ether", hwaddr, NULL};
            printf("\nSetting new MAC address: %s on device: %s\n", hwaddr, argv[1]);
            execvp(execArgs[0], execArgs);
        } else {
            waitpid(-1, &status, 0);
            sleep(10);
            pid = fork();
            if (pid == 0) {
                char *execArgs[] = {"sudo", "ifconfig", argv[1], "down", NULL};
                execvp(execArgs[0], execArgs);
            } else {
                waitpid(-1, &status, 0);
                sleep(10);
                pid = fork();
                if (pid == 0) {
                    char *execArgs[] = {"sudo", "ifconfig", argv[1], "up", NULL};
                    execvp(execArgs[0], execArgs);
                } else {
                    waitpid(-1, &status, 0);
                    printf("Completed\n");
                    int r = rand()%(maximum+1-minimum);
                    sleep(minimum+r);
                }
            }
        }
    }
    return EXIT_SUCCESS;
}
