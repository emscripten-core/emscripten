; ModuleID = 'a.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [11 x i8] c"float: %f\0A\00", align 1

define void @_Z10printFloatf(float %f) #0 {
entry:
  %conv = fpext float %f to double
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i32 0, i32 0), double %conv)
  ret void
}

define i32 @main() #1 {
entry:
  tail call void @_Z10printFloatf(float 1.000000e+00)
  call void @emscripten_preinvoke(i32 0)
  call void @_Z10printFloatf(float undef)
  %last = call i32 @emscripten_postinvoke(i32 0)
  %lastf = sitofp i32 %last to float
  tail call void @_Z10printFloatf(float %lastf)
  ret i32 1
}

declare void @emscripten_preinvoke(i32)
declare i32 @emscripten_postinvoke(i32)
declare i32 @printf(i8* nocapture, ...) #1

attributes #0 = { noinline nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
