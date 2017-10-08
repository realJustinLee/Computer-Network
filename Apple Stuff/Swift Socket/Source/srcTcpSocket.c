//
//  srcTcpSocket.c
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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <dirent.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>

void srcTcpSocket_set_block(int socket, int on) {
    int flags;
    /// 获取文件的flags，即open函数的第二个参数
    flags = fcntl(socket, F_GETFL, 0);
    if (on == 0) {
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    } else {
        flags &= ~O_NONBLOCK;
        fcntl(socket, F_SETFL, flags);
    }

}

int srcTcpSocket_connect(const char *host, int port, int timeout) {
    struct sockaddr_in serverAddress;
    struct hostent *hostEntry;
    int socketFD = -1;
    hostEntry = gethostbyname(host);
    if (hostEntry == NULL) {
        return -1;
    }

    bcopy(hostEntry->h_addr, (char *) &serverAddress.sin_addr, hostEntry->h_length);
    serverAddress.sin_family = (sa_family_t) hostEntry->h_addrtype;
    serverAddress.sin_port = htons(port);
    socketFD = socket(hostEntry->h_addrtype, SOCK_STREAM, 0);
    srcTcpSocket_set_block(socketFD, 0);
    connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    fd_set writeFD;
    struct timeval tvSelect;
    FD_ZERO(&writeFD);
    FD_SET(socketFD, &writeFD);
    tvSelect.tv_sec = timeout;
    tvSelect.tv_usec = 0;

    int retVal = select(socketFD + 1, NULL, &writeFD, NULL, &tvSelect);
    if (retVal < 0) {
        close(socketFD);
        return -2;
    } else if (retVal == 0) {
        /// 链接超时
        close(socketFD);
        return -3;
    } else {
        int error = 0;
        int errLen = sizeof(error);
        getsockopt(socketFD, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &errLen);
        /// 连接失败
        if (error != 0) {
            close(socketFD);
            return -4;
        }

        int set = 1;
        setsockopt(socketFD, SOL_SOCKET, SO_NOSIGPIPE, (void *) &set, sizeof(int));
        return socketFD;
    }
}

int srcTcpSocket_close(int socketFD) {
    return close(socketFD);
}

/// struct timeval结构用于描述一段时间长度，如果在这个时间内，需要监视的描述符没有事件发生则函数返回，返回值为0。
/// fd_set（它比较重要所以先介绍一下）是一组文件描述字(fd)的集合，它用一位来表示一个fd（下面会仔细介绍），对于fd_set类型通过下面四个宏来操作：
///     FD_ZERO(fd_set *fdset);将指定的文件描述符集清空，在对文件描述符集合进行设置前，必须对其进行初始化，如果不清空，由于在系统分配内存空间后，通常并不作清空处理，所以结果是不可知的。
///     FD_SET(fd_set *fdset);用于在文件描述符集合中增加一个新的文件描述符。
///     FD_CLR(fd_set *fdset);用于在文件描述符集合中删除一个文件描述符。
///     FD_ISSET(int fd,fd_set *fdset);用于测试指定的文件描述符是否在该集合中。

int srcTcpSocket_pull(int socketFD, char *data, int len, int timeout_sec) {
    int readLen = 0;
    int dataLen = 0;
    if (timeout_sec > 0) {
        fd_set FD_set;
        struct timeval timeout;
        timeout.tv_usec = 0;
        timeout.tv_sec = timeout_sec;
        FD_ZERO(&FD_set);
        FD_SET(socketFD, &FD_set);
        int ret = select(socketFD + 1, &FD_set, NULL, NULL, &timeout);
        if (ret <= 0) {
            /// select-call 在发送数据之前就失败或者超时了
            return ret;
        }
    }
    /// 使用循环却不接收到所有数据
    do {
        readLen = (int) read(socketFD, data + dataLen, (size_t) (len - dataLen));
        if (readLen > 0) {
            dataLen += readLen;
        }
    } while (readLen > 0);

    return dataLen;
}

int srcTcpSocket_send(int socketFD, const char *data, int len) {
    int bytesWrite = 0;
    while (len - bytesWrite > 0) {
        int writeLen = (int) write(socketFD, data + bytesWrite, (size_t) (len - bytesWrite));
        if (writeLen < 0) {
            return -1;
        }
        bytesWrite += writeLen;
    }
    return bytesWrite;
}

/// 返回 socketFD
int srcTcpSocket_listen(const char *address, int port) {
    /// 创建 socket
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    int reuseON = 1;
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &reuseON, sizeof(reuseON));

    /// 绑定参数
    struct sockaddr_in serverAddress;
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(address);
    serverAddress.sin_port = htons(port);
    if (bind(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) {
        if (listen(socketFD, 128) == 0) {
            return socketFD;
        } else {
            /// Listen Error
            return -2;
        }
    } else {
        /// Bind Error
        return -1;
    }
}

/// 返回 客户端socketFD
int srcTcpSocket_accept(int on_socketFD, char *remoteIP, int *remotePort, int timeout_sec) {
    struct sockaddr_in clientAddress;
    fd_set FD_set;
    FD_ZERO(&FD_set);
    FD_SET(on_socketFD, &FD_set);
    struct timeval *timePtr = NULL;
    struct timeval timeout;
    if (timeout_sec > 0) {
        timeout.tv_sec = timeout_sec;
        timeout.tv_usec = 0;
        timePtr = &timeout;
    }
    if (select(FD_SETSIZE, &FD_set, NULL, NULL, timePtr) != 1) {
        return -1;
    }
    int socketFD = accept(on_socketFD, (struct sockaddr *) &clientAddress, (socklen_t *) sizeof(clientAddress));
    char *clientIP = inet_ntoa(clientAddress.sin_addr);
    memcpy(remoteIP, clientIP, strlen(clientIP));
    *remotePort = clientAddress.sin_port;
    if (socketFD > 0) {
        int set = 1;
        setsockopt(socketFD, SOL_SOCKET, SO_NOSIGPIPE, (void *) &set, sizeof(int));
        return socketFD;
    } else {
        return -1;
    }
}

/// 返回 socket 端口
int srcTcpSocket_port(int socketFD) {
    struct sockaddr_in socketIN;
    socklen_t len = sizeof(socketIN);
    if (getsockname(socketFD, (struct sockaddr *) &socketIN, &len) == -1) {
        return -1;
    } else {
        return ntohs(socketIN.sin_port);
    }
}
