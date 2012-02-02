; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

; Phi nodes can refer to the entry. And the entry might be unnamed, and doesn't even have a consistent implicit name!

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

; [#uses=0]
define i32 @main() {
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  %16 = trunc i32 1 to i1
  br i1 %16, label %whoosh, label %26, !dbg !1269853  ; [debug line = 3920:5]

whoosh: ; preds = %1
  %25 = trunc i32 1 to i1
  br label %26

; <label>:26                                      ; preds = %17, %1
  %27 = phi i1 [ false, %1 ], [ %25, %whoosh ]        ; [#uses=1 type=i1]
  %28 = phi i1 [ true, %1 ], [ %25, %whoosh ]        ; [#uses=1 type=i1]
  store i32 0, i32* %retval
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0 type=i32]
  %cal2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([15 x i8]* @.str, i32 0, i32 0), i32 %27) ; make sure %27 is used
  %cal3 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([15 x i8]* @.str, i32 0, i32 0), i32 %28) ; make sure %28 is used
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
