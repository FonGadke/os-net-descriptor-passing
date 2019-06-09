//
// Created by Кирилл Чулков on 2019-05-11.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
#include <sys/un.h>

const size_t buf_size = 2048;
const char* SOCKET_NAME = "./mysocket";


int main() {
    int sock, listener;
    struct sockaddr_un addr;

    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, SOCKET_NAME, strlen(SOCKET_NAME) + 1);

    unlink(SOCKET_NAME);

    listener = socket(AF_UNIX, SOCK_STREAM, 0);

    if (listener < 0) {
        perror("Can't create socket\n");
        exit(EXIT_FAILURE);
    }

    if (bind(listener, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("Can't bind\n");
        exit(EXIT_FAILURE);
    }

    listen(listener, 1);

    while (true) {
        sock = accept(listener, NULL, NULL);
        if (sock < 0) {
            perror("Can't accept\n");
            exit(EXIT_FAILURE);
        }

        int pipefd[2];

        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        struct msghdr msg = { 0 };
        struct cmsghdr *cmsg;
        char iobuf[1];
        struct iovec io = {
                .iov_base = iobuf,
                .iov_len = sizeof(iobuf)
        };
        union {
            char buf[CMSG_SPACE(sizeof(pipefd[1]))];
            struct cmsghdr align;
        } u;

        msg.msg_iov = &io;
        msg.msg_iovlen = 1;
        msg.msg_control = u.buf;
        msg.msg_controllen = sizeof(u.buf);
        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(pipefd[1]));
        memcpy(CMSG_DATA(cmsg), &pipefd[1], sizeof(pipefd[1]));

        char buf[CMSG_SPACE(sizeof pipefd[1])];

        int code = sendmsg(sock, &msg, 0);

        if (code == -1) {
            perror("Send message failed");
        }

        close(pipefd[1]);

        while (read(pipefd[0], &buf, 1) > 0) {
            write(STDOUT_FILENO, &buf, 1);
        }
        close(sock);
    }
}