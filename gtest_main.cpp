#include <algorithm>
#include <cassert>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <ranges>

#include "llrbtree.hpp"
#include "gtest_all_func.h"
#include "syntax.h"

int main(int argc, char const *argv[])
{
    set_log_level(LOG_DEBUG);

    std::cout << "valid_action test" << std::endl;

    for (size_t i = 0; i < AVALIABLE_ACT; i++)
    {
        EXPECT_EQ(valid_action((char *)act_str[i]), i);
    }

    EXPECT_EQ(valid_action(""), -1);
    EXPECT_EQ(valid_action(NULL), -1);
    EXPECT_EQ(valid_action("132"), -1);
    EXPECT_EQ(valid_action("PUTT"), -1);
    EXPECT_EQ(valid_action("PU"), -1);
    std::cout << "valid_action test done" << std::endl;

    std::cout << "valid_type test done" << std::endl;

    for (size_t i = 0; i < AVALIABLE_TYPE; i++)
    {
        EXPECT_EQ(valid_action((char *)act_str[i]), i);
    }

    EXPECT_EQ(valid_action(""), -1);
    EXPECT_EQ(valid_action(NULL), -1);
    EXPECT_EQ(valid_action("132"), -1);
    EXPECT_EQ(valid_action("PUTT"), -1);
    EXPECT_EQ(valid_action("PU"), -1);
    EXPECT_EQ(valid_action("fffffffffffffffffffffffffffffffff"), -1);

    std::cout << "valid_type test done" << std::endl;

    std::cout << "decode_url test" << std::endl;
    uint8_t buf[128];

    bzero(buf, 128);
    decode_url("/Hello%20World%21", buf, 17);
    EXPECT_STREQ((char *)buf, "/Hello World!");

    bzero(buf, 128);
    decode_url("/", buf, 1);
    EXPECT_STREQ((char *)buf, "/");

    bzero(buf, 128);
    decode_url("/Caf%C3%A9", buf, 10);
    EXPECT_STREQ((char *)buf, "/Café");

    bzero(buf, 128);
    decode_url("/%3A%2F%3F%23%5B%5D%40%21%24%26%27%28%29%2A%2B%2C%3B%3D", buf, 55);
    EXPECT_STREQ((char *)buf, "/:/?#[]@!$&'()*+,;=");

    bzero(buf, 128);
    decode_url("%5E65", buf, 5);
    EXPECT_STREQ((char *)buf, "^65");
    std::cout << "decode_url test done" << std::endl;

    std::cout << "GET_req_parser_kw test" << std::endl;
    char *res = "GET /key";
    action_syntax_t action_block;
    EXPECT_EQ(GET_req_parser("GET /adf ", 9, &action_block), 0);
    EXPECT_STREQ((char *)action_block.key, "adf");

    EXPECT_EQ(GET_req_parser("GET /key?123456=213rw ", 22, &action_block), 0);
    EXPECT_STREQ((char *)action_block.key, "key");

    EXPECT_EQ(GET_req_parser("GET /?123456=213rw ", 19, &action_block), -1);

    EXPECT_EQ(GET_req_parser("GET /", 5, &action_block), -1);
    EXPECT_EQ(GET_req_parser("GET / ", 6, &action_block), -1);
    EXPECT_EQ(GET_req_parser("GET key", 8, &action_block), -1);
    EXPECT_EQ(GET_req_parser("GET /key", 8, &action_block), -1);
    EXPECT_EQ(GET_req_parser("ASF /ke/sfd", 11, &action_block), -1);
    EXPECT_EQ(GET_req_parser("ASF /ke?fsojfawfi=2131", 22, &action_block), -1);
    EXPECT_EQ(GET_req_parser("ASF", 3, &action_block), -1);

    std::cout << "GET_req_parser_kw test done" << std::endl;

    std::cout << "Content_Length_in_header" << std::endl;

    size_t num;
    EXPECT_EQ(Content_Length_in_header("GET /", 5, &num), nullptr);
    EXPECT_EQ(Content_Length_in_header("GET / ", 6, &num), nullptr);
    EXPECT_EQ(Content_Length_in_header("GET /xxx ", 6, &num), nullptr);

    char *CL_buf = "Get /xxx HTTP/1.1\r\nHost: localhost:xxx\r\nContent-Length: 3\r\nContent-Type: text/html\r\n\r\nasd";

    EXPECT_EQ(Content_Length_in_header(CL_buf, 82, &num), CL_buf + 57);
    EXPECT_EQ(Content_Length_in_header(CL_buf, 47, &num), nullptr);
    EXPECT_EQ(Content_Length_in_header(CL_buf, 58, &num), nullptr);
    EXPECT_EQ(Content_Length_in_header(CL_buf, 89, &num), CL_buf + 57);

    std::cout << "Content_Length_in_header test done" << std::endl;

    std::cout << "check_kv_in_query test start" << std::endl;
    char *kv_s = "TTL=12443&TYPE=STRING";
    action_syntax_t syn_block;
    EXPECT_EQ(check_kv_in_query(kv_s, &syn_block, 0), -1);
    EXPECT_EQ(check_kv_in_query(kv_s, &syn_block, 3), -1);
    EXPECT_EQ(check_kv_in_query(kv_s, &syn_block, 4), -1);
    EXPECT_EQ(check_kv_in_query(kv_s, &syn_block, 5), 0);
    EXPECT_EQ(syn_block.TTL, 1);
    EXPECT_EQ(check_kv_in_query(kv_s + 10, &syn_block, 2), -1);
    EXPECT_EQ(check_kv_in_query(kv_s + 10, &syn_block, 5), -1);
    EXPECT_EQ(check_kv_in_query(kv_s + 10, &syn_block, 11), 0);
    EXPECT_EQ(syn_block.data_type, sdt_STRING);

    std::cout << "check_kv_in_query test done" << std::endl;

    std::cout << "llrbtree test" << std::endl;

    red_black_BST<char, int> rbb;
    char s[] = "43217";
    int v[] = {
        0,
        0,
        0,
        0,
        0,
        0,
    };

    for (auto i : std::views::iota(0, (int)(sizeof(s) - 1)))
    {
        rbb.put(s + i, v + i);
    }
    for (auto i : rbb.keys())
    {
        std::cout << *i << std::endl;
    }
    rbb.clean_all_ref();
    EXPECT_TRUE(rbb.is_empty());

    std::cout << "llrbtree test done" << std::endl;

    return 0;
}
