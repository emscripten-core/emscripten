; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

; Phi nodes can refer to the entry. And the entry might be unnamed, and doesn't even have a consistent implicit name!

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

; [#uses=0]
define i32 @main() {
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  %a16 = trunc i32 1 to i1
  br i1 %a16, label %L17, label %L26

L17:
  %a25 = trunc i32 1 to i1
  br label %L26

L26:
  %a27 = phi i1 [ false, %0 ], [ %a25, %L17 ]        ; [#uses=1 type=i1]
  store i32 0, i32* %retval
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0 type=i32]
  %cal2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i1 %a27) ; make sure %27 is used
  ret i32 1
}

define i32 @main0() {
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  %a16 = trunc i32 1 to i1
  br i1 %a16, label %L17, label %L26

L17:
  %a25 = trunc i32 1 to i1
  br label %L26

L26:
  %a27 = phi i1 [ false, %0 ], [ %a25, %L17 ]        ; [#uses=1 type=i1]
  store i32 0, i32* %retval
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0 type=i32]
  %cal2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i1 %a27) ; make sure %27 is used
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
