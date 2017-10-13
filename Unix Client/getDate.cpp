//
// Created by 李欣 on 2017/10/13.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *host;
    int socketFD;
    int len, result;
    struct sockaddr_in address;
    struct hostent *hostInfo;
    struct servent *serviceInfo;
    char buffer[128];

    if (argc == 1) {
        host = const_cast<char *>("localhost");
    } else {
        host = argv[1];
    }

    hostInfo = gethostbyname(host);
    if (!hostInfo) {
        fprintf(stderr, "no host: %s\n", host);
        exit(1);
    }

    serviceInfo = getservbyname("daytime", "tcp");
    if (!serviceInfo) {
        fprintf(stderr, "no daytime service\n");
        exit(1);
    }
    printf("daytime port is %d\n", ntohs(serviceInfo->s_port));

    socketFD = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_port = htons(serviceInfo->s_port);
    address.sin_addr = *(struct in_addr *) *hostInfo->h_addr_list;
    len = sizeof(address);

    result = connect(socketFD, (struct sockaddr *) &address, static_cast<socklen_t>(len));
    if (result == -1) {
        perror("oops: getDate");
        exit(1);
    }

    result = static_cast<int>(read(socketFD, buffer, sizeof(buffer)));
    buffer[result] = '\0';
    printf("read %d bytes: %s", result, buffer);

    close(socketFD);
    exit(0);
}