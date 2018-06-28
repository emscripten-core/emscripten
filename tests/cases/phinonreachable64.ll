; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-p:32:32-i64:64-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

define i32 @main() {
_entry:
  %retval = alloca i32, align 4
  %a12 = zext i1 1 to i64
  br label %_phinode

_nonreachable:
  %b = zext i32 -1 to i64
  br label %_phinode

_phinode:
  %a14 = phi i64 [ %a12, %_entry ], [ %b, %_nonreachable ]
  %a14s = trunc i64 %a14 to i32
  %call0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0))
  ret i32 %a14s
}

; [#uses=1]
declare i32 @printf(i8*, ...)

