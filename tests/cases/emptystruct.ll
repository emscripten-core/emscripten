; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%struct.s = type { {}, i32 }

@.str = private constant [14 x i8] c"hello, world!\00", align 1 ; [#uses=1]

@.waka = extern_weak global i8* ; no initializer!

define i32 @main() nounwind {
entry:
  %z = alloca %struct.s, align 4
  %0 = bitcast %struct.s* %z to i8*
  call void @llvm.memset.p0i8.i32(i8* %0, i8 0, i32 4, i32 4, i1 false)
  %1 = call i32 bitcast (i32 (i8*)* @puts to i32 (i32*)*)(i32* bitcast (i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0) to i32*)) ; [#uses=0]
  ret i32 0
}

declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) nounwind

declare i32 @puts(i8*)

