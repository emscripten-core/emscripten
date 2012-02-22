OBJECTS = \
  cxa_vector.bc \
  private_typeinfo.bc \
  cxa_virtual.bc \
  temporary.bc \
  cxa_guard.bc \
  cxa_unexpected.bc \
  cxa_exception.bc \
  cxa_aux_runtime.bc \
  exception.bc \
  stdexcept.bc \
  abort_message.bc \
  cxa_personality.bc \
  cxa_new_delete.bc \
  cxa_handlers.bc \
  cxa_exception_storage.bc \
  typeinfo.bc \
  cxa_demangle.bc

all: libcxx.bc

%.bc: %.cpp
	$(CXX) $< -o $@

libcxx.bc: $(OBJECTS)
	$(CXX) $(OBJECTS) -o libcxxabi.bc

