target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [9 x i8] c"*%d,%d*\0A\00" ; [#uses=1]

define {i32, i32} @read_pair({i32, i32}* %ptr) norecurse nounwind readonly uwtable {
entry:
  %value = load {i32, i32}, {i32, i32}* %ptr, align 4
  ret {i32, i32} %value
}

; [#uses=0]
define i32 @main() {
entry:
  %a = alloca {i32, i32}, align 4
  %a0 = getelementptr {i32, i32}, {i32, i32}* %a, i32 0, i32 0
  %a1 = getelementptr {i32, i32}, {i32, i32}* %a, i32 0, i32 1
  %b = alloca {i32, i32}, align 4
  %b0 = getelementptr {i32, i32}, {i32, i32}* %b, i32 0, i32 0
  %b1 = getelementptr {i32, i32}, {i32, i32}* %b, i32 0, i32 1
  
  ; Initialize a with {1234, 5678} and b with {0, 0}
  store i32 1234, i32* %a0, align 4
  store i32 5678, i32* %a1, align 4
  store i32 0, i32* %b0, align 4
  store i32 0, i32* %b1, align 4
  
  ; This call should remain in the output.
  %v = call {i32, i32} @read_pair({i32, i32}* %a)
  store {i32, i32} %v, {i32, i32}* %b, align 4
  
  %b0v = load i32, i32* %b0, align 4
  %b1v = load i32, i32* %b1, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i32 %b0v, i32 %b1v) ; [#uses=0 type=i32]
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
