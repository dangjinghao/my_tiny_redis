.PHONY: all test debug clean 

NAME := tinyredis
SRCS := $(shell find . -maxdepth 1 -name "*.c")
CXXSRCS := $(shell find . -maxdepth 1 -name "*.cpp")
CFLAGS := -g 

CXXFLAGS := $(CFLAGS)
CXXFLAGS += -Wno-write-strings

LDLIBS := -luring
TESTLDLIBS := -libgtest

CC := gcc
CXX := g++

TEST_OBJS := $(patsubst ./%.c,test/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst ./%.c,debug/%.o,$(SRCS))

all: debug test

debug: $(DEBUG_OBJS)
	$(CC) $(CFLAGS) $(DEBUG_OBJS) -o debug/$(NAME)  $(LDLIBS) 

debug/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDLIBS) 

test/%.o: %.c
	$(CC) $(CFLAGS) -DTEST -c $< -o $@ $(LDLIBS) 


test: $(TEST_OBJS) 
	$(CXX) $(CXXSRCS) $(CXXFLAGS) -DTEST $(TEST_OBJS) -o test/gtest_$(NAME) $(LDLIBS) $(TESTLDLIBS)
	test/gtest_tinyredis
clean:
	rm -f test/* debug/*