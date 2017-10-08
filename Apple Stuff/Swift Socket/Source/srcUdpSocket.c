//
//  srcUdpSocket.c
//  Swift Socket
//
//  Created by 李欣 on 2017/10/8.
//  Copyright © 2017年 李欣. All rights reserved.
//

///
///  Copyright (c) <2017>, Lixin
///  All rights reserved.
///
///  Redistribution and use in source and binary forms, with or without
///  modification, are permitted provided that the following conditions are met:
///  1. Redistributions of source code must retain the above copyright
///  notice, this list of conditions and the following disclaimer.
///  2. Redistributions in binary form must reproduce the above copyright
///  notice, this list of conditions and the following disclaimer in the
///  documentation and/or other materials provided with the distribution.
///  3. All advertising materials mentioning features or use of this software
///  must display the following acknowledgement:
///  This product includes software developed by Lixin.
///  4. Neither the name of the Lixin nor the
///  names of its contributors may be used to endorse or promote products
///  derived from this software without specific prior written permission.
///
///  THIS SOFTWARE IS PROVIDED BY Lixin ''AS IS'' AND ANY
///  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///  DISCLAIMED. IN NO EVENT SHALL Lixin BE LIABLE FOR ANY
///  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
///  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
///  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
///  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
///  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
///  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
///  Contact: JustinDellAdam@live.com
///

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define srcUdpSocket_buff_len 8192

/// 返回 socket FD
int srcUdpSocket_server(const char *address, int port) {
    /// 创建 socket
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    int reuseON = 1;
    int ret = -1;

    /// 绑定参数
    struct sockaddr_in serverAddress;
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_len = sizeof(struct sockaddr_in);
    serverAddress.sin_family = AF_INET;
    if (address == NULL || strlen(address) == 0 || strcmp(address, "255.255.255.255") == 0) {
        ret = setsockopt(socketFD, SOL_SOCKET, SO_BROADCAST, &reuseON, sizeof(reuseON));
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        ret = setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &reuseON, sizeof(reuseON));
        serverAddress.sin_addr.s_addr = inet_addr(address);
        serverAddress.sin_port = htons(port);
    }
    if (ret == -1) {
        return -1;
    }

    if (bind(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) {
        return -1;
    } else {
        return socketFD;
    }

}

int srcUdpSocket_receive(int socketFD, char *outData, int expectedLen, char *remoteIP, int *remotePort) {
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    bzero(&clientAddress, sizeof(struct sockaddr_in));
    int len = (int) recvfrom(socketFD, outData, (size_t) expectedLen, 0, (struct sockaddr *) &clientAddress,
                             &clientLen);
    char *clientIP = inet_ntoa(clientAddress.sin_addr);
    memcpy(remoteIP, clientIP, strlen(clientIP));
    *remotePort = clientAddress.sin_port;

    return len;
}

int srcUdpSocket_close(int socketFD) {
    return close(socketFD);
}

/// 返回 socket FD
int srcUdpSocket_client() {
    /// 建立套接字
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    int reuseON = 1;
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &reuseON, sizeof(reuseON));

    return socketFD;
}

/// 广播功能
void enable_broadcast(int socketFD) {
    int reuseON = 1;
    setsockopt(socketFD, SOL_SOCKET, SO_BROADCAST, &reuseON, sizeof(reuseON));
}

int srcUdpSocket_get_server_ip(char *host, char *ip) {
    struct hostent *hostEntry;
    struct sockaddr_in address;

    hostEntry = gethostbyname(host);
    if (hostEntry == NULL) {
        return -1;
    }

    bcopy(hostEntry->h_addr, (char *) &address.sin_addr, hostEntry->h_length);
    char *clientIP = inet_ntoa(address.sin_addr);
    memcpy(ip, clientIP, strlen(clientIP));

    return 0;
}

/// 向 地址:端口 发送消息
int srcUdpSocket_sentto(int socketFD, char *msg, int len, char *desAddress, int desPort) {
    struct sockaddr_in address;
    bzero(&address, sizeof(struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_port = htons(desPort);
    address.sin_addr.s_addr = inet_addr(desAddress);
    int sendLen = (int) sendto(socketFD, msg, (size_t) len, 0, (struct sockaddr *) &address, sizeof(address));

    return sendLen;
}
