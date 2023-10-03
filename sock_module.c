#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <liburing.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <assert.h>
#include "log.h"
#include "sock_module.h"


// submit new accept event to io uring SQ
void new_accept_event(struct io_uring*ring,int serverfd,struct sockaddr*addr,socklen_t*addrlen,int flags,struct connect_info*new_heap_info)
{
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    io_uring_prep_accept(sqe,serverfd,addr,addrlen,flags);

    struct connect_info *info = new_heap_info;
    info->data = NULL;
    info->evtype = EV_ACCEPT_DONE;
    info->sockfd = serverfd;    

    io_uring_sqe_set_data(sqe,info);
}

void new_recv_event(struct io_uring*ring,int clientfd,void*read_buf,size_t buf_size,int flags,void*reuse_heap_info)
{
    struct io_uring_sqe*sqe = io_uring_get_sqe(ring);
    io_uring_prep_recv(sqe,clientfd,read_buf,buf_size,flags);
    
    struct connect_info *info = reuse_heap_info;
    info->data = read_buf;
    info->evtype = EV_READ_DONE;
    info->sockfd = clientfd;    

    io_uring_sqe_set_data(sqe,info);
}


// submit new send event 
void new_send_event(struct io_uring*ring,int clientfd,void *write_buf,size_t n,int flags,void*reuse_heap_info)
{
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    io_uring_prep_send(sqe,clientfd,write_buf,n,flags);
    struct connect_info *info = reuse_heap_info;
    info->data = write_buf;
    info->evtype = EV_WRITE_DONE;
    info->sockfd = clientfd;    
    io_uring_sqe_set_data(sqe,info);
}



int init_sock(uint16_t port)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1){
        log_errno("socket");
    }
    struct sockaddr_in addr = {.sin_family = AF_INET,.sin_port = ntohs(port),.sin_addr.s_addr = ntohl(INADDR_ANY)};
    if(bind(sockfd,(struct sockaddr*)&addr,sizeof(struct sockaddr)) == -1)
    {
        log_errno("bind");
    }
    
    if(listen(sockfd,SOMAXCONN) == -1)
    {
        log_errno("listen");
    }
    
    return sockfd;
}

