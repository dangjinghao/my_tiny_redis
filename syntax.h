#ifndef SYNTAX_H
#define SYNTAX_H
#include <stdlib.h>
#include <stdint.h>

extern const size_t AVALIABLE_ACT;
extern const char*act_str[];

typedef enum 
{
    sat_CREATE = 0,
    sat_DELETE,
    sat_UPDATE,
    sat_GET,
}syntax_action_type;

typedef struct action_syntax
{
    syntax_action_type action;
    uint8_t* kw1;
    uint8_t* kw2;
} action_syntax_t;

#endif