CC ?= cc
CFLAGS += -O2 -Wall
INCLUDES = -I.

SRC = debug.c parser.c lexer.c array.c memory.c vm.c types.c selene.c compile.c chunk.c slnlib.c
OBJ = $(SRC:.c=.o)
TARGET = selene

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJ) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm $(OBJS)
	rm $(TARGET)
