CC=gcc

UNAME := $(shell uname)

DEBUG ?= 0

ifeq ($(DEBUG), 1)
	CFLAGS=-O0 $(INCLUDES) -Wall -g
	LDFLAGS= -lc -lm -ldl -lpthread -lasan
else
	CFLAGS=-O3 $(INCLUDES) -Wall
	LDFLAGS= -lc -lm -ldl -s
endif

ifeq ($(SDL_SUPPORT), 1)
	CFLAGS += -DSDL_SUPPORT
	LDFLAGS += -lSDL2
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

ifeq ($(EMSCRIPTEN), 1)
	CC=emcc
	CFLAGS=-O3 $(INCLUDES) -Wall -DSDL_SUPPORT -DEMSCRIPTEN_SUPPORT
	LDFLAGS_WASM=-s EXPORTED_FUNCTIONS="['_malloc','_free','_main']" -s BINARYEN=1 -s BINARYEN_METHOD='native-wasm' -s USE_SDL=1  --preload-file test.vdt
	LDFLAGS=-s EXPORTED_FUNCTIONS="['_malloc','_free','_main']" -s WASM=0 -s LEGACY_VM_SUPPORT=1 --closure=0 -s USE_SDL=1  --preload-file test.vdt
	EXEC=vdt2bmp_asmjs.js vdt2bmp_wasm.js
endif

all: $(EXEC)

vdt2bmp:  vdt2bmp.o videotex.o bmp_file.o cache.o modem.o teletel_font.o ef9345_font.o FIR_V22_Minitel.o
	$(CC) -o $@    $^ $(LDFLAGS)

vdt2bmp_asmjs.js:  vdt2bmp.o videotex.o bmp_file.o cache.o modem.o teletel_font.o ef9345_font.o FIR_V22_Minitel.o
	$(CC) -o $@    $^ $(LDFLAGS)

vdt2bmp_wasm.js:  vdt2bmp.o videotex.o bmp_file.o cache.o modem.o teletel_font.o ef9345_font.o FIR_V22_Minitel.o
	$(CC) -o $@    $^ $(LDFLAGS_WASM)

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

FIR_V22_Minitel.o: ../src/FIR/FIR_V22_Minitel.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf *.so
	rm -rf *.js *.wasm

mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper

