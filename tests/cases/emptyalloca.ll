; ModuleID = '/tmp/tmpjSNiky/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [30 x i8] c"Module.print('hello, world!')\00", align 1

; Function Attrs: nounwind
define internal void @_Z9doNothingPi(i32* %arr) #0 {
  %1 = alloca i32*, align 4
  store i32* %arr, i32** %1, align 4
  ret void
}

define i32 @main() #1 {
  %arr = alloca [0 x i32], align 4
  %1 = bitcast [0 x i32]* %arr to i8*
  call void @llvm.memset.p0i8.i32(i8* %1, i8 0, i32 0, i32 4, i1 false)
  %2 = getelementptr inbounds [0 x i32], [0 x i32]* %arr, i32 0, i32 0
  call void @_Z9doNothingPi(i32* %2)
  call void @emscripten_asm_const(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @.str, i32 0, i32 0))
  ret i32 0
}

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) #2

declare void @emscripten_asm_const(i8*) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
