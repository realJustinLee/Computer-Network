//
// Created by 李欣 on 2017/10/13.
//

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *host, **names, **address;
    struct hostent *hostInfo;

    if (argc == 1) {
        char myName[256];
        gethostname(myName, 255);
        host = myName;
    } else {
        host = argv[1];
    }

    hostInfo = gethostbyname(host);
    if (!hostInfo) {
        fprintf(stderr, "cannot get info for host: %s\n", host);
        exit(1);
    }

    printf("results for host %s:\n", host);
    printf("Name: %s\n", hostInfo->h_name);
    printf("Aliases:");
    names = hostInfo->h_aliases;
    while (*names) {
        printf(" %s", *names);
        names++;
    }
    printf("\n");

    if (hostInfo->h_addrtype != AF_INET) {
        fprintf(stderr, "not an IP host!\n");
        exit(1);
    }

    address = hostInfo->h_addr_list;
    while (*address) {
        printf(" %s", inet_ntoa(*(struct in_addr *) *address));
        address++;
    }
    printf("\n");
    exit(0);
}