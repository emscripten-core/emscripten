; ModuleID = '/dev/shm/tmp/src.cpp.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-f128:128:128-n8:16:32"
target triple = "i386-pc-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"*yes*\00", align 1 ; [#uses=1]

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32                            ; [#uses=2]
  %0 = alloca i32                                 ; [#uses=2]
  %x = alloca i32                                 ; [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; [#uses=0]
  store i32 5, i32* %x, align 4
  %1 = load i32* %x, align 4                      ; [#uses=1]
  br i1 icmp sgt (i32 %1, i32 3), label %bb, label %bb1

bb:                                               ; preds = %entry
  %3 = call i32 @puts(i8* getelementptr inbounds ([6 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  br label %bb1

bb1:                                              ; preds = %bb, %entry
  store i32 0, i32* %0, align 4
  %4 = load i32* %0, align 4                      ; [#uses=1]
  store i32 %4, i32* %retval, align 4
  br label %return

return:                                           ; preds = %bb1
  %retval2 = load i32* %retval                    ; [#uses=1]
  ret i32 %retval2
}

; [#uses=1]
declare i32 @puts(i8*)

;  br i1 icmp ne (%16* bitcast (%11* @PyFloat_Type to %16*), %16* @PyBaseObject_Type), label %bb6, label %bb14

