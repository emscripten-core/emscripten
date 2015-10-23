; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%1 = type { %2 }
%2 = type <{ i32 }>

@curly = constant %1 { %2 <{ i32 99 }> }, align 2 ; [#uses=1]

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00" ; [#uses=1]

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1]
  store i32 0, i32* %retval
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  ret i32 0
}

; [#uses=1]
declare i32 @printf(i8*, ...)

