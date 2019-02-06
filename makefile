CC=gcc
RM=rm -rf
MKDIR=mkdir -p

BIN=netplex
SRC=$(wildcard src/*.c)
OBJ=$(subst src/,bin/,$(SRC:.c=.o))

.SILENT:
.PHONY: all clean new

all: $(BIN)

clean:
	echo cleaning
	$(RM) bin $(BIN)

new: clean all

$(BIN): $(OBJ)
	echo linking $(BIN)
	$(CC) $(OBJ) -o$(BIN)

bin/%.o: src/%.c
	echo compiling $<
	$(MKDIR) bin
	$(CC) -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_DEFAULT_SOURCE -Wall -Wextra -Wno-long-long -c -O2 $< -o$@
