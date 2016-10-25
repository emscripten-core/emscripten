; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [18 x i8] c"hello, world %d!\0A\00", align 1
@.str.1 = private unnamed_addr constant [164 x i8] c"{ var xhr = new XMLHttpRequest(); xhr.open('GET', 'http://localhost:8888/report_result?' + $0, !$1); xhr.send(); setTimeout(function() { window.close() }, 1000); }\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %s4 = alloca i64, align 8
  store i64 0, i64* %s4
  %ar = atomicrmw add i64* %s4, i64 1 monotonic
  ; %ar = load i64, i64* %s4, align 8
  %value = trunc i64 %ar to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 %value)
  call i32 (i8*, ...) @emscripten_asm_const_int(i8* getelementptr inbounds ([164 x i8], [164 x i8]* @.str.1, i32 0, i32 0), i32 %value, i32 0)
  ret i32 0
}

declare i32 @emscripten_asm_const_int(i8*, ...) #1

