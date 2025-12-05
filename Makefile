CC = gcc
CFLAGS = -Wall -Wextra -g -pedantic
# Link with pthread (for semaphores) and rt (for shared memory)
LDFLAGS = -pthread -lrt

TARGET = ipc_sync
SRCS = main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
