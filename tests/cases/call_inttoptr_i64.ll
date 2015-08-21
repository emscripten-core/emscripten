; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [21 x i8] c"hello, world %d %d!\0A\00", align 1

declare i32 @printf(i8*, ...)

define void @waka(i32 %x, i32 %y) {
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str, i32 0, i32 0), i32 %x, i32 %y)
  ret void
}

define i32 @main() {
entry:
  call void inttoptr (i32 ptrtoint (void (i32, i32)* @waka to i32) to void (i64)*)(i64 528280977418)
  ret i32 0
}



