# Simple makefile for building. Works only on emcc and linux probably.

OBJECTS = \
  core_list_join.o \
  core_matrix.o \
  core_portme.o \
  core_state.o \
  core_util.o

all: coremark.a

%.o: %.c
	$(CC) $(CFLAGS) -I. $< -o $@ -O2 -c

coremark.a: $(OBJECTS)
	$(AR) rvs $@ $(OBJECTS)

clean:
	rm coremark.a
	rm *.o

