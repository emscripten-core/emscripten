; ModuleID = '/dev/shm/tmp/src.cpp.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]
@_dispatchTable = internal global i64 0

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  %comp = alloca { i32, i32 }, align 4            ; [#uses=1]
  store i32 0, i32* %retval
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0 type=i32] [debug line = 5:13]
  br label %cond.end

cond.null:
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi { i32, i32 } [ { i32 5, i32 6 }, %entry ], [ zeroinitializer, %cond.null ] ; [#uses=1]
  store { i32, i32 } %cond, { i32, i32 }* %comp

  ret i32 0                             ; [debug line = 6:13]
}

; [#uses=1]
declare i32 @printf(i8*, ...)

