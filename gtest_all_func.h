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
COM_INNER_DECL int GET_req_parser_kw(char *req, size_t n, action_syntax_t *syntax_block);

COM_INNER_DECL int GET_req_parser_kw(char *req, size_t n, action_syntax_t *syntax_block);


#undef COM_INNER_DECL
}

