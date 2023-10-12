#include "core.h"
#include "handler.h"
#include "test_common.h"
#include <time.h>
#include <liburing.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include "event_module.h"
#include "log.h"
#include <assert.h>

// main loop configuration

const size_t INIT_BUFFER_UNIT_SIZE = 2 * 1024; 
const size_t MAX_RECV_BUFFER_SIZE = 512 * 1024 * 1024; 
COM_INNER_DECL const unsigned int ENTRIES_SIZE_IN_QUEUE = 1024;
COM_INNER_DECL const unsigned long TIMER_INTERVAL_SECS = 30;

/****************************************/

struct event_info  *forever_timer_info = NULL;
COM_INNER_DECL struct io_uring ring;
COM_INNER_DECL int serverfd, loop_timer_fd;
COM_INNER_DECL void sig_intp_handler(int signo)
{
    if (signo == SIGINT)
    {
        puts("bye!\n");
        io_uring_close_ring_fd(&ring);
        close(serverfd);
        free(forever_timer_info);
        close(loop_timer_fd);
        release_all_data_in_core_tree();
        
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
        printf("Using specified port: %d\n", port);
    }
    set_log_level(LOG_DEBUG);
    disable_ctrl_c_output();
    serverfd = init_sock(port);
    loop_timer_fd = init_timer(TIMER_INTERVAL_SECS);

    struct io_uring_params params = {0};
    io_uring_queue_init_params(ENTRIES_SIZE_IN_QUEUE, &ring, &params);

    signal(SIGINT, sig_intp_handler);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(struct sockaddr);

    forever_timer_info = malloc(sizeof(struct event_info));
    struct event_info *first_reuse_event_heap_info = malloc(sizeof(struct event_info));

    // init the initial event
    new_accept_event(&ring, serverfd, (struct sockaddr *)&client_addr, &client_len, 0, first_reuse_event_heap_info);
    new_timer_event(&ring, timer_worker, loop_timer_fd, forever_timer_info);

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
            struct event_info *process_heap_info = (struct event_info *)cqe->user_data;

            if (process_heap_info->evtype == EV_ACCEPT_DONE)
            {
                EV_ACCPET_DONE_handler(cqe, &ring, serverfd, &client_addr, &client_len, process_heap_info);
            }
            else if (process_heap_info->evtype == EV_READ_DONE)
            {
                EV_READ_DONE_handler(cqe, &ring, process_heap_info);
            }
            else if (process_heap_info->evtype == EV_WRITE_DONE)
            {
                EV_WRITE_DONE_handler(cqe, &ring,process_heap_info);
            }
            else if (process_heap_info->evtype == EV_TIMER_DONE)
            {
                EV_TIMER_DONE_handler(loop_timer_fd, process_heap_info, &ring);
            }
        }
        io_uring_cq_advance(&ring, cqecount);
    }
    return 0;
}

#endif