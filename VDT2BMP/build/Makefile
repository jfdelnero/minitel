CC=gcc

UNAME := $(shell uname)

DEBUG ?= 0

ifeq ($(DEBUG), 1)
	CFLAGS=-O0 $(INCLUDES) -Wall -g
	LDFLAGS= -lm -lpthread -lasan
else
	CFLAGS=-O3 $(INCLUDES) -Wall
	LDFLAGS= -lm -s
endif

EXEC=vdt2bmp

ifeq ($(TARGET),)
	ifeq ($(UNAME), Darwin)
		TARGET = Darwin
	endif

	ifeq ($(UNAME), Linux)
		TARGET = Linux
	endif
endif

ifeq ($(TARGET), Linux)
	CFLAGS += -Wl,-Map,foo.map
	LDFLAGS += -Wl,-rpath=. -lc -lm -ldl
endif

ifeq ($(TARGET), mingw32)
	CC=i686-w64-mingw32-gcc
	RESC=i686-w64-mingw32-windres
	LDFLAGS += -static-libgcc -lpthread -Wl,-Map=output.map --static
	CFLAGS += -I./SDL2-2.28.4/i686-w64-mingw32/include/
	EXEC=vdt2bmp.exe
endif

ifeq ($(TARGET), mingw64)
	CC=x86_64-w64-mingw32-gcc
	RESC=x86_64-w64-mingw32-windres
	LDFLAGS += -static-libgcc -lpthread -Wl,-Map=output.map --static
	CFLAGS += -I./SDL2-2.28.4/x86_64-w64-mingw32/include/
	EXEC=vdt2bmp.exe
endif

ifeq ($(findstring CYGWIN,$(shell uname)),CYGWIN)
	LDFLAGS= -shared -lc -lm  -ldl
	EXEC=vdt2bmp.exe
endif

ifeq ($(TARGET), Darwin)
	MACOSX_ARCH ?= -arch arm64 -arch x86_64
	MACOSX_MIN_VER ?= 10.9
	CFLAGS += ${MACOSX_ARCH} -mmacosx-version-min=${MACOSX_MIN_VER}
	LDFLAGS += -lc -lm -ldl ${MACOSX_ARCH} -current_version 2.0 -install_name @executable_path/vdt2bmp -mmacosx-version-min=${MACOSX_MIN_VER}
	EXEC=vdt2bmp
endif

ifeq ($(SDL_SUPPORT), 1)
	CFLAGS += -DSDL_SUPPORT

ifeq ($(TARGET), mingw32)
	LDFLAGS +=  -lmingw32 ./SDL2-2.28.4/i686-w64-mingw32/lib/libSDL2main.a ./SDL2-2.28.4/i686-w64-mingw32/lib/libSDL2.dll.a 
else ifeq ($(TARGET), mingw64)
	LDFLAGS +=  -lmingw32 ./SDL2-2.28.4/x86_64-w64-mingw32/lib/libSDL2main.a ./SDL2-2.28.4/x86_64-w64-mingw32/lib/libSDL2.dll.a
else
	LDFLAGS += -lSDL2
endif

endif

ifeq ($(TARGET), EMSCRIPTEN)
	CC=emcc
	CFLAGS=-O2 $(INCLUDES) -Wall -DSDL_SUPPORT -DEMSCRIPTEN_SUPPORT -s USE_SDL=2 -pthread
	LDFLAGS_WASM=-s EXPORTED_FUNCTIONS="['_malloc','_free','_main']" -s ASYNCIFY -s BINARYEN=1 -s BINARYEN_METHOD='native-wasm' -s USE_SDL=2 -pthread -sPTHREAD_POOL_SIZE=8 -sPROXY_TO_PTHREAD
	LDFLAGS=-s EXPORTED_FUNCTIONS="['_malloc','_free','_main']" -s ASYNCIFY -s WASM=0 -s LEGACY_VM_SUPPORT=1 -s USE_SDL=2 -pthread -sPTHREAD_POOL_SIZE=8 -sPROXY_TO_PTHREAD
	EXEC=vdt2bmp_asmjs.js vdt2bmp_wasm.js
endif

all: $(EXEC)

$(EXEC):  vdt2bmp.o videotex.o script_exec.o env.o bmp_file.o cache.o modem.o teletel_font.o ef9345_font.o FIR_V22_Minitel.o band_pass_rx_Filter.o low_pass_tx_Filter.o vdt_test_001.o vdt_test_002.o vdt_test_003.o
	$(CC) -o $@    $^ $(LDFLAGS)

vdt2bmp_asmjs.js:  vdt2bmp.o videotex.o script_exec.o env.o bmp_file.o cache.o modem.o teletel_font.o ef9345_font.o FIR_V22_Minitel.o band_pass_rx_Filter.o low_pass_tx_Filter.o vdt_test_001.o vdt_test_002.o vdt_test_003.o
	$(CC) -o $@    $^ $(LDFLAGS)

vdt2bmp_wasm.js:  vdt2bmp.o videotex.o script_exec.o env.o bmp_file.o cache.o modem.o teletel_font.o ef9345_font.o FIR_V22_Minitel.o band_pass_rx_Filter.o low_pass_tx_Filter.o vdt_test_001.o vdt_test_002.o vdt_test_003.o
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

script_exec.o: ../src/script_exec.c
	$(CC) -o $@ -c $< $(CFLAGS)

env.o: ../src/env.c
	$(CC) -o $@ -c $< $(CFLAGS)

FIR_V22_Minitel.o: ../src/FIR/FIR_V22_Minitel.c
	$(CC) -o $@ -c $< $(CFLAGS)

band_pass_rx_Filter.o: ../src/FIR/band_pass_rx_Filter.c
	$(CC) -o $@ -c $< $(CFLAGS)

low_pass_tx_Filter.o: ../src/FIR/low_pass_tx_Filter.c
	$(CC) -o $@ -c $< $(CFLAGS)

vdt_test_001.o: ../src/data/vdt_test_001.c
	$(CC) -o $@ -c $< $(CFLAGS)

vdt_test_002.o: ../src/data/vdt_test_002.c
	$(CC) -o $@ -c $< $(CFLAGS)

vdt_test_003.o: ../src/data/vdt_test_003.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf *.so
	rm -rf *.js *.wasm
	rm -rf $(EXEC) *.exe

mrproper: clean
	rm -rf $(EXEC)

.PHONY: clean mrproper

