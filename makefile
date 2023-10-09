.PHONY: all test debug clean 

NAME := tinyredis
CSRCS := $(shell find . -maxdepth 1 -name "*.c")
CXXSRCS := $(shell find . -maxdepth 1 -name "*.cpp" -not -name "gtest*")
ALLSRCS := $(CSRCS) $(CXXSRCS)

DEBUGFLAGS := -g 

TESTFLAGS := $(DEBUGFLAGS)
TESTFLAGS += -Wno-write-strings -DTEST

LDLIBS := -luring

TESTLDLIBS := $(LDLIBS)
TESTLDLIBS += -lgtest

CC := gcc
CXX := g++

TEST_OBJS := $(patsubst ./%.c,test/%.o,$(patsubst ./%.cpp,test/%.o,$(shell echo $(ALLSRCS) ./gtest_main.cpp)))
DEBUG_OBJS := $(patsubst ./%.c,debug/%.o,$(patsubst ./%.cpp,debug/%.o,$(ALLSRCS)))

all: debug test

debug/%.o: %.c 
	$(CC) $(DEBUGFLAGS) -c $< -o $@ $(LDLIBS) 
debug/%.o: %.cpp
	$(CXX) $(DEBUGFLAGS) -c $< -o $@
debug: $(DEBUG_OBJS)
	$(CXX) $(DEBUGFLAGS) $(DEBUG_OBJS) -o debug/$(NAME)  $(LDLIBS) 
run-debug:debug
	gdb ./debug/$(NAME)


test/%.o: %.c 
	$(CC) $(TESTFLAGS) -c $< -o $@ 
test/%.o: %.cpp
	$(CXX) $(TESTFLAGS) -c $< -o $@

test: $(TEST_OBJS) 
	$(CXX) $(TEST_OBJS) $(TESTFLAGS)  -o test/gtest_$(NAME) $(TESTLDLIBS)
run-test:test
	./test/gtest_$(NAME)


clean:
	rm -f test/* debug/*