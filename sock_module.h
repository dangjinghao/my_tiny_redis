#ifndef SOCK_MODULE_H
#define SOCK_MODULE_H

#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>
#include <liburing.h>

struct connect_info 
{
    int sockfd; // for accept_event, it is server fd, for others, it is client fd.
    enum {
        EV_ACCEPT_DONE,
        EV_READ_DONE,
        EV_WRITE_DONE,
    } evtype;
    void*data; // for accept_event,it is NULL;send is send data and recv is receive data
};

void new_accept_event(struct io_uring*ring,int serverfd,struct sockaddr*addr,socklen_t*addrlen,int flags,struct connect_info*new_heap_info);
void new_send_event(struct io_uring*ring,int clientfd,void *write_buf,size_t n,int flags,void*reuse_heap_info);
void new_recv_event(struct io_uring*ring,int clientfd,void*heap_buf,size_t buf_size,int flags,void*reuse_heap_info);
int init_sock(uint16_t port);

#endif