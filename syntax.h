#ifndef SYNTAX_H
#define SYNTAX_H
#include <stdlib.h>
#include <stdint.h>

extern const size_t AVALIABLE_ACT;
extern const size_t AVALIABLE_TYPE;
extern const char *act_str[];
extern const char *type_str[];
typedef enum
{
    sat_CREATE = 0,
    sat_DELETE,
    sat_UPDATE,
    sat_GET,
} syntax_action_type;

typedef enum
{
    sdt_STRING = 0,
    sdt_SET,
    sdt_LIST,
} syntax_data_type;

typedef enum
{
    ao_sadd,
    ao_srm,
    ao_sexists,
    ao_llpush,
    ao_lrpush,
    ao_lidxins,
    ao_lidxget,
    ao_lidxrm,
    ao_lidxmod,
} adv_opt_type;

typedef struct action_syntax
{
    syntax_action_type action;
    uint8_t *key;
    size_t key_size;
    uint8_t *val;
    size_t val_size;
    syntax_data_type data_type;
    uint64_t TTL; // 0 is forever
    adv_opt_type adv_opt;
} action_syntax_t;

struct read_buffer
{
    uint8_t *data;
    size_t len;
    size_t used;
};
struct write_buffer
{
    uint8_t *data;
    size_t len;
    size_t sent;
};
#endif