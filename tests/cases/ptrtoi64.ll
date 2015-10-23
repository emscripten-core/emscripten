; pointer to i64, then to i32

; ModuleID = '/tmp/emscripten/tmp/src.cpp.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str2 = private constant [9 x i8] c"*%d,%d*\0A\00", align 1 ; [#uses=1]

; [#uses=1]
declare i32 @puts(i8*)

declare i32 @printf(i8* noalias, ...) nounwind

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32                            ; [#uses=2]
  %0 = alloca i32                                 ; [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; [#uses=0]
  %sz.i7 = inttoptr i32 400 to i32*          ; [#uses=1 type=i32*]
  %a10 = ptrtoint i32* %sz.i7 to i64
  %conv5 = trunc i64 %a10 to i32
  %a11 = ptrtoint i32* %sz.i7 to i8
  %conv6 = zext i8 %a11 to i32
  %a55 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 %conv5, i32 %conv6)
  ret i32 0
}
