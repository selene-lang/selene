CC ?= cc
CFLAGS += -O2 -Wall
INCLUDES = -I.
SRCS = debug.c parser.c lexer.c array.c memory.c vm.c types.c selene.c compile.c
OBJS = $(SRCS:.c=.o)
TARGET = selene

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm $(OBJS)
	rm $(TARGET)
