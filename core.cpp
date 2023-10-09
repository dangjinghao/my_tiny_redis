#include "test_common.h"
#include "llrbtree.hpp"
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "syntax.h"
#include "core.h"

struct tiny_string
{
    size_t cap = 48;
    size_t used = 0;
    int8_t p[]; //stdc99 
    int64_t operator-(const tiny_string &rhs)
    {
        int rel;
        for (size_t comp = 0; comp < this->used && comp < rhs.used; comp++)
        {
            if ((rel = this->p[comp] - rhs.p[comp]) != 0) return rel;
        }
        return 0;
    }
    // create heap memory,do not forget to calling remove to free heap memeory 
    static tiny_string* create(uint8_t *s,size_t arr_size)
    {
        int cap = 48;
        while (arr_size > cap) {
            cap *=1.5;
        }
        tiny_string* o = (tiny_string*)malloc(sizeof(tiny_string) + cap);
        o->cap = cap;
        o->used = arr_size;
        memcpy(o->p,s,arr_size);
        return o;
    }
    static void remove(tiny_string*s)
    {
        free(s);
    }
};

struct trds_Object
{
    syntax_data_type type;
    void *ptr;
    trds_Object(syntax_data_type type,void*ptr):type(type),ptr(ptr){}
};


static red_black_BST<tiny_string, trds_Object> global_tree;

struct {
    size_t key_max = 1024;
    size_t val_max = 512 * 1024;

}core_config;

extern "C"
{
    // int init_core()
    // {
    // }
    
    //copy data from syn_block, you can free the syn block memory safety
    int put_into_tree(action_syntax_t* syn_block)
    {

        auto key_size = syn_block->key_size;
        auto val_size = syn_block->val_size;
        if(key_size >= core_config.key_max || val_size >= core_config.val_max)
        {
            return -1;
        }

        auto ts_val = tiny_string::create(syn_block->val,val_size);
        auto ts_key = tiny_string::create(syn_block->key,key_size);        
        auto val_warp = new trds_Object(sdt_STRING,(void*)ts_val);
        // until now, it only supports string value.

        global_tree.put(ts_key,val_warp);
        
        // donot free them
        // tiny_string::remove(ts_key);
        // tiny_string::remove(ts_val);
        // delete val_warp;

        return 0;

    
    }

    int get_from_tree(action_syntax_t*syn_block,struct tiny_string_raw** tiny_str_ref)
    {
        size_t key_size = syn_block->key_size;
        if(key_size >= core_config.key_max)
        {
            return -1;
        }
        auto ts_key = tiny_string::create(syn_block->key,key_size);        
        auto rel_val = global_tree.get(ts_key);
        assert(rel_val->type == sdt_STRING); // until now, it only supports string value.
        *tiny_str_ref = (tiny_string_raw*)rel_val->ptr;

        tiny_string::remove(ts_key);
        return 0;
    }
}


