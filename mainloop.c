#include "test_common.h"
#include <liburing.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <termios.h>
#include "core.h"
#include "syntax.h"
#include "sock_module.h"
#include "parser.h"
#include "log.h"
#include <assert.h>
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
// TODO:gtest
void solver(uint8_t *read_buf, size_t n,uint8_t *send_buf,size_t send_buf_n)
{
    action_syntax_t syn;
    if (http_req_parser(read_buf, n, &syn) == -1)
    {
        return;
    }

    char *kw1 = syn.key == NULL ? "NULL" : (char *)syn.key;
    char *kw2 = syn.val == NULL ? "NULL" : (char *)syn.val;
    struct tiny_string_raw*value_result;
    int result_get;
    log_printf_debug("syn: %s\t|k: %s\t|v: %s|type:%s \t|TTL: %ld\n", act_str[syn.action], kw1, kw2, type_str[syn.data_type], syn.TTL);

    switch (syn.action)
    {
    case sat_CREATE:
        log_msg_debug("dealing with create request");
        if(put_into_tree(&syn)!= 0)
        {
            log_msg_warn("something is wrong in `put_into_tree`!");
        }
        gen_response(send_buf,send_buf_n,(uint8_t*)"create done!\n",13);
        break;
    case sat_DELETE:
        log_msg_debug("dealing with delete request");
        
        break;
    case sat_GET:
        result_get = get_from_tree(&syn,&value_result);

        switch (result_get) {
            case 0:
                gen_response(send_buf,send_buf_n,value_result->p,value_result->used);
                break;
            case -2:
                gen_response(send_buf,send_buf_n,(uint8_t*)"no content",10);
                break;
            default:
                log_msg_warn("something is wrong in `get_from_tree`!");
        }
        log_msg_debug("dealing with get request");
        break;

    case sat_UPDATE:
        log_msg_debug("dealing with update request");
        break;
    }
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
    set_log_level(LOG_INFO);
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
                    uint8_t *read_buf = process_heap_info->data;
                    uint8_t *send_buf = calloc( 2048,sizeof(uint8_t));

                    log_printf_debug("\nrecv[%d]\n%s\n", cqe->res, read_buf);
                    solver(read_buf, cqe->res,send_buf,2048);

                    // we don't need this read buffer now
                    free(read_buf);
                    process_heap_info->data = send_buf;
                    new_send_event(&ring, process_heap_info->sockfd, send_buf, 2048 , 0, process_heap_info);
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