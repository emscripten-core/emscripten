; ModuleID = 'foo.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%ac = type { i8*, i64 }

@0 = constant [9 x i8] c"func %s\0A\00"
@1 = constant [4 x i8] c"foo\00"

declare void @llvm.trap() noreturn nounwind

define void @direct(%ac) {
entry:
  %str = alloca %ac
  store %ac %0, %ac* %str
  %1 = getelementptr inbounds %ac, %ac* %str, i32 0, i32 0
  %2 = load i8*, i8** %1
  call void (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @0, i32 0, i32 0), i8* %2)
  ret void
}

declare void @printf(i8*, ...)

define i32 @main() {
entry:
  call void @direct(%ac { i8* getelementptr inbounds ([4 x i8], [4 x i8]* @1, i32 0, i32 0), i64 3 })
  ret i32 0
}

