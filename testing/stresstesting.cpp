#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE 2048
#define MAX_EVENT_NUMBER 10000

static const char* request = "GET http://localhost/index.html HTTP/1.1\r\nConnection: keep-alive\r\n\r\nxxxxxxxxxx";


int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, O_NONBLOCK | old_option);
    return old_option;
}

void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLOUT | EPOLLET | EPOLLERR;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

bool write_nbytes(int sockfd, const char* buffer, int len)
{
    int bytes_write = 0;
    printf("write out %d bytes to socket %d\n", len, sockfd);
    for(;;)
    {
        bytes_write = send(sockfd, buffer, len, 0);
        if(bytes_write == -1)
            return false;
        else if(bytes_write == 0)
            return false;
        
        len -= bytes_write;
        buffer = buffer + bytes_write;
        if(len <= 0)
            return true;
    }
}

bool read_once(int sockfd, char* buffer, int len)
{
    int bytes_read = 0;
    memset(buffer, 0, len);
    bytes_read = recv(sockfd, buffer, len, 0);
    if(bytes_read == -1)
        return false;
    else if(bytes_read == 0)
        return false;
    
    printf("read in %d bytes from socket %d with content: %s\n", bytes_read, sockfd, buffer);
    return true;
}

void start_conn(int epollfd, int num, const char* ip, int port)
{
    int ret = 0;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);

    for(int i = 0; i < num; i++)
    {
        // sleep(1);
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        printf("create 1 sock\n");
        if(sockfd < 0)
            continue;
        
        if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
        {
            printf("build connection %d\n", i);
            addfd(epollfd, sockfd);
        }
    }
}

void close_conn(int epollfd, int sockfd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, 0);
    close(sockfd);
}

int main(int argc, char* argv[])
{
    assert(argc == 4);
    int epoll_fd = epoll_create(100);
    start_conn(epoll_fd, atoi(argv[3]), argv[1], atoi(argv[2]));
    epoll_event events[MAX_EVENT_NUMBER];
    char buffer[BUFFER_SIZE];
    for(;;)
    {
        int fds = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
        // printf("epoll_wait return: %d\n", fds);
        for(int i = 0; i < fds; ++i)
        {
            int sockfd = events[i].data.fd;
            if(events[i].events & EPOLLIN)
            {
                if(!read_once(sockfd, buffer, BUFFER_SIZE))
                    close_conn(epoll_fd, sockfd);
                
                struct epoll_event event;
                event.events = EPOLLOUT | EPOLLET | EPOLLERR;
                event.data.fd = sockfd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
            }
            else if(events[i].events & EPOLLOUT)
            {
                if(!write_nbytes(sockfd, request, strlen(request)))
                    close_conn(epoll_fd, sockfd);
                
                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET | EPOLLERR;
                event.data.fd = sockfd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
            }
            else if(events[i].events & EPOLLERR)
            {
                close_conn(epoll_fd, sockfd);
            }
        }
    }
    return 0;
}