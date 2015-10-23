; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private constant [9 x i8] c"*%d,%d*\0A\00", align 1 ; [#uses=1]

; [#uses=1]
declare i32 @printf(i8* noalias, ...)

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32                            ; [#uses=2]
  %0 = alloca i32                                 ; [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; [#uses=0]
  store i32 4, i32* %0, align 4
  %1 = load i32, i32* %0, align 4                      ; [#uses=1]
  %2 = trunc i32 %1 to i1                          ; [#uses=1]
  %3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i32 %1, i1 %2) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  %retval1 = load i32, i32* %retval                    ; [#uses=1]
  ret i32 %retval1
}
