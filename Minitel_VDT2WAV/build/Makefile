CC=gcc

UNAME := $(shell uname)

DEBUG ?= 0

ifeq ($(DEBUG), 1)
	CFLAGS=-O0 $(INCLUDES) -Wall -g -DDEBUG
	LDFLAGS= -lc -lm -ldl -lpthread -lasan
else
	CFLAGS=-O3 $(INCLUDES) -Wall
	LDFLAGS= -lc -lm -ldl -s
endif

ifeq ($(UNAME), Darwin)
CFLAGS += -arch x86_64 -mmacosx-version-min=10.9
LDFLAGS += -arch x86_64 -mmacosx-version-min=10.9
endif

ifeq ($(UNAME), Linux)
LDFLAGS += -Wl,-rpath=.
endif

ifeq ($(findstring CYGWIN,$(shell uname)),CYGWIN)
endif

EXEC=vdt2wav

all: $(EXEC)

vdt2wav:  vdt2wav.o
	$(CC) -o $@    $^ $(LDFLAGS)

vdt2wav.o: ../src/vdt2wav.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf *.so

mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper

