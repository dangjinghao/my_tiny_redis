#include "sock_module.h"
#include <liburing.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

static const unsigned int ENTRIES_SIZE = 1024;

static struct io_uring ring;
static int serverfd;
static void sig_intp_handler(int signo)
{
    if (signo == SIGINT)
    {
        puts("bye!\n");
        io_uring_close_ring_fd(&ring);
        close(serverfd);
        exit(0);
    }
}


int main(int argc, char const *argv[])
{
    serverfd = init_sock(8000);

    struct io_uring_params params = {0};
    io_uring_queue_init_params(ENTRIES_SIZE, &ring, &params);

    signal(SIGINT, sig_intp_handler);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(struct sockaddr);

    struct connect_info *first_heap_info = malloc(sizeof(struct connect_info));
    new_accept_event(&ring, serverfd, (struct sockaddr *)&client_addr, &client_len, 0, first_heap_info);

    while (true)
    {
        io_uring_submit(&ring);

        struct io_uring_cqe *cqe;
        io_uring_wait_cqe(&ring, &cqe);

        struct io_uring_cqe *cqes[10];
        int cqecount = io_uring_peek_batch_cqe(&ring, cqes, 10);

        for (size_t i = 0; i < cqecount; i++)
        {
            cqe = cqes[i];
            struct connect_info *process_heap_info = (struct connect_info *)cqe->user_data;

            if (process_heap_info->evtype == EV_ACCEPT_DONE)
            {

                if (cqe->res < 0)
                    continue;

                int clientfd = cqe->res;

                uint8_t *read_buf = malloc(sizeof(char) * 10240);
                new_recv_event(&ring, clientfd, read_buf, 10240, 0, process_heap_info);

                // add new accept event to server socket fd
                struct connect_info *next_accpet_info = malloc(sizeof(struct connect_info *));

                new_accept_event(&ring, serverfd, (struct sockaddr *)&client_addr, &client_len, 0, next_accpet_info);
            }
            // FIXME:maybe the buffer is not enough to store
            else if (process_heap_info->evtype == EV_READ_DONE)
            {

                if (cqe->res < 0)
                    continue;
                if (cqe->res == 0)
                {
                    close(process_heap_info->sockfd);
                }
                else
                {

                    char *write_buf = process_heap_info->data;
                    printf("recv[%d] --> %s\n", cqe->res, write_buf);
                    new_send_event(&ring, process_heap_info->sockfd, write_buf, cqe->res, 0, process_heap_info);
                }
            }
            else if (process_heap_info->evtype == EV_WRITE_DONE)
            {

                free(process_heap_info->data);
                free(process_heap_info);
                close(process_heap_info->sockfd);
            }
        }
        io_uring_cq_advance(&ring, cqecount);
    }
    return 0;
}
