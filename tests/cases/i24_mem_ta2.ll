; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [6 x i8] c".%x.\0A\00", align 1 ; [#uses=1 type=[5 x i8]*]

define i32 @main() {
entry:
  %mem = alloca i32
  store i32 4279383126, i32* %mem
  %i24 = bitcast i32* %mem to i24*
  %load = load i24, i24* %i24, align 4
  %load32 = zext i24 %load to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i32 %load32)
  %val_24 = trunc i32 4041265344 to i24
  store i24 %val_24, i24* %i24, align 4
  %load32b = load i32, i32* %mem, align 4
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i32 %load32b)
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
