; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"helloworld%d.\0A\00", align 1

define i1 @boolretter() {
  ret i1 -1
}

define i8 @smallretter() {
  ret i8 -1
}

define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  store i32 0, i32* %retval
  %bool = call i1 () @boolretter()
  %combined = xor i1 %bool, 1
  %int = select i1 %combined, i32 20, i32 30
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i32 %int)
  %small = call i8 () @smallretter()
  %bcombined = xor i8 %small, 1
  %bcheck = icmp eq i8 %bcombined, 255
  %bint = select i1 %bcheck, i32 20, i32 30
  %bcall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i32 %bint)
  ret i32 1
}

declare i32 @printf(i8*, ...)
