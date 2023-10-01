#include "syntax.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
static int valid_action(char*s){
    for (size_t i = 0; i < AVALIABLE_ACT; i++)
    {
        if(strcmp(act_str[i],s) == 0) return i;
    }
    return -1;
}

static int decode_url(char*url,uint8_t*buf,size_t n){
    size_t i = 0,j = 0;
    while (i < n)
    {
        if(url[i] == '%'){
            if(i+2 >= n) return -1; // wrong decode url format
            uint8_t onebyte = url[i+1] * 16 + url[i+2];

            buf[j] = onebyte;
            i+=2;
            
        }
        else{
            buf[j] = url[i];
        }

        i+=1;
        j+=1;
    }
    
    return 0;
}

static int GET_req_parser(char* req,size_t n,action_syntax_t *syntax_block){
    size_t should_skipped_byte = 5; //skip "GET /"
    uint8_t* alloc_key = NULL;
    if(n <= should_skipped_byte) goto FAIL;
    
    char* key_ptr = req + should_skipped_byte; 

    char* first_whitespace_pos = strchr(key_ptr,' ');

    if(first_whitespace_pos == NULL){
        goto FAIL;
    }
    
    // key is allowed to be encoded invisiable byte
    alloc_key = malloc(first_whitespace_pos - key_ptr);
    if(decode_url(key_ptr,alloc_key,first_whitespace_pos - key_ptr) == -1){
        goto FAIL;
    }
    syntax_block->action = sat_GET;
    syntax_block->kw1 = alloc_key;
    syntax_block->kw2 = NULL;

    return 0;

FAIL:
    if(alloc_key != NULL){
        free(alloc_key);
    } 
    return -1;
}



int http_req_parser(uint8_t*req,size_t n,action_syntax_t syntax_block){
    char* first_whitespace_pos = strchr((const char*)req,' ');

    if(first_whitespace_pos == NULL) 
    {
        // not a correct format
        goto FAIL;
    }
    
    char act[64] = {0};
    size_t cpy_len = first_whitespace_pos - (char*)req > sizeof(act) - 1 ? sizeof(act) - 1 : first_whitespace_pos - (char*)req;

    strncpy(act,req,
        cpy_len);
    
    int act_type = valid_action(act);
    // not a correct action
    if(act_type == -1){
        goto FAIL;
    }

    syntax_block.action = act_type;

    switch (syntax_block.action)
    {
    case sat_GET:
        if(GET_req_parser(req,n,&syntax_block)==-1){
            goto FAIL;

        }
        break;
    // TODO:unimplement

    case sat_CREATE:
        break;
    case sat_DELETE:
        break;
    case sat_UPDATE:
        break;
    default:
        break;
    }

    return 0;
FAIL:

    return -1;
}