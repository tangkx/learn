#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>  
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, old_option | O_NONBLOCK);
    return old_option;
}

void addfd(int epollfd, int fd, int op)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = op;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

int main(int argc, char* argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);

    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(connfd > 0);
    setnonblocking(connfd);

    int epollfd = epoll_create(5);
    assert(epollfd > 0);
    addfd(epollfd, connfd, EPOLLIN | EPOLLET);
    addfd(epollfd, STDIN_FILENO, EPOLLIN | EPOLLET);

    ret = connect(connfd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1 && errno != EINPROGRESS)
    {
        printf("connect failure\n");
        close(connfd);
        close(epollfd);
        return -1;
    }

    epoll_event events[1024];
    for(;;)
    {
        int number = epoll_wait(epollfd, events, 1024, -1);
         char buf[BUFFER_SIZE];
        if(number == -1 && errno != EAGAIN)
        {
            printf("epoll_wait failure\n");
            break;
        }

        for(int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            memset(buf, 0, BUFFER_SIZE);
            if(events[i].events & EPOLLIN)
            {
                if(sockfd == STDIN_FILENO)
                {
                    ret = read(STDIN_FILENO, buf, BUFFER_SIZE - 1);
                    if(ret > 0)
                    {
                        printf("stdin data: %s\n", buf);
                        ret = send(connfd, buf, ret, 0);
                        printf("client send result: %d\n", ret);
                    }
                    else
                    {
                        printf("recv failure: %s\n", strerror(errno));
                        break;
                    }
                }
                else
                {
                    ret = recv(connfd, buf, BUFFER_SIZE - 1, 0);
                    if(ret > 0)
                    {
                        printf("recv server data: %s\n", buf);
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    return 1;
}