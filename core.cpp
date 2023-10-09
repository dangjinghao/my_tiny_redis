#include "log.h"
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
    uint8_t p[]; // stdc99
    int64_t operator-(const tiny_string &rhs)
    {
        int rel;
        for (size_t comp = 0; comp < this->used && comp < rhs.used; comp++)
        {
            if ((rel = this->p[comp] - rhs.p[comp]) != 0) return rel;
        }
        if (rhs.used > this->used)
            return -1;
        else if (rhs.used < this->used)
            return 1;

        return 0;
    }
    // create heap memory,do not forget to calling remove to free heap memeory
    static tiny_string *create(uint8_t *s, size_t arr_size)
    {
        int cap = 48;
        while (arr_size > cap)
        {
            cap *= 1.5;
        }
        tiny_string *o = (tiny_string *)malloc(sizeof(tiny_string) + cap);
        o->cap = cap;
        o->used = arr_size;
        memcpy(o->p, s, arr_size);
        return o;
    }
    static void remove(tiny_string *s)
    {
        free(s);
    }
    static int reassign(tiny_string **ts, uint8_t *s, size_t n)
    {
        bool should_realloc = false;
        if (n > (*ts)->cap)
        {
            while (n > (*ts)->cap)
            {
                (*ts)->cap *= 1.5;
            }

            size_t realloc_size = (*ts)->cap;
            tiny_string *new_ptr = (tiny_string *)realloc(*ts, realloc_size + sizeof(struct tiny_string));
            if (new_ptr == NULL)
            {
                return -1;
            }

            *ts = new_ptr;
        }

        memcpy((*ts)->p, s, sizeof(uint8_t) * n);
        (*ts)->used = n;
        return 0;
    }
};

struct trds_Object
{
    syntax_data_type type;
    void *ptr;
    trds_Object(syntax_data_type type, void *ptr) :
        type(type), ptr(ptr)
    {
    }
};

static red_black_BST<tiny_string, trds_Object> global_tree;

struct
{
    size_t key_max = 1024;
    size_t val_max = 512 * 1024;

} core_config;

extern "C"
{
    // int init_core()
    // {
    // }

    // copy data from syn_block, you can free the syn block memory safety after calling this function
    int put_into_tree(action_syntax_t *syn_block)
    {
        auto key_size = syn_block->key_size;
        auto val_size = syn_block->val_size;
        tiny_string *ts_val = nullptr, *ts_key = nullptr;
        trds_Object *val_warp = nullptr, *exists_val_warp = nullptr;

        if (key_size >= core_config.key_max || val_size >= core_config.val_max)
        {
            goto FAIL;
        }

        ts_key = tiny_string::create(syn_block->key, key_size);
        // until now, it only supports string value.
        // exists
        if ((exists_val_warp = global_tree.get(ts_key)) != nullptr)
        {
            tiny_string *exists_ts_val = (tiny_string *)exists_val_warp->ptr;
            if (tiny_string::reassign(&exists_ts_val, syn_block->val, val_size) == -1)
            {
                goto FAIL;
            }
            // put reallocted ptr
            exists_val_warp->ptr = exists_ts_val;
        }
        else
        {
            ts_val = tiny_string::create(syn_block->val, val_size);
            val_warp = new trds_Object(sdt_STRING, (void *)ts_val);

            global_tree.put(ts_key, val_warp);
        }
        return 0;
    FAIL:
        if (ts_key != nullptr)
            tiny_string::remove(ts_key);
        if (ts_val != nullptr)
            tiny_string::remove(ts_val);
        if (val_warp != nullptr)
            delete val_warp;
        return -1;
    }
    // get the exists val,no allocate action
    int get_from_tree(action_syntax_t *syn_block, struct tiny_string_raw **tiny_str_ref)
    {
        size_t key_size = syn_block->key_size;
        if (key_size >= core_config.key_max)
        {
            return -1;
        }
        auto ts_key = tiny_string::create(syn_block->key, key_size);
        auto rel_val = global_tree.get(ts_key);
        tiny_string::remove(ts_key);
        if (rel_val == nullptr)
        {
            return -2;
        }

        assert(rel_val->type == sdt_STRING); // until now, it only supports string value.
        *tiny_str_ref = (tiny_string_raw *)rel_val->ptr;

        return 0;
    }
}
