; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str2 = private constant [6 x i8] c"*%d*\0A\00", align 1 ; [#uses=1]

declare i32 @printf(i8* noalias, ...) nounwind
declare i32 @llvm.bitreverse.i32(i32) #1

define i32 @main() {
entry:
  %a = call i32 @llvm.bitreverse.i32(i32 1)
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), i32 %a)
  %b = call i32 @llvm.bitreverse.i32(i32 -2147483648)
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), i32 %b)
  %c = call i32 @llvm.bitreverse.i32(i32 -1)
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), i32 %c)
  %d = call i32 @llvm.bitreverse.i32(i32 0)
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), i32 %d)
  %e = call i32 @llvm.bitreverse.i32(i32 275600565)
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), i32 %e)
  %f = call i32 @llvm.bitreverse.i32(i32 -8)
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), i32 %f)

  ret i32 0
}
