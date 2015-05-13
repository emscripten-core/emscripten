; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1
@.str1 = private unnamed_addr constant [15 x i8] c"hello, worldA\0A\00", align 1
@.str2 = private unnamed_addr constant [15 x i8] c"hello, worldB\0A\00", align 1

define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  store i32 0, i32* %retval
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i32 1)
  %bad = call zeroext i1 null()
  %bad2 = call zeroext i1 null(i32 5, float 1.0E2, double 0.02, i64 1000, i1 0, i32* %retval)
  %bad3 = call zeroext i1 (i32, float, double, i64, i1, i32*) null(i32 5, float 1.0E2, double 0.02, i64 1000, i1 0, i32* %retval)
  %bad4 = or i1 %bad, %bad2
  %bad5 = or i1 %bad3, %bad4
  br i1 %bad5, label %pre, label %finish

pre:
  %call0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str1, i32 0, i32 0), i32 0)
  ret i32 0

finish:
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str2, i32 0, i32 0), i32 1)
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
