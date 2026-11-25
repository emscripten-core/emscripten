# Internal Headers

- [`types.h`](types.h): Defines the types of all internal mimalloc datastructures
and configuration options.
- [`config.h`](config.h): Build configuration defines (like `MI_DEBUG`, `MI_SECURE`, etc.).
- [`decl.h`](decl.h): Compiler attributes, assertions, initialization macros, and platform-specific declarations.
- [`arch.h`](arch.h): Architecture dependent type definitions, bit operations, and pointer provenance support (as on CHERI).
- [`atomic.h`](atomic.h): Portable atomic operations and locks.
- [`track.h`](track.h): Memory-tracking integration for Valgrind, AddressSanitizer, and Windows event tracing.
- [`qsbr.h`](qsbr.h): Quiescent-state based reclamation primitives (for runtime systems like CPython).
- [`internal.h`](internal.h): Deprecated: umbrella header for internal allocator declarations and utilities.

# Source Headers

- [`alloc`](alloc): Internal allocation, freeing, and page-management interfaces.
- [`arena`](arena): Arena management, slice allocation, and arena-backed page interfaces.
- [`heap`](heap): Heap, thread-local heap, subprocess, initialization, and page-map interfaces.
  Also contains [`heap/process.h`](heap/process.h) which declares (almost) all static variables.
- [`profile`](profile): Sampled allocation, guarded allocation, and profiler interfaces.
- [`util`](util): Runtime utilities, options, randomness, statistics, and dynamic thread-local interfaces.
- [`prim`](prim): Higher-level OS memory operations and primitive thread-local access.
  * [`prim/prim.h`](prim/prim.h): Primitive and minimal portability API needed from a host system.
  * [`prim/prim-tls.h`](prim/prim-tls.h): Fast thread-local access (for the default theap).
  * [`prim/prim-mte.h`](prim/prim-mte.h): Primitives for memory-tagging ([MTE] or MIE); this is enabled when building with `-DMI_MTE=ON`.
  * [`prim/prim-pac.h`](prim/prim-pac.h): Pointer authentication and signing (PAC); enabled when building with `-DMI_PAC=ON`.


[MTE]:   https://thore.io/posts/2025/09/introduction-to-arm-memory-tagging-extensions/
