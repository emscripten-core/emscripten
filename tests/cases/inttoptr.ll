; ModuleID = '/tmp/emscripten/tmp/src.cpp.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private constant [14 x i8] c"hello, world!\00", align 1 ; [#uses=1]

; [#uses=1]
declare i32 @puts(i8*)

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32                            ; [#uses=2]
  %0 = alloca i32                                 ; [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; [#uses=0]
  %sz.i7 = inttoptr i32 32768 to i32*          ; [#uses=1 type=i32*] XXX VERY BAD, but we pick a big number so unlikely to hit anything in the runtime
  store i32 184, i32* %sz.i7, align 8
  %1 = call i32 @puts(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  ret i32 0
}
