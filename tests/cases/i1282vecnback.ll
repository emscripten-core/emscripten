; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"hello, wor%f!\0A\00", align 1
@.str1 = private unnamed_addr constant [15 x i8] c"hello, wor%d!\0A\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %z = insertelement <4 x i32> undef, i32 123, i32 1
  %a = bitcast <4 x i32> %z to i128
  %b = bitcast i128 %a to <4 x float>
  %c = extractelement <4 x float> %b, i32 1
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), float %c)
  %z1 = insertelement <4 x i32> undef, i32 123, i32 1
  %a1 = bitcast <4 x i32> %z1 to i128
  %b1 = bitcast i128 %a1 to <4 x i32>
  %c1 = extractelement <4 x i32> %b1, i32 1
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str1, i32 0, i32 0), i32 %c1)
  ret i32 0
}

