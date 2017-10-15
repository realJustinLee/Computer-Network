//
// Created by 李欣 on 2017/10/15.
//

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    char buffer[128];
    int result, nRead;
    fd_set inputs, testFDs;
    struct timeval timeout;

    FD_ZERO(&inputs);
    FD_SET(0, &inputs);

    while (true) {
        testFDs = inputs;
        timeout.tv_sec = 2;
        timeout.tv_usec = 500000;

        result = select(FD_SETSIZE, &testFDs, (fd_set *) 0, (fd_set *) 0, &timeout);

        switch (result) {
            case 0:
                printf("timeout\n");
                break;
            case -1:
                perror("select");
                exit(1);
            default:
                if (FD_ISSET(0, &testFDs)) {
                    ioctl(0, FIONREAD, &nRead);
                    if (nRead == 0) {
                        printf("keyboard done\n");
                        exit(0);
                    }
                    nRead = read(0, buffer, nRead);
                    buffer[nRead] = 0;
                    printf("read %d from keyboard: %s", nRead, buffer);
                }
                break;
        }
    }
}