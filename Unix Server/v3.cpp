//
// Created by 李欣 on 2017/10/14.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int server_socketFD, client_socketFD;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_socketFD = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(5005);
    server_len = sizeof(server_address);
    bind(server_socketFD, (struct sockaddr *) &server_address, static_cast<socklen_t>(server_len));

    listen(server_socketFD, 5);

    signal(SIGCHLD, SIG_IGN);

    while (true) {
        char ch;

        printf("server waiting\n");

        client_len = sizeof(client_address);
        client_socketFD = accept(server_socketFD, (struct sockaddr *) &client_address, (socklen_t *) &client_len);

        if (fork() == 0) {
            read(client_socketFD, &ch, 1);
            sleep(1);
            ch++;
            write(client_socketFD, &ch, 1);
            close(client_socketFD);
            exit(0);
        } else {
            close(client_socketFD);
        }
    }
}