; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [23 x i8] c"hello..world!%x,%x,%x\0A\00", align 1

; [#uses=0]
define i32 @main() {
entry:
  %a = trunc i128 5634002668910802268259393791 to i96
  %x = trunc i96 %a to i32
  %y0 = lshr i96 %a, 32
  %y = trunc i96 %y0 to i32
  %z0 = lshr i96 %a, 64
  %z = trunc i96 %z0 to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str, i32 0, i32 0), i32 %x, i32 %y, i32 %z) ; [#uses=0 type=i32]
  %b = lshr i96 78981949996569583556032504063, 17
  %bx = trunc i96 %b to i32
  %by0 = lshr i96 %b, 32
  %by = trunc i96 %by0 to i32
  %bz0 = lshr i96 %b, 64
  %bz = trunc i96 %bz0 to i32
  %bcall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str, i32 0, i32 0), i32 %bx, i32 %by, i32 %bz) ; [#uses=0 type=i32]
  %c = ashr i96 78981949996569583556032504063, 17
  %cx = trunc i96 %c to i32
  %cy0 = lshr i96 %c, 32
  %cy = trunc i96 %cy0 to i32
  %cz0 = lshr i96 %c, 64
  %cz = trunc i96 %cz0 to i32
  %ccall = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str, i32 0, i32 0), i32 %cx, i32 %cy, i32 %cz) ; [#uses=0 type=i32]
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
