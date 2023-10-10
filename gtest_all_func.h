#pragma once
#include <stdio.h>
#include <stdint.h>
extern "C"
{
#include "syntax.h"
#include "log.h"
#define COM_INNER_DECL extern

    COM_INNER_DECL int valid_action(char *s);
    COM_INNER_DECL int valid_type(char *s);

    COM_INNER_DECL size_t decode_url(char *url, uint8_t *buf, size_t n);

    COM_INNER_DECL int GET_req_parser(char *req, size_t n, action_syntax_t *syntax_block);

    COM_INNER_DECL char *Content_Length_in_header(char *req, size_t n, size_t *num);
    COM_INNER_DECL int check_kv_in_query(char *s, action_syntax_t *syntax_block,
                                         size_t n);

#undef COM_INNER_DECL
}
