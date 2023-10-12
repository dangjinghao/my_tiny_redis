
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
#include "core.h"
#include "syntax.h"
#include "event_module.h"
#include "parser.h"
#include "log.h"
#include <assert.h>

extern const size_t INIT_BUFFER_UNIT_SIZE;
extern const size_t MAX_RECV_BUFFER_SIZE;
void timer_worker()
{
    log_msg_info("starting clear outdated node");
    clear_outdated_node();
    log_msg_info("clear outdated node done");
}
// declaration
void EV_WRITE_DONE_handler(struct io_uring_cqe *cqe, struct io_uring *ring, struct event_info *process_heap_info);
size_t gen_response(uint8_t **send_buf, uint8_t *content, size_t content_length);
size_t solver(uint8_t *read_buf, size_t n, uint8_t **send_buf_empty_ptr);
void EV_ACCPET_DONE_handler(struct io_uring_cqe *cqe, struct io_uring *ring, int serverfd,
                            struct sockaddr_in *client_addr, socklen_t *client_len, struct event_info *process_heap_info);
void EV_READ_DONE_handler(struct io_uring_cqe *cqe, struct io_uring *ring, struct event_info *process_heap_info);
void EV_TIMER_DONE_handler(int loop_timer_fd, struct event_info *process_heap_info, struct io_uring *ring);
// declaration done

size_t gen_response(uint8_t **send_buf, uint8_t *content, size_t content_length)
{
    size_t print_char_size = sprintf((char *)(*send_buf), "HTTP/1.0 200 OK\r\nServer: tiny_redis_httpd/0.0.1\r\nContent-Type: text/plain\r\nContent-Length:%ld\r\n\r\n", content_length);
    if (print_char_size + content_length > INIT_BUFFER_UNIT_SIZE)
    {
        *send_buf = realloc(*send_buf, print_char_size + content_length);
    }

    uint8_t *start_body = *send_buf + print_char_size;
    memcpy(start_body, content, content_length);
    return print_char_size + content_length;
}

// TODO:gtest
size_t solver(uint8_t *read_buf, size_t n, uint8_t **send_buf_empty_ptr)
{
    // default size
    uint8_t *final_content = NULL;
    *send_buf_empty_ptr = malloc(INIT_BUFFER_UNIT_SIZE * sizeof(uint8_t));
    action_syntax_t syn;
    struct tiny_string_raw *value_result;
    int tree_action_rel = 0;
    size_t content_length = 0;
    // check whether it is root path or not
    char *is_not_root = strstr((char *)read_buf, " / ");

    if (is_not_root != NULL)
    {
        final_content = (uint8_t *)"hello from tiny-redis!\n";
        goto DONE;
    }

    if (http_req_parser(read_buf, n, &syn) == -1)
    {
        final_content = (uint8_t *)"http parser error!\n";
        goto DONE;
    }

    switch (syn.action)
    {
    case sat_CREATE:
        log_msg_debug("dealing with create request");
        switch (put_into_tree(&syn))
        {
        case 0:
            final_content = (uint8_t *)"create done!\n";
            goto DONE;
            break;
        default:
            final_content = (uint8_t *)"something is wrong in `put_into_tree`!\n";
            goto DONE;
            break;
        }
        break;
    case sat_DELETE:
        log_msg_debug("dealing with delete request");
        tree_action_rel = delete_in_tree(&syn);
        switch (tree_action_rel)
        {
        case 0:
            final_content = (uint8_t *)"removed!\n";
            goto DONE;
            break;
        case -2:
            final_content = (uint8_t *)"no content!\n";
            goto DONE;
            break;
        default:
            final_content = (uint8_t *)"something is wrong in `delete_in_tree`!";
            goto DONE;
        }
        break;
    case sat_GET:
        log_msg_debug("dealing with get request");
        tree_action_rel = get_from_tree(&syn, &value_result);
        switch (tree_action_rel)
        {
        case 0:
            final_content = (uint8_t *)value_result->p;
            content_length = value_result->used;
            goto DONE;
            break;
        case -2:
            final_content = (uint8_t *)"no content\n";
            goto DONE;
            break;
        default:
            final_content = (uint8_t *)"something is wrong in `get_from_tree`!";
        }
        break;

    case sat_UPDATE:
        log_msg_debug("dealing with update request");
        break;
    }
DONE:
    free_syntax_block_content(&syn);
    if (content_length != 0)
        return gen_response(send_buf_empty_ptr, final_content, content_length);

    return gen_response(send_buf_empty_ptr, final_content, strlen((char *)final_content));
}

void EV_ACCPET_DONE_handler(struct io_uring_cqe *cqe, struct io_uring *ring, int serverfd,
                            struct sockaddr_in *client_addr, socklen_t *client_len, struct event_info *process_heap_info)
{
    if (cqe->res < 0)
        return;

    int clientfd = cqe->res;

    struct read_buffer *read_buf = malloc(sizeof(struct read_buffer));
    // read_buf 41e700 data 41e720 phi 41e6e0
    read_buf->data = calloc(INIT_BUFFER_UNIT_SIZE, sizeof(char));
    read_buf->len = INIT_BUFFER_UNIT_SIZE;
    read_buf->used = 0;
    new_recv_event(ring, clientfd, read_buf, 0, process_heap_info);

    // add new accept event to server socket fd
    struct event_info *next_accpet_info = malloc(sizeof(struct event_info *));

    new_accept_event(ring, serverfd, (struct sockaddr *)client_addr, client_len, 0, next_accpet_info);
}

void EV_READ_DONE_handler(struct io_uring_cqe *cqe, struct io_uring *ring, struct event_info *process_heap_info)
{
    struct read_buffer *read_buf = process_heap_info->data;
    size_t prev_used = read_buf->used;
    size_t req_len = cqe->res;
    char *err_msg = NULL;
    uint8_t *send_buf = NULL;
    size_t buffer_size = 0;
    size_t body_size, header_size;
    struct write_buffer *write_buf;
    read_buf->used += req_len;

    if (cqe->res <= 0)
    {
        // close by remote host ?
        free(read_buf->data);

        close(process_heap_info->sockfd);
        free(process_heap_info->data);
        free(process_heap_info);
        return;
    }

    // request body size limit
    if (read_buf->used > MAX_RECV_BUFFER_SIZE)
    {
        err_msg = "the request is too large!\n";
        goto SEND_ERROR_MSG;
    }
    // first time we need get the body
    if (prev_used == 0)
    {
        uint8_t *next_line_of_CL = (uint8_t *)Content_Length_in_header((char *)read_buf->data, req_len, &body_size);

        // request headers should less that INIT_BUFFER_UNIT_SIZE
        // get the header or first part of a big payload
        if (req_len < INIT_BUFFER_UNIT_SIZE && next_line_of_CL != NULL)
        {
            // the header contains the Content-Length

            // full request in buffer
            if (body_size < req_len)
                goto READ_ALL;

            // appears when send big file data in curl
            // only header in buffer,realloc
            else
                goto REALLOC;
        }
        // request that without body is too large
        else if (req_len == INIT_BUFFER_UNIT_SIZE && next_line_of_CL == NULL)
        {
            // overflow request without Content-Length
            err_msg = "the request is too large!\n";
            goto SEND_ERROR_MSG;
        }
        // post with small body
        else if (req_len == INIT_BUFFER_UNIT_SIZE && next_line_of_CL != NULL)
            goto REALLOC;
        // other methods without body and less than INIT_BUFFER_UNIT_SIZE
        else /*if (req_len < INIT_BUFFER_UNIT_SIZE && next_line_of_CL == NULL)*/
            goto READ_ALL;
    }
    // other parts,only POST can arrives only once
    else
    {
        // full buffer,read done
        // it's possible that server cannot read all the send byte from client
        if (read_buf->len == read_buf->used)
            goto READ_ALL;
        else
            new_recv_event(ring, process_heap_info->sockfd, read_buf, 0, process_heap_info);
    }

    return;

READ_ALL:
    buffer_size = solver(read_buf->data, read_buf->used, &send_buf);
    // we don't need this read buffer now
    free(read_buf->data);
    free(process_heap_info->data); // read_buf
    write_buf = malloc(sizeof(struct write_buffer));
    write_buf->data = send_buf;
    write_buf->len = buffer_size;
    write_buf->sent = 0;
    new_send_event(ring, process_heap_info->sockfd, write_buf, 0, process_heap_info);

    return;

SEND_ERROR_MSG:
    send_buf = malloc(INIT_BUFFER_UNIT_SIZE * sizeof(uint8_t));
    buffer_size = gen_response(&send_buf, (uint8_t *)err_msg, strlen(err_msg));
    free(read_buf->data);
    free(process_heap_info->data); // read_buf

    write_buf = malloc(sizeof(struct write_buffer));
    write_buf->data = send_buf;
    write_buf->len = buffer_size;
    write_buf->sent = 0;
    new_send_event(ring, process_heap_info->sockfd, write_buf, 0, process_heap_info);
    return;

REALLOC:
    header_size = (uint8_t *)strstr((char *)read_buf->data, "\r\n\r\n") + 4 - read_buf->data;
    read_buf->data = realloc(read_buf->data, body_size + header_size); // larger than required
    read_buf->len = body_size + header_size;
    new_recv_event(ring, process_heap_info->sockfd, read_buf, 0, process_heap_info);
    return;
}

void EV_WRITE_DONE_handler(struct io_uring_cqe *cqe, struct io_uring *ring, struct event_info *process_heap_info)
{
    struct write_buffer *wb = process_heap_info->data;

    if (cqe->res < wb->len - wb->sent)
    {
        // resend the remaining data
        wb->sent += cqe->res;
        new_send_event(ring, process_heap_info->sockfd, wb, 0, process_heap_info);
    }
    else
    {
        close(process_heap_info->sockfd);
        free(((struct write_buffer *)process_heap_info->data)->data);
        free(process_heap_info->data);
        free(process_heap_info);
    }
}

void EV_TIMER_DONE_handler(int loop_timer_fd, struct event_info *process_heap_info, struct io_uring *ring)
{
    flush_timer_when_available(loop_timer_fd);
    void (*func)(void) = process_heap_info->data;
    func();
    new_timer_event(ring, timer_worker, loop_timer_fd, process_heap_info);
}
