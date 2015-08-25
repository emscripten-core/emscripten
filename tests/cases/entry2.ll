; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [11 x i8] c"sched_=%d\0A\00", align 1
@.str1 = private unnamed_addr constant [6 x i8] c"f=%d\0A\00", align 1

define internal i32 @_Z1fii(i32, i32) noinline {
  %3 = tail call i32 @sched_yield()
  %4 = icmp eq i32 %3, 0
  br i1 %4, label %7, label %5

; <label>:5                                       ; preds = %2
  %6 = tail call i32 @sched_yield()
  br label %7

; <label>:7                                       ; preds = %5, %2
  %.0 = phi i32 [ 0, %5 ], [ 1, %2 ]
  ret i32 %.0
}

declare i32 @sched_yield()

define i32 @main() {
  %1 = tail call i32 @sched_yield()
  %2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i32 0, i32 0), i32 %1)
  %3 = tail call i32 @_Z1fii(i32 undef, i32 undef)
  %4 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str1, i32 0, i32 0), i32 %3)
  ret i32 0
}

declare i32 @printf(i8* nocapture, ...) nounwind
