#pragma once

#include <netinet/in.h>

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
#include "event_module.h"

void EV_ACCPET_DONE_handler(struct io_uring_cqe *cqe, struct io_uring *ring, int serverfd,
                            struct sockaddr_in *client_addr, socklen_t *client_len, struct event_info *process_heap_info);


void EV_READ_DONE_handler(struct io_uring_cqe *cqe, struct io_uring *ring, struct event_info *process_heap_info);

void timer_worker();

void EV_WRITE_DONE_handler(struct io_uring_cqe *cqe, struct io_uring *ring, struct event_info *process_heap_info);

void EV_TIMER_DONE_handler(int  loop_timer_fd,struct event_info *process_heap_info,struct io_uring *ring);