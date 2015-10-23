target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private constant [18 x i8] c"hello, world: %d\0A\00", align 1

declare i32 @printf(i8*, ...)

define linkonce_odr i32 @main() align 2 {
entry:
  %temp32 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 5)
  switch i32 %temp32, label %mid1 [
    i32 1000, label %mid1
    i32 1001, label %mid2
    i32 2000, label %finish
  ]

mid1:
  br label %finish

mid2:
  br label %finish

finish:                                     ; preds = %555
  %last = phi i32 [0, %entry], [1, %mid1], [2, %mid2]
  %a333c = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 %last)
  ret i32 0
}

