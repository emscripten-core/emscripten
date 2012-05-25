; ModuleID = '/tmp/tmpqfApGD/a.out.bc'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

@other-name = alias i32 ()* @main

@.st-r = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1

define i32 @main() {
entry:
  %ret-val = alloca i32, align 4
  store i32 0, i32* %ret-val
  %aaa = bitcast i32 ()* @other-name to i32
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([15 x i8]* @.st-r, i32 0, i32 0), i32 %aaa)
  ret i32 %ret-val
}

declare i32 @printf(i8*, ...)
