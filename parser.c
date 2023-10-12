#include "log.h"
#include "syntax.h"
#include "test_common.h"
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
const size_t AVALIABLE_ACT = 4;

const char *act_str[] = {
    "POST",
    "DELETE",
    "PUT",
    "GET",
};

const size_t AVALIABLE_TYPE = 3;

const char *type_str[] = {
    "STRING",
    "SET",
    "LIST",
};

// return the action enum
int valid_action(char *s)
{
    if (s == NULL)
        return -1;
    for (size_t i = 0; i < AVALIABLE_ACT; i++)
    {
        if (strcmp(act_str[i], s) == 0)
            return i;
    }
    return -1;
}

COM_INNER_DECL int valid_type(char *s)
{
    if (s == NULL)
        return -1;
    for (size_t i = 0; i < AVALIABLE_TYPE; i++)
    {
        if (strcmp(type_str[i], s) == 0)
            return i;
    }
    return -1;
}

// usually, n >= return value
COM_INNER_DECL size_t decode_url(char *url, uint8_t *buf, size_t n)
{
    size_t i = 0, j = 0;
    size_t key_size = n;
    while (i < n)
    {
        if (url[i] == '%')
        {
            if (i + 2 >= n)
                return -1; // wrong decode url format

            char hexchars[3] = {0};
            strncpy(hexchars, url + i + 1, 2);
            // it is not suitable for sscanf %hhx because of serial data
            uint8_t onebyte = (uint8_t)strtoul(hexchars, NULL, 16);
            buf[j] = onebyte;
            i += 2;
        }
        else
        {
            buf[j] = url[i];
            if (url[i] == '?' && key_size == n)
            {
                key_size = j;
            }
        }

        i += 1;
        j += 1;
    }

    return key_size;
}

void free_syntax_block_content(action_syntax_t *syntax_block)
{
    if (syntax_block->key != NULL)
    {
        free(syntax_block->key);
    }
    if (syntax_block->val != NULL)
    {
        free(syntax_block->val);
    }
}

// donot parse query
COM_INNER_DECL int GET_req_parser(char *req, size_t n,
                                  action_syntax_t *syntax_block)
{
    size_t should_skipped_byte = 5; // skip "GET /"
    uint8_t *alloc_key = NULL;
    if (n <= should_skipped_byte)
    {
        log_msg_debug("request is too short");
        goto FAIL;
    }

    char *key_start = req + should_skipped_byte;

    char *key_end = strchr(key_start, '?');

    char *after_URL_whitespace = strchr(key_start, ' ');
    if (after_URL_whitespace == NULL)
    {
        log_msg_debug("wrong format request");
        goto FAIL;
    }

    if (key_end == NULL)
    {
        // no query
        log_msg_debug("get req has not query symbol");
        key_end = after_URL_whitespace;
    }

    // key is allowed to be encoded invisiable byte
    if (key_end - key_start == 0)
    {
        log_msg_debug("key size is 0");
        goto FAIL;
    }
    alloc_key = malloc((key_end - key_start) * sizeof(uint8_t));
    size_t key_size =
        decode_url(key_start, alloc_key, key_end - key_start);
    // make sure correct c-style string
    alloc_key[key_size] = '\0';
    syntax_block->key_size = key_size;
    syntax_block->key = alloc_key;
    syntax_block->val = NULL;
    syntax_block->val_size = 0;
    syntax_block->data_type = 0;
    syntax_block->TTL = 0;

    return 0;

FAIL:
    if (alloc_key != NULL)
    {
        free(alloc_key);
    }
    return -1;
}

// return the end position of Content-Length line(no contains \r\n)
char *Content_Length_in_header(char *req, size_t n,
                               size_t *num)
{
    char *content_length_pos = strstr(req, "Content-Length: ");
    if (content_length_pos == NULL || content_length_pos - req + 16 >= n)
        goto FAIL; // maybe there are not vaild infomation in n range

    content_length_pos += 16;
    char *end_CRLF = strstr(content_length_pos, "\r\n");
    if (end_CRLF == NULL || end_CRLF - req + 2 >= n)
        goto FAIL;

    *num = strtoul(content_length_pos, &end_CRLF, 10);

    if (num == 0)
        goto FAIL;

    return end_CRLF;

FAIL:
    return NULL;
}

// TODO:gtest
COM_INNER_DECL int check_kv_in_query(char *s, action_syntax_t *syntax_block,
                                     size_t n)
{
    char buf[128] = {0};
    char *key_end = strchr(s, '=');
    if (key_end == NULL || key_end - s + 1 >= n)
    {
        return -1;
    }

    if (strncmp("TTL", s, 3) == 0)
    {
        strncpy(buf, key_end + 1, n - (key_end + 1 - s));
        syntax_block->TTL = atol(buf);
    }
    else if (strncmp("TYPE", s, 4) == 0)
    {
        strncpy(buf, key_end + 1, n - (key_end + 1 - s));
        int current_type = valid_type(buf);
        if (current_type == -1)
            return -1;

        syntax_block->data_type = current_type;
    }

    return 0;
}

// TODO:gtest
COM_INNER_DECL int url_query_parser(char *URL_without_start_slash,
                                    action_syntax_t *syntax_block,
                                    size_t URL_without_start_slash_N)
{
    char *start_query = strchr(URL_without_start_slash, '?');
    if (start_query == NULL || start_query - URL_without_start_slash >= URL_without_start_slash_N)
    {
        log_msg_debug("cannot find ?");
        return -2;
    }

    start_query += 1;
    char *end_q1 = strchr(start_query, '&');

    if (end_q1 == NULL || end_q1 - URL_without_start_slash >= URL_without_start_slash_N)
    {
        log_msg_debug("cannot find &");
        end_q1 = URL_without_start_slash + URL_without_start_slash_N;
    }

    syntax_block->TTL = 0;
    syntax_block->data_type = 0;

    if (-1 == check_kv_in_query(start_query, syntax_block, end_q1 - start_query))
    {
        log_msg_debug("not a correct query in the url");
    }

    if (end_q1 == URL_without_start_slash + URL_without_start_slash_N)
    {
        log_msg_debug("only one query");
        return 0;
    }
    char *start_q2 = end_q1 + 1;

    if (-1 == check_kv_in_query(start_q2, syntax_block, URL_without_start_slash + URL_without_start_slash_N - start_q2))
    {
        log_msg_debug("not a correct query in the url");
    }

    return 0;
}

// TODO: for list/set
// TODO:gtest
COM_INNER_DECL int advanced_operator()
{
    return 0;
}

// TODO:gtest
//
// allocate key and value when success
COM_INNER_DECL int content_parser(size_t should_skipped_byte, char *req,
                                  size_t n, action_syntax_t *syntax_block)
{
    uint8_t *alloc_key = NULL;
    uint8_t *alloc_value = NULL;
    if (n <= should_skipped_byte)
    {
        log_msg_debug("request body is too short");
        goto FAIL;
    }

    char *key_ptr = req + should_skipped_byte;

    char *first_whitespace_pos_after_key = strchr(key_ptr, ' ');

    if (first_whitespace_pos_after_key == NULL)
    {
        log_msg_debug("wrong format request");
        goto FAIL;
    }

    // key is allowed to encoded to invisiable byte
    alloc_key = malloc(first_whitespace_pos_after_key - key_ptr);

    int url_query_parser_rel = url_query_parser(
        key_ptr, syntax_block, first_whitespace_pos_after_key - key_ptr);
    if (url_query_parser_rel == -1)
    {
        log_msg_debug("url query parsing error");
        goto FAIL;
    }
    else if (url_query_parser_rel == -2)
    {
        log_msg_debug("no query,use default query");
        syntax_block->data_type = 0;
        syntax_block->TTL = 0;
    }

    // TODO:write adv opt before here
    size_t key_size =
        decode_url(key_ptr, alloc_key, first_whitespace_pos_after_key - key_ptr);

    // read real content length from header: Content-Length
    size_t Content_Length_header;
    char *CL_end_ptr = Content_Length_in_header(req, n, &Content_Length_header);
    if (CL_end_ptr == NULL)
    {
        log_msg_debug("cannot found content length in header");
        goto FAIL;
    }

    char *double_CRLF = strstr(CL_end_ptr, "\r\n\r\n");
    if (double_CRLF == NULL)
    {
        log_msg_debug("cannot found header/body split");
        goto FAIL;
    }

    char *body_start = double_CRLF + 4; // skip \\r\\n\\r\\n

    alloc_value = malloc(Content_Length_header);
    memmove(alloc_value, body_start, Content_Length_header);

    syntax_block->key_size = key_size;
    syntax_block->key = alloc_key;
    syntax_block->val = alloc_value;
    syntax_block->val_size = Content_Length_header;

    return 0;

FAIL:
    if (alloc_key != NULL)
    {
        free(alloc_key);
    }
    if (alloc_value != NULL)
    {
        free(alloc_value);
    }
    return -1;
}
static inline int POST_req_parser_kw(char *req, size_t n,
                                     action_syntax_t *syntax_block)
{
    size_t should_skipped_byte = 6; // skip "POST /"
    return content_parser(should_skipped_byte, req, n, syntax_block);
}

static inline int PUT_req_parser_kw(char *req, size_t n,
                                    action_syntax_t *syntax_block)
{
    size_t should_skipped_byte = 5; // skip "PUT /"
    return content_parser(should_skipped_byte, req, n, syntax_block);
}

// TODO:gtest
COM_INNER_DECL int DELETE_req_parser_kw(char *req, size_t n,
                                        action_syntax_t *syntax_block)
{
    size_t should_skipped_byte = 8; // skip "DELETE /"
    uint8_t *alloc_key = NULL;
    if (n <= should_skipped_byte)
    {
        log_msg_debug("request body is too short");
        goto FAIL;
    }

    char *key_ptr = req + should_skipped_byte;

    char *first_whitespace_pos = strchr(key_ptr, ' ');

    if (first_whitespace_pos == NULL)
    {
        log_msg_debug("wrong format request");
        goto FAIL;
    }

    // key is allowed to be encoded invisiable byte
    alloc_key = malloc(first_whitespace_pos - key_ptr);
    size_t key_size =
        decode_url(key_ptr, alloc_key, first_whitespace_pos - key_ptr);

    syntax_block->key = alloc_key;
    syntax_block->key_size = key_size;
    syntax_block->val = NULL;
    syntax_block->val_size = 0;
    syntax_block->TTL = 0;
    syntax_block->data_type = 0;
    return 0;

FAIL:
    if (alloc_key != NULL)
    {
        free(alloc_key);
    }
    return -1;
}

// TODO:gtest
// heap memory will be used when success, calling free_syntax_block_content to free the used heap memory after used
int http_req_parser(uint8_t *req, size_t n, action_syntax_t *syntax_block)
{
    char *first_whitespace_pos = strchr((const char *)req, ' ');

    if (first_whitespace_pos == NULL || first_whitespace_pos >= (char *)req + n)
    {
        log_msg_debug("not a correct format");
        goto FAIL;
    }

    char act[64] = {0};
    size_t cpy_len = first_whitespace_pos - (char *)req > sizeof(act) - 1 ? sizeof(act) - 1 : first_whitespace_pos - (char *)req;

    strncpy(act, req, cpy_len);

    int act_type = valid_action(act);

    if (act_type == -1)
    {
        log_msg_debug("not a correct action");
        goto FAIL;
    }

    syntax_block->action = act_type;

    switch (syntax_block->action)
    {
    case sat_GET:
        if (GET_req_parser(req, n, syntax_block) == -1)
        {
            goto FAIL;
        }
        break;
    case sat_CREATE:
        if (POST_req_parser_kw(req, n, syntax_block) == -1)
        {
            goto FAIL;
        }
        break;
    case sat_DELETE:
        if (DELETE_req_parser_kw(req, n, syntax_block) == -1)
        {
            goto FAIL;
        }
        break;
    case sat_UPDATE:
        if (PUT_req_parser_kw(req, n, syntax_block) == -1)
        {
            goto FAIL;
        }
        break;
    default:
        break;
    }

    return 0;

FAIL:
    return -1;
}