; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"hello, %d,%d!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

; [#uses=0]
define i32 @main() {
entry:
  %t = alloca i32, align 4                  ; [#uses=2 type=i32**]
  store i32 50, i32* %t, align 4
  %0 = load i32, i32* %t
  %1 = atomicrmw sub i32* %t, i32 3 seq_cst ; [#uses=0 type=i32] [debug line = 21:12]
  %2 = load i32, i32* %t
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i32 %0, i32 %2) ; [#uses=0 type=i32]
  %3 = atomicrmw volatile add i32* %t, i32 3 seq_cst ; [#uses=0 type=i32] [debug line = 21:12]
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
