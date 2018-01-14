//
// Created by 李欣 on 2017/10/15.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int server_socketFD, client_socketFD;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int result;
    fd_set readFDs, testFDs;

    server_socketFD = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(5005);
    server_len = sizeof(server_address);

    bind(server_socketFD, (struct sockaddr *) &server_address, static_cast<socklen_t>(server_len));

    listen(server_socketFD, 5);

    FD_ZERO(&readFDs);
    FD_SET(server_socketFD, &readFDs);

    while (true) {
        char ch;
        int fd;
        int nRead;

        testFDs = readFDs;
        printf("server waiting\n");
        result = select(FD_SETSIZE, &testFDs, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0);

        if (result < 1) {
            perror("server5");
            exit(1);
        }

        for (fd = 0; fd < FD_SETSIZE; fd++) {
            if (FD_ISSET(fd, &testFDs)) {
                if (fd == server_socketFD) {
                    client_len = sizeof(client_address);
                    client_socketFD = accept(server_socketFD, (struct sockaddr *) &client_address,
                                             (socklen_t *) &client_len);
                    FD_SET(client_socketFD, &readFDs);
                    printf("adding cient on fd %d\n", client_socketFD);
                } else {
                    ioctl(fd, FIONREAD, &nRead);

                    if (nRead == 0) {
                        close(fd);
                        FD_CLR(fd, &readFDs);
                        printf("removing client on fd %d\n", fd);
                    } else {
                        read(fd, &ch, 1);
                        // sleep(5);
                        printf("serving client on fd %d\n", fd);
                        ch++;
                        write(fd, &ch, 1);
                    }
                }
            }
        }
    }
}