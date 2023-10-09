#pragma once
#include "syntax.h"

#ifdef __cplusplus
extern "C" {
#endif
struct tiny_string_raw
{
    size_t cap;
    size_t used;
    uint8_t p[]; // stdc99
};
int put_into_tree(action_syntax_t *syn_block);
int get_from_tree(action_syntax_t*syn_block,struct tiny_string_raw** tiny_str_ref);
int delete_in_tree(action_syntax_t *syn_block);
#ifdef __cplusplus
}
#endif