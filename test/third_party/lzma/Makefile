# Makefile for generating an LZMA library using Emscripten.

O = .
OBJECTS = \
  $(O)/Alloc.o \
  $(O)/LzFind.o \
  $(O)/LzmaDec.o \
  $(O)/LzmaEnc.o

all: lzma.a

%.o: %.c
	$(CC) -I. $< -o $@ -O2 -c

lzma.a: $(OBJECTS)
	$(AR) rv $@ $(OBJECTS)

clean:
	rm lzma.a

