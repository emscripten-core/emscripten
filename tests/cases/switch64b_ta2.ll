target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private constant [18 x i8] c"hello, world: %d\0A\00", align 1

declare i32 @printf(i8*, ...)

define linkonce_odr i32 @main() align 2 {
  %a333 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 5)
  %a400 = zext i32 %a333 to i64
  %a444 = udiv i64 %a400, 3
  switch i64 %a444, label %label999 [
    i64 0, label %label9950
    i64 1, label %label9951
    i64 2, label %label9952
    i64 3, label %label9953
    i64 4, label %label9954
    i64 5, label %label9955
    i64 6, label %label9956
    i64 7, label %label9957
    i64 8, label %label9958
    i64 9, label %label9959
  ]

label9950:
  %waka = phi i32 [11000, %0], [10, %label9951], [11, %label9952], [12, %label9953], [13, %label9954], [14, %label9955], [15, %label9956], [16, %label9957], [17, %label9958], [18, %label9959]
  %a333b = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 %waka)
  br label %label999

label9951:
  br label %label9950
label9952:
  br label %label9950
label9953:
  br label %label9950
label9954:
  br label %label9950
label9955:
  br label %label9950
label9956:
  br label %label9950
label9957:
  br label %label9950
label9958:
  br label %label9950
label9959:
  br label %label9950

label999:                                     ; preds = %555
  %last = phi i32 [1, %0], [2, %label9950]
  %a333c = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 %last)
  ret i32 0
}

