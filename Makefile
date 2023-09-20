CC=gcc
INC_PATH=-I./inc
SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst src/%.c, obj/%.o, $(SOURCES))
CFLAGS=-DDEBUG_STARTUP $(INC_PATH)
LIBS=-lm
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