; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [20 x i8] c"hello, world %.2f!\0A\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %waka = select i1 fcmp ult (float fadd (float fmul (float undef, float 1.0), float 2.0), float 3.0), double 4.0, double 5.0
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str, i32 0, i32 0), double %waka)
  ret i32 0
}

