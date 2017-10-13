//
// Created by 李欣 on 2017/10/13.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int socketFD;
    int len;
    struct sockaddr_in address;
    int result;
    char data = 'A';

    socketFD = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(5005);
    len = sizeof(address);

    result = connect(socketFD, (struct sockaddr *) &address, static_cast<socklen_t>(len));

    if (result == -1) {
        perror("oops: client2");
        exit(1);
    }

    write(socketFD, &data, 1);
    read(socketFD, &data, 1);
    printf("char from server = %c\n", data);
    close(socketFD);
    exit(0);
}