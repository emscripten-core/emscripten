target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private constant [18 x i8] c"hello, world: %d\0A\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
  %a333 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 5)
  %a400 = zext i32 %a333 to i64
  %check = trunc i32 %a333 to i1
  br i1 %check, label %l1, label %l2

l1:
  %bbb = phi i64 [ %a400, %0 ], [ 10, %l2 ]
  %bbb32 = trunc i64 %bbb to i32
  %a333z = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 %bbb32)
  %check2 = trunc i32 %bbb32 to i1
  br i1 %check2, label %l2, label %label999

l2:
  %a410 = phi i64 [ %a400, %0 ], [ %bbb, %l1 ]
  %a444 = udiv i64 %a410, 3
  switch i64 %a444, label %l1 [
    i64 1000, label %label9950
    i64 1001, label %label9951
    i64 1002, label %label9952
    i64 1003, label %label9953
    i64 1004, label %label9954
    i64 1005, label %label9955
    i64 1006, label %label9956
    i64 1007, label %label9957
    i64 1008, label %label9958
    i64 1009, label %label9959
  ]

label9950:
  %waka = phi i64 [1000, %l2], [0, %label9951], [1, %label9952], [2, %label9953], [3, %label9954], [4, %label9955], [5, %label9956], [6, %label9957], [7, %label9958], [8, %label9959]
  %waka32 = trunc i64 %waka to i32
  %a333b = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 %waka32)
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
  %last = phi i64 [1, %l1], [2, %label9950]
  %last32 = trunc i64 %last to i32
  %a333c = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 %last32)
  ret i32 0
}

