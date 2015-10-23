; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str2 = private constant [6 x i8] c"*%f*\0A\00", align 1 ; [#uses=1]

declare i32 @printf(i8* noalias, ...) nounwind

define i32 @main() {
entry:
  %c = frem double 2.5, 1.0
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), double %c)
  ret i32 0
}
