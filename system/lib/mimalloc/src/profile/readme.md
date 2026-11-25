# Allocation Sampling and Profiling

- [`sample-profile.c`](sample-profile.c): Allocation sampling, profiler callbacks.
- [`sample-guarded.c`](sample-guarded.c): Sampled allocations protected by guard pages.
- [`pprof.c`](pprof.c): Default heap profiler implementation with stack traces and pprof-compatible text or protobuf snapshots.
- [`pprof-os.c`](pprof-os.c): OS specific primitives of the profiler: backtraces, wall-clock time, snapshot files, and loaded modules.
- [`pprof-write.c`](pprof-write.c): Writing profiler snapshots in the pprof text or protobuf format.
- [`pprof.h`](pprof.h): Private declarations shared by the profiler modules.
