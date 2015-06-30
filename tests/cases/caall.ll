; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  store i32 0, i32* %retval
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0 type=i32]
  %call12 = call void (i32*)** @_ZNSt3__13mapINS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEEPFvP6ObjectENS_4lessIS6_EENS4_INS_4pairIKS6_SA_EEEEEixERSE_(i32 10)
  %l26 = load void (i32*)*, void (i32*)** %call12
  ret i32 1
}

define void (i32*)** @_ZNSt3__13mapINS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEEPFvP6ObjectENS_4lessIS6_EENS4_INS_4pairIKS6_SA_EEEEEixERSE_(i32 %x) {
entry:
  %ret = inttoptr i32 0 to void (i32*)**
  ret void (i32*)** %ret
}

; [#uses=1]
declare i32 @printf(i8*, ...)
