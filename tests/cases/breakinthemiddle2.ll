; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1]

define linkonce_odr i32 @main() align 2 {
  %a333 = call i32 @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  %b199 = trunc i8 1 to i1                        ; [#uses=1]
  br i1 %b199, label %label555, label %label569

label555:                                     ; preds = %0
  br label %label569 ; branch should ignore all code after it in the block
                                              ; No predecessors!
  br label %label569

label569:                                     ; preds = %0
  br i1 %b199, label %label990, label %label999

label990:
  ret i32 0 ; ret should ignore all code after it in the block
                                              ; No predecessors!
  br label %label569

label999:                                     ; preds = %555
  ret i32 0
}

declare i32 @printf(i8*)

