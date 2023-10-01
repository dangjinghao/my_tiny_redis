#pragma once
#include <stdlib.h>
enum syntax_action_type
{
    sat_CREATE = 0,
    sat_DELETE,
    sat_UPDATE,
    sat_GET,
};

typedef struct action_syntax
{
    enum syntax_action_type action;
    uint8_t* kw1;
    uint8_t* kw2;
} action_syntax_t;

char* act_str[] = {
    "POST",
    "DELETE",
    "PUT",
    "GET"
    };

const size_t AVALIABLE_ACT = 4;