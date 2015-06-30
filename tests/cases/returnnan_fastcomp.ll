; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [18 x i8] c"hello, world %f!\0A\00", align 1

define i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %f = call double @nand()
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), double %f)
  %g = call double @zerod()
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), double %g)
  %h = call float @zerof()
  %hd = fpext float %h to double
  %call3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), double %hd)
  ret i32 1
}

define double @nand() unnamed_addr align 2 {
  ret double 0x7FF8000000000000
}

define double @zerod() unnamed_addr align 2 {
  ret double 0x0000000000000000
}

define float @zerof() unnamed_addr align 2 {
  ret float 0x0000000000000000
}

declare i32 @printf(i8*, ...)

