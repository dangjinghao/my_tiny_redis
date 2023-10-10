#ifndef EVENT_MODULE_H
#define EVENT_MODULE_H

#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>
#include <liburing.h>

struct event_info
{
    int sockfd; // for accept_event, it is server fd;for timeout_event is -1 for others, it is client fd. 
    enum
    {
        EV_ACCEPT_DONE,
        EV_READ_DONE,
        EV_WRITE_DONE,
        EV_TIMER_DONE,
    } evtype;
    void *data; // for accept_event,it is NULL;send is send data and recv is receive data;timeout_event is function pointer which should be executed
};

void new_accept_event(struct io_uring *ring, int serverfd, struct sockaddr *addr, socklen_t *addrlen, int flags, struct event_info *new_heap_info);
void new_send_event(struct io_uring *ring, int clientfd, void *write_buf, size_t n, int flags, void *reuse_heap_info);
void new_recv_event(struct io_uring *ring, int clientfd, void *heap_buf, size_t buf_size, int flags, void *reuse_heap_info);
void new_timer_event(struct io_uring *ring, void (*func)(void),int timerfd, void *reuse_heap_info);

void flush_timer_when_available(int timerfd);

int init_sock(uint16_t port);
int init_timer(int secs);
#endif