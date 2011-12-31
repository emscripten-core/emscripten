; pointer to i64, then to i32

; ModuleID = '/tmp/emscripten/tmp/src.cpp.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i386-pc-linux-gnu"

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
  %10 = ptrtoint i32* %sz.i7 to i64, !dbg !8557        ; [#uses=1 type=i64] [debug line = 99:3]
  %conv5 = trunc i64 %10 to i32, !dbg !8557       ; [#uses=1 type=i32] [debug line = 99:3]
  %11 = ptrtoint i32* %sz.i7 to i8, !dbg !8557        ; [#uses=1 type=i64] [debug line = 99:3]
  %conv6 = zext i8 %11 to i32, !dbg !8557       ; [#uses=1 type=i32] [debug line = 99:3]
  %55 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([9 x i8]* @.str2, i32 0, i32 0), i32 %conv5, i32 %conv6) ; [#uses=0]
  ret i32 0
}
