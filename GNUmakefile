CC = $(CROSS)gcc
LD = $(CROSS)gcc

CFLAGS = -Wall -std=gnu99 -I. -Os -g0 -march=i686 -mfpmath=sse -msse -ffast-math -fexcess-precision=fast
LDFLAGS = -shared -Wl,--dll,--add-stdcall-alias -Wl,-s -L.

.Phony: all clean

all: benchmark.auo

benchmark.auo: benchmark.o
	$(LD) $(LDFLAGS) $(XLDFLAGS) -o $@ $^

benchmark.o: benchmark.c output.h
	$(CC) -c $(CFLAGS) $(XCFLAGS) -o $@ $<

clean:
	$(RM) *.auo *.o
