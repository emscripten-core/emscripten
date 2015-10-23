; ModuleID = '/tmp/emscripten/tmp/src.cpp.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private constant [14 x i8] c"hello, world!\00", align 1 ; [#uses=1]

; [#uses=2]
define void @"_Z5hellov"() {
entry:
  %0 = call i32 bitcast (i32 (i32*)* @puts to i32 (i8*)*)(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
declare i32 @puts(i32*)

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32                            ; [#uses=2]
  %0 = alloca i32                                 ; [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; [#uses=0]
  call void @"_Z5hellov"()
  store i32 0, i32* %0, align 4
  %1 = load i32, i32* %0, align 4                      ; [#uses=1]
  store i32 %1, i32* %retval, align 4
  br label %return

return:                                           ; preds = %entry
  %retval1 = load i32, i32* %retval                    ; [#uses=1]
  ret i32 %retval1
}
