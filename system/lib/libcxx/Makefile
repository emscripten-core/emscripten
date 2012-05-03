OBJECTS = \
  algorithm.bc \
  condition_variable.bc \
  future.bc \
  iostream.bc \
  memory.bc \
  random.bc \
  stdexcept.bc \
  system_error.bc \
  utility.bc \
  bind.bc \
  debug.bc \
  hash.bc \
  mutex.bc \
  string.bc \
  thread.bc \
  valarray.bc \
  chrono.bc \
  exception.bc \
  ios.bc \
  locale.bc \
  regex.bc \
  strstream.bc \
  typeinfo.bc

all: libcxx.bc

%.bc: %.cpp
	$(CXX) $< -o $@

libcxx.bc: $(OBJECTS)
	$(CXX) $(OBJECTS) -o libcxx.bc

