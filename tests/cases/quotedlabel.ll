; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private constant [14 x i8] c"hello, world!\00", align 1 ; [#uses=1]

; [#uses=1]
declare i32 @puts(i8*)

; [#uses=0]
define i32 @main() {
entry:
  br label %"finish$$$"

"finish$$$":                                           ; preds = %entry
  %0 = call i32 bitcast (i32 (i8*)* @puts to i32 (i32*)*)(i32* bitcast (i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0) to i32*)) ; [#uses=0]
  ret i32 0
}

