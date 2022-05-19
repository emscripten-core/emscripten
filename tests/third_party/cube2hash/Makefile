all: libcube2hash.a

libcube2hash.a: cube2crypto.o tiger.o hashstring.o
	$(AR) rcs $@ $^

hashstring.o: hashstring.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

cube2crypto.o: cube2crypto.c cube2crypto.h
	$(CC) $(CFLAGS) -c $< -o $@

tiger.o: tiger.c tiger.h
	$(CC) $(CFLAGS) -c $< -o $@

