#include "syntax.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

static int valid_action(char *s)
{
    for (size_t i = 0; i < AVALIABLE_ACT; i++)
    {
        if (strcmp(act_str[i], s) == 0)
            return i;
    }
    return -1;
}

static int decode_url(char *url, uint8_t *buf, size_t n)
{
    size_t i = 0, j = 0;
    while (i < n)
    {
        if (url[i] == '%')
        {
            if (i + 2 >= n)
                return -1; // wrong decode url format
            uint8_t onebyte = url[i + 1] * 16 + url[i + 2];

            buf[j] = onebyte;
            i += 2;
        }
        else
        {
            buf[j] = url[i];
        }

        i += 1;
        j += 1;
    }

    return 0;
}

void free_syntax_block_content(action_syntax_t *syntax_block){
    if(syntax_block->kw1!=NULL){
        free(syntax_block->kw1);
    }
    if(syntax_block->kw2!=NULL){
        free(syntax_block->kw2);
    }
    
}
static int GET_req_parser_kw(char *req, size_t n, action_syntax_t *syntax_block)
{
    size_t should_skipped_byte = 5; // skip "GET /"
    uint8_t *alloc_key = NULL;
    if (n <= should_skipped_byte)
        goto FAIL;

    char *key_ptr = req + should_skipped_byte;

    char *first_whitespace_pos = strchr(key_ptr, ' ');

    if (first_whitespace_pos == NULL)
    {
        goto FAIL;
    }

    // key is allowed to be encoded invisiable byte
    alloc_key = malloc(first_whitespace_pos - key_ptr);
    if (decode_url(key_ptr, alloc_key, first_whitespace_pos - key_ptr) == -1)
    {
        goto FAIL;
    }
    syntax_block->kw1 = alloc_key;
    syntax_block->kw2 = NULL;

    return 0;

FAIL:
    if (alloc_key != NULL)
    {
        free(alloc_key);
    }
    return -1;
}

static char* Content_Length_in_header(char*req,size_t n,ssize_t*num){
    char* content_length_pos = strstr(req,"Content-Length: ");
    if(content_length_pos == NULL) goto FAIL;
    content_length_pos += 16;
    char*end_CRLF = strstr(content_length_pos,"\r\n");
    if(end_CRLF == NULL) goto FAIL;
    *num = strtoul(content_length_pos,end_CRLF,10);

    if(num == 0) goto FAIL;

    return end_CRLF;
    
FAIL:
    return NULL;
}

//TODO:TTL should be support
static int POST_req_parser_kw(char*req,size_t n,action_syntax_t* syntax_block)
{
    size_t should_skipped_byte = 6; // skip "POST /"
    uint8_t *alloc_key = NULL;
    uint8_t *alloc_value = NULL;
    if (n <= should_skipped_byte)
        goto FAIL;

    char *key_ptr = req + should_skipped_byte;

    char *first_whitespace_pos_after_key = strchr(key_ptr, ' ');

    if (first_whitespace_pos_after_key == NULL)
    {
        goto FAIL;
    }

    // key is allowed to be encoded invisiable byte
    alloc_key = malloc(first_whitespace_pos_after_key - key_ptr);
    if (decode_url(key_ptr, alloc_key, first_whitespace_pos_after_key - key_ptr) == -1)
    {
        goto FAIL;
    }
    
    // read real content length from header: Content-Length
    ssize_t Content_Length_header;
    char*newptr = Content_Length_in_header(req,n,&Content_Length_header);
    
    char*double_CRLF = strstr(newptr,"\r\n\r\n");
    if(double_CRLF == NULL){
        goto FAIL;
    }

    if(Content_Length_header == -1) goto FAIL;

    char*body_start = double_CRLF + 4;// skip \\r\\n\\r\\n
    size_t real_content_length = n - (req - body_start);
    if(Content_Length_header != real_content_length)
    {
        goto FAIL;
    }
    
    alloc_value = malloc(real_content_length);
    memcpy(alloc_value,body_start,real_content_length);
    syntax_block->kw1 = alloc_key;
    syntax_block->kw2 = alloc_value;

    return 0;

    FAIL:
    if (alloc_key != NULL)
    {
        free(alloc_key);
    }
    if(alloc_value != NULL)
    {
        free(alloc_value);
    }
    return -1;
}

//TODO:TTL should be support
static int PUT_req_parser_kw(char*req,size_t n,action_syntax_t* syntax_block)
{
    size_t should_skipped_byte = 5; // skip "PUT /"
    uint8_t *alloc_key = NULL;
    uint8_t *alloc_value = NULL;
    if (n <= should_skipped_byte)
        goto FAIL;

    char *key_ptr = req + should_skipped_byte;

    char *first_whitespace_pos_after_key = strchr(key_ptr, ' ');

    if (first_whitespace_pos_after_key == NULL)
    {
        goto FAIL;
    }

    // key is allowed to be encoded invisiable byte
    alloc_key = malloc(first_whitespace_pos_after_key - key_ptr);
    if (decode_url(key_ptr, alloc_key, first_whitespace_pos_after_key - key_ptr) == -1)
    {
        goto FAIL;
    }
    
    // read real content length from header: Content-Length
    ssize_t Content_Length_header;
    char*newptr = Content_Length_in_header(req,n,&Content_Length_header);
    
    char*double_CRLF = strstr(newptr,"\r\n\r\n");
    if(double_CRLF == NULL){
        goto FAIL;
    }

    if(Content_Length_header == -1) goto FAIL;

    char*body_start = double_CRLF + 4;// skip \\r\\n\\r\\n
    size_t real_content_length = n - (req - body_start);
    if(Content_Length_header != real_content_length)
    {
        goto FAIL;
    }
    
    alloc_value = malloc(real_content_length);
    memcpy(alloc_value,body_start,real_content_length);
    syntax_block->kw1 = alloc_key;
    syntax_block->kw2 = alloc_value;

    return 0;

    FAIL:
    if (alloc_key != NULL)
    {
        free(alloc_key);
    }
    if(alloc_value != NULL)
    {
        free(alloc_value);
    }
    return -1;
}


static int DELETE_req_parser_kw(char* req,size_t n,action_syntax_t*syntax_block)
{
    size_t should_skipped_byte = 8; // skip "DELETE /"
    uint8_t *alloc_key = NULL;
    if (n <= should_skipped_byte)
        goto FAIL;

    char *key_ptr = req + should_skipped_byte;

    char *first_whitespace_pos = strchr(key_ptr, ' ');

    if (first_whitespace_pos == NULL)
    {
        goto FAIL;
    }

    // key is allowed to be encoded invisiable byte
    alloc_key = malloc(first_whitespace_pos - key_ptr);
    if (decode_url(key_ptr, alloc_key, first_whitespace_pos - key_ptr) == -1)
    {
        goto FAIL;
    }
    syntax_block->kw1 = alloc_key;
    syntax_block->kw2 = NULL;

    return 0;

FAIL:
    if (alloc_key != NULL)
    {
        free(alloc_key);
    }
    return -1;
}

int http_req_parser(uint8_t *req, size_t n, action_syntax_t syntax_block)
{
    char *first_whitespace_pos = strchr((const char *)req, ' ');

    if (first_whitespace_pos == NULL)
    {
        // not a correct format
        goto FAIL;
    }

    char act[64] = {0};
    size_t cpy_len = first_whitespace_pos - (char *)req > sizeof(act) - 1 ? sizeof(act) - 1 : first_whitespace_pos - (char *)req;

    strncpy(act, req,
            cpy_len);

    int act_type = valid_action(act);
    // not a correct action
    if (act_type == -1)
    {
        goto FAIL;
    }

    syntax_block.action = act_type;

    switch (syntax_block.action)
    {
    case sat_GET:
        if (GET_req_parser_kw(req, n, &syntax_block) == -1)
        {
            goto FAIL;
        }
        break;
    case sat_CREATE:
        if(POST_req_parser_kw(req,n,&syntax_block) == -1) 
        {
            goto FAIL;
        }
        break;
    case sat_DELETE:
        if(DELETE_req_parser_kw(req,n,&syntax_block) == -1)
        {
            goto FAIL;
        }
        break;
    case sat_UPDATE:
        if(PUT_req_parser_kw(req,n,&syntax_block) == -1)
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