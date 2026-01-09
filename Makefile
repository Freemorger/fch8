CC = gcc
CC = gcc
CFLAGS = -Wall -Wextra
TARGET = fch8

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

RAYLIB_FLAGS = -lraylib -lm

all: CFLAGS += -O2
all: $(TARGET)
	@true 

debug: CFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)
	@true


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(RAYLIB_FLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all debug clean
