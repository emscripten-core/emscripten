; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private constant [14 x i8] c"hello, world!\00", align 1 ; [#uses=1]
@.str2 = private unnamed_addr constant [9 x i8] c"*%d...*\0A\00", align 1 ; [#uses=1 type=[9 x i8]*]

; [#uses=1]
declare i32 @puts(i8*)

declare i32 @printf(i8* nocapture, ...) nounwind

define i32 @PyLong_FromVoidPtr(i8* %p) nounwind {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i8* %p) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret i32 0
}

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32                            ; [#uses=2]
  %0 = alloca i32                                 ; [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; [#uses=0]
  %a5 = call i32 @PyLong_FromVoidPtr(i8* null) nounwind ; [#uses=0]
  %a13 = call i32 @PyLong_FromVoidPtr(i8* inttoptr (i64 1 to i8*)) nounwind ; [#uses=0]
  %a1 = call i32 @puts(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  ret i32 0
}
