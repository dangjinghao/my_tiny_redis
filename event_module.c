#include "test_common.h"

#include <bits/time.h>
#include <bits/types/struct_itimerspec.h>
#include <time.h>
#include <unistd.h>
#include <liburing/io_uring.h>
#include <stdio.h>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <liburing.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include "log.h"
#include "event_module.h"

// submit new accept event to io uring SQ
void new_accept_event(struct io_uring *ring, int serverfd, struct sockaddr *addr, socklen_t *addrlen, int flags, struct event_info *new_heap_info)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_accept(sqe, serverfd, addr, addrlen, flags);

    struct event_info *info = new_heap_info;
    info->data = NULL;
    info->evtype = EV_ACCEPT_DONE;
    info->sockfd = serverfd;

    io_uring_sqe_set_data(sqe, info);
}

void new_recv_event(struct io_uring *ring, int clientfd, void *read_buf, size_t buf_size, int flags, void *reuse_heap_info)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_recv(sqe, clientfd, read_buf, buf_size, flags);

    struct event_info *info = reuse_heap_info;
    info->data = read_buf;
    info->evtype = EV_READ_DONE;
    info->sockfd = clientfd;

    io_uring_sqe_set_data(sqe, info);
}

// submit new send event
void new_send_event(struct io_uring *ring, int clientfd, void *write_buf, size_t n, int flags, void *reuse_heap_info)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_send(sqe, clientfd, write_buf, n, flags);
    struct event_info *info = reuse_heap_info;
    info->data = write_buf;
    info->evtype = EV_WRITE_DONE;
    info->sockfd = clientfd;
    io_uring_sqe_set_data(sqe, info);
}

void new_timer_event(struct io_uring *ring, void (*func)(void),int timerfd, void *reuse_heap_info)
{
    // if I read something in here, when the first time of calling this function
    // before loop,whole process will be blocked for specified seconds.
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_poll_add(sqe,timerfd,POLLIN);
    struct event_info *info = reuse_heap_info;
    info->data = func;
    info->evtype = EV_TIMER_DONE;
    info->sockfd = -1;
    io_uring_sqe_set_data(sqe, info);
}

// read some char from timer fd to block the timer again 
void flush_timer_when_available(int timerfd)
{
    uint64_t tmp_timer_count;
    read(timerfd,&tmp_timer_count,sizeof(tmp_timer_count));
}

int init_sock(uint16_t port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        log_errno("socket");
    }
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = ntohs(port), .sin_addr.s_addr = ntohl(INADDR_ANY)};
    int val = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1)
    {
        log_errno("bind");
    }

    if (listen(sockfd, SOMAXCONN) == -1)
    {
        log_errno("listen");
    }

    return sockfd;
}

int init_timer(int secs)
{
    struct timespec now;
    timespec_get(&now,TIME_UTC);

    struct itimerspec new_value = {.it_value.tv_sec = secs,
                                    .it_value.tv_nsec=0,
                                    .it_interval.tv_sec = secs,
                                    .it_interval.tv_nsec = 0};

    int fd = timerfd_create(CLOCK_REALTIME,0);
    assert(timerfd_settime(fd,0,&new_value,NULL) != -1);
    return fd;
}