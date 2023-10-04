#ifndef SYNTAX_H
#define SYNTAX_H
#include <stdlib.h>
#include <stdint.h>

extern const size_t AVALIABLE_ACT;
extern const char*act_str[];
extern const char*type_str[];
typedef enum 
{
    sat_CREATE = 0,
    sat_DELETE,
    sat_UPDATE,
    sat_GET,
}syntax_action_type;

typedef enum 
{
    sdt_STRING = 0,
    sdt_INT, //int64_t
    sdt_COUNT, //uint64_t
    sdt_FLOAT, //double 
} syntax_data_type;

typedef struct action_syntax
{
    syntax_action_type action;
    uint8_t* key;
    uint8_t* val;
    syntax_data_type data_type;
    int64_t TTL; //if ttl is 0,it is forever
} action_syntax_t;

#endif