//#include <sys/types.h>
//#include <sys/socket.h>
//#include <sys/un.h>
//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <unistd.h>
//
//int main(int argc, char *argv[]) {
//    int server_sockFD, client_sockFD;
//    int server_len, client_len;
//    struct sockaddr_un server_address{};
//    struct sockaddr_un client_address{};
//    unlink("server_socket");
//    server_sockFD = socket(AF_UNIX, SOCK_STREAM, 0);
//    server_address.sun_family = AF_UNIX;
//    strcpy(server_address.sun_path, "sever_socket");
//    server_len = sizeof(server_address);
//    bind(server_sockFD, (struct sockaddr *) &server_address, static_cast<socklen_t>(server_len));
//    listen(server_sockFD, 5);
//    while (true) {
//        char ch;
//        printf("server waiting\n");
//        client_len = sizeof(client_address);
//        client_sockFD = accept(server_sockFD, (struct sockaddr *) &client_address, (socklen_t *) &client_len);
//        read(client_sockFD, &ch, 1);
//        write(client_sockFD, &ch, 1);
//        close(client_sockFD);
//    }
//    return 0;
//}

/*  Make the necessary includes and set up the variables.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>

int main() {
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_un server_address;
    struct sockaddr_un client_address;

/*  Remove any old socket and create an unnamed socket for the server.  */

    unlink("server_socket");
    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

/*  Name the socket.  */

    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, "server_socket");
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *) &server_address, server_len);

/*  Create a connection queue and wait for clients.  */

    listen(server_sockfd, 5);
    while (1) {
        char ch;

        printf("server waiting\n");

/*  Accept a connection.  */

        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_address, (socklen_t *) &client_len);

/*  We can now read/write to client on client_sockfd.  */

        read(client_sockfd, &ch, 1);
        ch++;
        write(client_sockfd, &ch, 1);
        close(client_sockfd);
    }
}



