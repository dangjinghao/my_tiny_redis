.PHONY: run test 
SRCS := $(shell find . -maxdepth 1 -name "*.c")
CFLAGS := -g 
LDLIBS := -luring
CC := gcc
OBJS = $(SRCS:.c=.o)
NAME := tinyredis
all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)  $(LDLIBS) 
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDLIBS) 

debug: all
	gdb ./$(NAME)

run: all
	./$(NAME)

clean:
	rm ./*.o