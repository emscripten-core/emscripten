; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

define void @glSampleCoverage(float %a18, i8 zeroext %invert) {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0 type=i32]
  ret void
}

; [#uses=0]
define i32 @main() {
entry:
  tail call void @glSampleCoverage(float 3.5, i8 zeroext 12)
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)

