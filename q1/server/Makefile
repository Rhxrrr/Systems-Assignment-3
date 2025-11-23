CC     = gcc
CFLAGS = -Wall -Wextra -pthread -g

TARGET = server
SRCS   = server.c client_handler.c
OBJS   = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c to .o
%.o: %.c client_handler.h
	$(CC) $(CFLAGS) -c $<

run: $(TARGET)
	./$(TARGET) 5000

clean:
	rm -f $(TARGET) $(OBJS)
