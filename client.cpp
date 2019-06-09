//
// Created by Кирилл Чулков on 2019-05-12.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstring>
#include <sys/un.h>

const char* SOCKET_NAME = "./mysocket";

int main() {
    int sock;
    struct sockaddr_un addr;

    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, SOCKET_NAME, strlen(SOCKET_NAME) + 1);

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Can't create socket");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("Can't connect");
        exit(EXIT_FAILURE);
    }

    struct msghdr msg = {0};
    char iobuf[1];
    struct iovec io = {
            .iov_base = iobuf,
            .iov_len = sizeof(iobuf)
    };
    union {
        char buf[CMSG_SPACE(sizeof(int))];
        struct cmsghdr align;
    } u;

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = u.buf;
    msg.msg_controllen = sizeof(u.buf);

    ssize_t read_bytes = recvmsg(sock, &msg, 0);
    if (read_bytes == -1 || read_bytes == 0) {
        perror("Receive message failed");
        return 0;
    }

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    int fd;
    memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));

    write(fd, "It is unlikely that there will be an interesting message.", 57);

    close(fd);
    close(sock);

    return 0;
}