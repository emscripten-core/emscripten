OBJECTS = \
  src/private_typeinfo.bc \
  $(NULL)
  #src/cxa_vector.bc \
  #src/cxa_virtual.bc \
  #src/temporary.bc \
  #src/cxa_guard.bc \
  #src/cxa_unexpected.bc \
  #src/cxa_exception.bc \
  #src/cxa_aux_runtime.bc \
  #src/exception.bc \
  #src/stdexcept.bc \
  #src/abort_message.bc \
  #src/cxa_personality.bc \
  #src/cxa_new_delete.bc \
  #src/cxa_handlers.bc \
  #src/cxa_exception_storage.bc \
  #src/typeinfo.bc \
  #src/cxa_demangle.bc

all: libcxxabi.bc

%.bc: %.cpp
	$(CXX) -I./include $< -o $@

libcxxabi.bc: $(OBJECTS)
	$(CXX) $(OBJECTS) -o libcxxabi.bc

