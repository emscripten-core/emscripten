; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-p:32:32-i64:64-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]
@.str2 = private unnamed_addr constant [15 x i8] c"hello!!world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

define i32 @main() {
  %retval = alloca i32, align 4
  %a12 = zext i1 1 to i32
  br label %L13

L13:
  %.0 = phi i64 [ undef, %0 ], [ 123, %L13 ]
  %a14 = trunc i64 %.0 to i32
  %call0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0))
  %a15 = add nsw i32 %a14, 2
  %a16 = icmp ne i32 %a15, 9
  br i1 %a16, label %L17, label %L13

L17:
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str2, i32 0, i32 0))
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)

