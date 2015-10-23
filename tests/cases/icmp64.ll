; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [17 x i8] c"hello, world %d\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  store i32 0, i32* %retval
  %a0 = add i64 0, 0
  %a1 = icmp slt i64 %a0, 0
  %a2 = zext i1 %a1 to i32
  %calla = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i32 0, i32 0), i32 %a2)
  %b0 = add i64 0, 1
  %b1 = icmp slt i64 %b0, 0
  %b2 = zext i1 %b1 to i32
  %callb = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i32 0, i32 0), i32 %b2)
  %c0 = sub i64 1, 0
  %c1 = icmp slt i64 %c0, 0
  %c2 = zext i1 %c1 to i32
  %callc = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i32 0, i32 0), i32 %c2)
  %d0 = sub i64 0, 1
  %d1 = icmp slt i64 %d0, 0
  %d2 = zext i1 %d1 to i32
  %calld = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i32 0, i32 0), i32 %d2)
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
