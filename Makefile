CC=gcc
INC_PATH=-I./inc
SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst src/%.c, obj/%.o, $(SOURCES))
CFLAGS=$(INC_PATH) -Wall -O2
LIBS=-lm -lpthread
EXECUTABLE=bin/main

all:	build $(EXECUTABLE)

$(EXECUTABLE):  $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LIBS)

$(OBJECTS): obj/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< $(LIBS) -o $@

build:
	@mkdir -p bin
	@mkdir -p obj

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) 