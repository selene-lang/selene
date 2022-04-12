CC ?= cc
CFLAGS += -O2 -Wall

SRC = debug.c parser.c lexer.c array.c memory.c vm.c types.c selene.c compile.c chunk.c slnlib.c
OBJ = $(SRC:.c=.o)
TARGET = selene

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(OBJ)
	rm $(TARGET)
