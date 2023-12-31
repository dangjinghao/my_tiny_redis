#ifndef PARSER_H
#define PARSER_H
#include "syntax.h"
#include <stdlib.h>
#include <stdint.h>

void free_syntax_block_content(action_syntax_t *syntax_block);

int http_req_parser(uint8_t *req, size_t n, action_syntax_t *syntax_block);
int valid_action(char *s);
char *Content_Length_in_header(char *req, size_t n,
                                              size_t *num);

#endif