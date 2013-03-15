; ModuleID = 'emptystruct.c'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

%struct.s = type { {}, i32 }

@.str = private constant [14 x i8] c"hello, world!\00", align 1 ; [#uses=1]

define i32 @main() nounwind {
entry:
  %z = alloca %struct.s, align 4
  %0 = bitcast %struct.s* %z to i8*
  call void @llvm.memset.p0i8.i32(i8* %0, i8 0, i32 4, i32 4, i1 false)
  %1 = call i32 bitcast (i32 (i8*)* @puts to i32 (i32*)*)(i8* getelementptr inbounds ([14 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  ret i32 0
}

declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) nounwind

declare i32 @puts(i8*)

