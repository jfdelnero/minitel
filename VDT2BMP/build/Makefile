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

EXEC=vdt2bmp

all: $(EXEC)

vdt2bmp:  vdt2bmp.o videotex.o bmp_file.o cache.o modem.o teletel_font.o ef9345_font.o
	$(CC) -o $@    $^ $(LDFLAGS)

vdt2bmp.o: ../src/vdt2bmp.c
	$(CC) -o $@ -c $< $(CFLAGS)

videotex.o: ../src/videotex.c
	$(CC) -o $@ -c $< $(CFLAGS)

bmp_file.o: ../src/bmp_file.c
	$(CC) -o $@ -c $< $(CFLAGS)

cache.o: ../src/cache.c
	$(CC) -o $@ -c $< $(CFLAGS)

modem.o: ../src/modem.c
	$(CC) -o $@ -c $< $(CFLAGS)

teletel_font.o: ../src/teletel_font.c
	$(CC) -o $@ -c $< $(CFLAGS)

ef9345_font.o: ../src/ef9345_font.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf *.so

mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper

