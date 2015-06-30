; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [6 x i8] c"test\0A\00"

declare hidden zeroext i1 @_OptionParser(i8*, i8) ; separate test: check that we can compile this line (zeroext confused us)

define i32 @main() nounwind {
  %1 = alloca i32, align 4
  store i32 0, i32* %1
  %2 = call i32 @llvm.expect.i32(i32 0, i32 1)
  %3 = call i32 @puts(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0))
  ret i32 0
}

declare i32 @puts(i8*)

declare i32 @llvm.expect.i32(i32, i32)

