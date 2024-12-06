SRCS = $(wildcard *.c *.s *.cpp)
OBJS = $(SRCS:.c = .o .cpp = .o .s = .o)
CC = arm-linux-gnueabihf-g++
#INCLUDES = -I/×××
#LIBS = -L/×××
CCFLAGS = -g -Wall -O0 -lc  -static
monolith_ljg : $(OBJS)
	$(CC) -static $^ -o $@
%.o : %.c %.s %.cpp
	$(CC) -c $< $(CCFLAGS)
clean:
	rm -f ./monolith_ljg
.PHONY:clean
