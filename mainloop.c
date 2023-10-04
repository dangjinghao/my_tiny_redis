#include "test_common.h"
#include <liburing.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <termios.h>

#include "syntax.h"
#include "sock_module.h"
#include "parser.h"
#include "log.h"
COM_INNER_DECL const size_t HEADER_SIZE_LIMIT = 2 * 1024; // put or post
COM_INNER_DECL const unsigned int ENTRIES_SIZE = 1024;

COM_INNER_DECL struct io_uring ring;
COM_INNER_DECL int serverfd;
COM_INNER_DECL void sig_intp_handler(int signo)
{
    if (signo == SIGINT)
    {
        puts("bye!\n");
        io_uring_close_ring_fd(&ring);
        close(serverfd);
        exit(EXIT_SUCCESS);
    }
}

void disable_ctrl_c_output()
{
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) < 0)
    {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    term.c_lflag &= ~ECHOCTL;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0)
    {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}
//TODO:gtest
void solver(uint8_t *read_buf, size_t n)
{
    action_syntax_t syn;
    if (http_req_parser(read_buf, n, &syn) == -1)
    {
        return;
    }

    char *kw1 = syn.key == NULL ? "NULL" : (char *)syn.key;
    char *kw2 = syn.val == NULL ? "NULL" : (char *)syn.val;
    printf("syn: %s\t|k: %s\t|v: %s|type:%s \t|TTL: %ld\n", act_str[syn.action], kw1, kw2, type_str[syn.data_type], syn.TTL);

    free_syntax_block_content(&syn);
}

#ifndef TEST
int main(int argc, char const *argv[])
{
    int port = 8000;
    if (argc == 1)
    {
        printf("Using default port: %d\n", port);
    }
    else if (argc == 2)
    {
        port = atoi(argv[1]);
    }
    set_log_level(LOG_DEBUG);
    disable_ctrl_c_output();
    serverfd = init_sock(port);

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

                uint8_t *read_buf = calloc(HEADER_SIZE_LIMIT, sizeof(char));
                new_recv_event(&ring, clientfd, read_buf, HEADER_SIZE_LIMIT, 0, process_heap_info);

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

                    char *read_buf = process_heap_info->data;
                    log_printf_debug("\nrecv[%d]\n%s\n", cqe->res, read_buf);
                    solver(read_buf, cqe->res);
                    new_send_event(&ring, process_heap_info->sockfd, read_buf, cqe->res, 0, process_heap_info);
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

#endif