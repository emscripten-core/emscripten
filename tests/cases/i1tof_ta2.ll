; ModuleID = 'bad/emcc-0-basebc.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [7 x i8] c"%0.1f\0A\00", align 1
@.str2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline
define float @_Z2v1v() #0 {
entry:
  %call = tail call double @emscripten_get_now()
  %cmp = fcmp oge double %call, 0.000000e+00
  %cond = select i1 %cmp, float 0x3FECCCCCC0000000, float 0.000000e+00
  ret float %cond
}

define double @emscripten_get_now() #0 {
  ret double 1.0
}

; Function Attrs: noinline
define float @_Z2v2v() #0 {
entry:
  %call = tail call double @emscripten_get_now()
  %cmp = fcmp oge double %call, 0.000000e+00
  %cond = select i1 %cmp, float 0x3FD99999A0000000, float 0.000000e+00
  ret float %cond
}

; Function Attrs: noinline
define float @_Z2v3v() #0 {
entry:
  %call = tail call double @emscripten_get_now()
  %cmp = fcmp oge double %call, 0.000000e+00
  %cond = select i1 %cmp, float 0x3FB99999A0000000, float 0.000000e+00
  ret float %cond
}

define i32 @main() #1 {
entry:
  %call = tail call float @_Z2v1v()
  %call1 = tail call float @_Z2v2v()
  %call2 = tail call float @_Z2v3v()
  %sub = fsub float %call1, %call
  %cmp.i = fcmp ogt float %sub, 0.000000e+00
  br i1 %cmp.i, label %_ZL5signff.exit, label %cond.false.i

cond.false.i:                                     ; preds = %entry
  %cmp1.i = fcmp olt float %sub, 0.000000e+00
  %phitmp = sitofp i1 %cmp1.i to float
  %phitmpd = fpext float %phitmp to double
  %call1115a = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), double %phitmpd)
  %phitmpi = sext i1 %cmp1.i to i32
  %call1115b = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str2, i32 0, i32 0), i32 %phitmpi)
  br label %_ZL5signff.exit

_ZL5signff.exit:                                  ; preds = %cond.false.i, %entry
  %cond2.i = phi float [ %phitmp, %cond.false.i ], [ 1.000000e+00, %entry ]
  %mul = fmul float %call2, %cond2.i
  %add = fadd float %call, %mul
  %conv4 = fpext float %add to double
  %call5 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), double %conv4)
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture, ...) #2

attributes #0 = { noinline "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
