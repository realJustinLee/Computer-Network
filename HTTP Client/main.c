//
// Created by 李欣 on 2017/9/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/// 搜索字符串右边起的第一个匹配字符
char *safeStrChr(char *srcString, char tgtChar) {
    int bottomIndex = (int) strlen(srcString);
    if (!*srcString) return 0;
    while (srcString[bottomIndex - 1])
        if (strchr(srcString + (bottomIndex - 1), tgtChar))
            return (srcString + (bottomIndex - 1));
        else bottomIndex--;
    return 0;
}


// /// 把字符串转换为全小写 # 同 strlwr()
// void strlwr(char *srcString) {
//    while (*srcString) *srcString = (char) tolower(*srcString++);
// }

/// 从字符串 srcString 中分析出网站地址和端口,并得到用户请求的文件
void infoAnalyse(char *src, char *host, char *file, int *port) {
    char *rest_of_src;
    char *indexPtr;
    /// 设置内存
    bzero(host, sizeof(host));
    bzero(file, sizeof(file));
    /// 初始化为默认 HTTP 端口为 80
    *port = 80;
    if (*src != 0) {
        /// 初始化 rest_of_src
        rest_of_src = src;
        /**
         * 检查协议类型 (HTTP or HTTP[S])
         * 检测这里的内容: (使用"-[-","-]-"来标记)
         * -[- http:// -]- www.example.com/index.html
         */
        if (!strncmp(rest_of_src, "http://", strlen("http://")))
            rest_of_src = src + strlen("http://");
        else if (!strncmp(rest_of_src, "https://", strlen("https://")))
            rest_of_src = src + strlen("https://");
        /**
         * 检测 host 结尾的 "/"
         * 检测这里的内容: (使用"-[-","-]-"来标记)
         * http://www.example.com -[- / -]- index.html
         */
        indexPtr = strchr(rest_of_src, '/');
        if (indexPtr) {
            /**
             * "www.example.com/index.html" - "/index.html" -> host
             * host = "www.example.com"
             */
            memcpy(host, rest_of_src, strlen(rest_of_src) - strlen(indexPtr));
            if (indexPtr + 1) {
                /**
                 * 指针后以一位恰好获得文件字符串首地址
                 * "index.html"
                 *  ^
                 *  indexPtr
                 */
                memcpy(file, indexPtr + 1, strlen(indexPtr) - 1);
                /**
                 * 将 file 字符串结尾置'\0',防止数组越界
                 * "index.html'\0'"
                 *             ^
                 */
                file[strlen(indexPtr) - 1] = 0;
            }
        } else {
            /**
            * 检测不到 host 结尾的 "/"
            * 直接把 rest_of_src 赋值给 host
            * http:// -[- www.example.com -]-
            */
            memcpy(host, rest_of_src, strlen(rest_of_src));
        }
        /**
         * 将 host 字符串结尾置'\0',防止数组越界
         * "index.html'\0'"
         *             ^
         */
        if (indexPtr) {
            host[strlen(rest_of_src) - strlen(indexPtr)] = 0;
        } else {
            host[strlen(rest_of_src)] = 0;
        }
        /**
         * 检测 ":" 来判断是否制定了端口号
         * 检测这里的内容: (使用"-[-","-]-"来标记)
         * http://www.example.com/index.html -[- : -]- 80
         */
        rest_of_src = strchr(host, ':');
        if (rest_of_src) {
            *port = atoi(rest_of_src + 1);
        } else {
            *port = 80;
        }
    }
}

int main(int argc, char *argv[]) {
    int socketFD;
    char buffer[1024];
    struct sockaddr_in serverAddress;
    struct hostent *host;
    int port, requestLength;
    char hostAddress[256];
    char remoteFile[1024];
    char localFile[256];
    FILE *file;
    char request[1024];
    int sendCondition, dataOutCounter;
    int dataInCounter;
    /// isRFinD -> is Remote File in Directory
    char *isRFinD;

    /// 首次运行的时候可能有人不知道使用方法
    if (argc != 2) {
        fprintf(stderr, "[!]Usage: %s URI\a\n", argv[0]);
        exit(1);
    }
    printf("\033[1;37m======================================= \033[1;31mSeparating Line \033[1;37m=======================================\n\033[0m");
    printf("\033[1;36m[+] URI: \033[33m%s\n\033[0m", argv[1]);
    // strlwr(argv[1]);
    // printf("\033[1;36m[+] URI after strlwr: \033[33m%s\n\033[0m", argv[1]);

    infoAnalyse(argv[1], hostAddress, remoteFile, &port);
    printf("\033[1;36m[+] Host: \033[33m%s\n\033[0m", hostAddress);
    printf("\033[1;36m[+] File: \033[33m%s\n\033[0m", remoteFile);
    printf("\033[1;36m[+] Port: \033[33m%d\n\033[0m", port);

    /// 取得主机IP地址
    if ((host = gethostbyname(hostAddress)) == NULL) {
        fprintf(stderr, "Unable to get hostname, %s\n", strerror(errno));
        exit(1);
    }

    /// 客户程序开始建立 sockFD描述符
    if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Socket Error:%s\a\n", strerror(errno));
        exit(1);
    }

    /// 客户程序填充服务端的资料
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr = *((struct in_addr *) host->h_addr);

    /// 客户程序发起连接请求
    if (connect(socketFD, (struct sockaddr *) (&serverAddress), sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "Connect Error:%s\a\n", strerror(errno));
        exit(1);
    }
    printf("\033[1;37m======================================= \033[1;31mSeparating Line \033[1;37m=======================================\n\033[0m");
    printf("\033[1;36m[+] HTTP request header: \n\033[1;34m");
    /// 准备 request ,将要发送给主机
    sprintf(request, ""
            "GET /%s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "Accept: */*\r\n"
            "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Safari/604.1.38\r\n"
            "Accept-Language: zh-cn\r\n"
            "Connection: Close\r\n\r\n",
            remoteFile,
            hostAddress,
            port
    );
    printf("%s", request);
    printf("\033[1;37m======================================= \033[1;31mSeparating Line \033[1;37m=======================================\n\033[0m");


    /// 取得真实的文件名(而不是路径)
    if (remoteFile && *remoteFile) isRFinD = safeStrChr(remoteFile, '/');
    else isRFinD = 0;
    bzero(localFile, sizeof(localFile));
    if (isRFinD && *isRFinD) {
        if ((isRFinD + 1) && *(isRFinD + 1)) strcpy(localFile, isRFinD + 1);
        else
            memcpy(localFile, remoteFile, strlen(remoteFile) - 1);
    } else if (remoteFile && *remoteFile) strcpy(localFile, remoteFile);
    else
        strcpy(localFile, "index.html");
    printf("\033[1;36m[+] Requesting file from server: \033[33m%s\n\033[0m", localFile);

    /// 发送 http 请求(request)
    dataOutCounter = 0;
    requestLength = (int) strlen(request);
    while (dataOutCounter < requestLength) {
        sendCondition = (int) write(socketFD, request + dataOutCounter, (size_t) (requestLength - dataOutCounter));
        if (sendCondition == -1) {
            printf("\033[1;31m[-]Send Error!%s\n\033[0m", strerror(errno));
            exit(0);
        }
        dataOutCounter += sendCondition;
        printf("\033[1;36m[+] Bytes sent: \033[33m%d.\n", dataOutCounter);
    }

    file = fopen(localFile, "a");
    if (!file) {
        printf("\033[1;31m[-] Unable to create file! %s\n\033[0m", strerror(errno));
        return 0;
    }
    printf("\033[1;37m======================================= \033[1;31mSeparating Line \033[1;37m=======================================\n\033[0m");
    printf("\033[1;36m[+] HTTP response header: \n\033[1;34m");
    dataInCounter = 0;
    /// 连接成功, 接收 http 响应(response)
    // while ((requestLength = (int) read(socketFD, buffer, 1)) == 1) {
    while ((int) read(socketFD, buffer, 1) == 1) {
        if (dataInCounter < 4) {
            if (buffer[0] == '\r' || buffer[0] == '\n') dataInCounter++;
            else dataInCounter = 0;
            /// 显示 http 头文件信息
            printf("%c", buffer[0]);
        } else {
            /// 将 http 主体信息写入文件
            fwrite(buffer, 1, 1, file);
            dataInCounter++;
            /// 每1K时存盘一次
            if (dataInCounter % 1024 == 0) fflush(file);
        }
    }
    fclose(file);
    /// 结束通讯
    close(socketFD);
    printf("\033[0m\033[1;37m======================================= \033[1;31mSeparating Line \033[1;37m=======================================\n\033[0m");
    char cmd[1024] = "open /Applications/Safari.app ";
    strcat(cmd, localFile);
    system(cmd);
    system("say "
                   "进到网络课上各个都是人才，说话又好听，超喜欢在里面。"
                   "自己喜欢网络课，网络课比别的课好多了。在寝室里面一个人很无聊，没有女朋友，也没有男朋友。"
                   "肯定要上啊，不上没有知识用。打工的话也不能打一辈子，做生意又不会做，就是来上钱老师的网络课才能维持生活。"
                   "上网络课就像回寝室一样，我大年三十晚上都不回去，就平时要回寝室睡觉，我回去躺躺。"
                  );
    exit(0);
}
