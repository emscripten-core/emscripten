; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

define i32 @main() {
entry:
  %retval = alloca i32, align 4
  %a12 = zext i1 1 to i32
  br label %label13

label13:                                      ; preds = %13, %1
  %a14 = phi i32 [ %a12, %entry ], [ %a15, %135 ]
  %call0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0))
  %a15 = add nsw i32 %a14, 2
  %a16 = icmp eq i32 %a15, 9
  br label %label17

label17:                                      ; preds = %1
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)

