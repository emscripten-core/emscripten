target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i386-pc-linux-gnu"

@.str2 = private constant [6 x i8] c"*%f*\0A\00", align 1 ; [#uses=1]

declare i32 @printf(i8* noalias, ...) nounwind

define i32 @main() {
entry:
  %c = frem double 2.5, 1.0
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([6 x i8]* @.str2, i32 0, i32 0), double %c)
  ret i32 0
}
