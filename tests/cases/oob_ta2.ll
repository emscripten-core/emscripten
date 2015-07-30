; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%structy = type { [2 x [10 x i8]] }

@.str1 = private unnamed_addr constant [10 x i8] c"1234567890", align 1
@.str2 = private unnamed_addr constant [10 x i8] c"wakawaka\0A\00", align 1
@.stry = private unnamed_addr constant [2 x %structy] { %structy { [10 x i8] @.str1, [10 x i8] @.str2 }, %structy { [10 x i8] @.str1, [10 x i8] @.str2 } }

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

; [#uses=0]
define i32 @main(i32 %argc, i8** %argv) {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  store i32 0, i32* %retval
  %ind = add i32 %argc, 13
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x %structy], [2 x %structy]* @.stry, i32 0, i32 2, i32 0, i32 %ind))
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0 type=i32]
  ret i32 1  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
