#include <iostream>
#include <gtest/gtest.h>

#include "gtest_all_func.h"

int main(int argc, char const *argv[])
{
    std::cout<<"valid_action test"<<std::endl;

    for (size_t i = 0; i < AVALIABLE_ACT; i++)
    {
        EXPECT_EQ(valid_action((char*)act_str[i]),i);
    }
    
    EXPECT_EQ(valid_action(""),-1);
    EXPECT_EQ(valid_action(NULL),-1);
    EXPECT_EQ(valid_action("132"),-1);
    EXPECT_EQ(valid_action("PUTT"),-1);
    EXPECT_EQ(valid_action("PU"),-1);
    std::cout<<"valid_action test done"<<std::endl;


    std::cout<<"valid_type test done"<<std::endl;

    for (size_t i = 0; i < AVALIABLE_TYPE; i++)
    {
        EXPECT_EQ(valid_action((char*)act_str[i]),i);
    }

    EXPECT_EQ(valid_action(""),-1);
    EXPECT_EQ(valid_action(NULL),-1);
    EXPECT_EQ(valid_action("132"),-1);
    EXPECT_EQ(valid_action("PUTT"),-1);
    EXPECT_EQ(valid_action("PU"),-1);
    EXPECT_EQ(valid_action("fffffffffffffffffffffffffffffffff"),-1);

    std::cout<<"valid_type test done"<<std::endl;
    
    

    return 0;
}
