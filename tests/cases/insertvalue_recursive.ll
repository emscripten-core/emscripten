; ModuleID = 'insertvalue_recursive.ll'
target datalayout = "e-p:32:32-i64:64-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%nested_struct = type { [1 x i64] }
%struct = type { [1 x i64], %nested_struct }

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %.1 = insertvalue [1 x i64] undef, i64 1, 0
  %inserted.first = insertvalue %struct undef, [1 x i64] %.1, 0

  %.2 = insertvalue [1 x i64] undef, i64 2, 0
  %.nested_struct = insertvalue %nested_struct undef, [1 x i64] %.2, 0
  %inserted.second = insertvalue %struct %inserted.first, %nested_struct %.nested_struct, 1
  
  %.3 = alloca %struct, align 8
  store %struct %inserted.second, %struct* %.3, align 8
  
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0 type=i32]
  ret i32 0
}