var name;

var asmLibraryArg = {
 "a": 1,
 "A": 33,
 "b": ___syscall6,
 "__setErrNo": ___setErrNo,
 "memset": _memset,
 "sbrk": _sbrk,
 "memcpy": _memcpy,
 "emscripten_memcpy_big": _emscripten_memcpy_big,
 "c": ___syscall54,
 "d": ___syscall140,
 "q": ___syscall146
};

var expD1 = Module["expD1"] = asm["c"];

var expI1 = Module["expI1"] = function() {
 return Module["asm"]["d"].apply(null, arguments);
};
