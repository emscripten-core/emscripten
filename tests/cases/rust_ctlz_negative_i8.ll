; ModuleID = 'foo.cgu-0.rs'
source_filename = "foo.cgu-0.rs"
target datalayout = "e-p:32:32-i64:64-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [18 x i8] c"hello, world %d!\0A\00", align 1

; Function Attrs: inlinehint uwtable
define internal i32 @"_ZN4core3num20_$LT$impl$u20$u8$GT$13leading_zeros17h0d6a1b849828bff0E"(i8) unnamed_addr #0 {
entry-block:
  %tmp_ret = alloca i8
  br label %start

start:                                            ; preds = %entry-block
  %1 = call i8 @llvm.ctlz.i8(i8 %0, i1 false)
  store i8 %1, i8* %tmp_ret
  %2 = load i8, i8* %tmp_ret
  br label %bb1

bb1:                                              ; preds = %start
  %3 = zext i8 %2 to i32
  ret i32 %3
}

define i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %value = call i32 @"_ZN4core3num20_$LT$impl$u20$u8$GT$13leading_zeros17h0d6a1b849828bff0E"(i8 -1)
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 %value)
  ret i32 0
}

; Function Attrs: nounwind readnone
declare i8 @llvm.ctlz.i8(i8, i1) #2

declare i32 @printf(i8*, ...)

attributes #0 = { inlinehint uwtable }
attributes #1 = { uwtable }
attributes #2 = { nounwind readnone }
