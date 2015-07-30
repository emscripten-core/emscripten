; ModuleID = 'foo.bc'
target datalayout = "e-p:32:32-i64:64-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1

; Function Attrs: nounwind
define i32 @main() {
  %1 = call i8* @malloc(i32 4)
  %2 = ptrtoint i8* %1 to i1
  call void @llvm.assume(i1 %2)
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0))
  ret i32 5
}

declare i8* @malloc(i32)
declare void @llvm.assume(i1 %cond)
declare i32 @printf(i8*, ...)

