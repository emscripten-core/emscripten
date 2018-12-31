; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [20 x i8] c"hello, world %lld!\0A\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %s4 = alloca i64, align 8
  store i64 0, i64* %s4
  %ar = atomicrmw add i64* %s4, i64 1 monotonic
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str, i32 0, i32 0), i64 %ar)
  %ar2 = atomicrmw add i64* %s4, i64 1099511627776 monotonic
  %ar3 = atomicrmw add i64* %s4, i64 1099511627776 monotonic
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str, i32 0, i32 0), i64 %ar3)
  ret i32 0
}

declare i32 @emscripten_asm_const_int(i8*, ...) #1

declare i64 @_emscripten_atomic_fetch_and_add_u64(i8*, i64)

