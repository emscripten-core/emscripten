; ModuleID = '/tmp/emscripten/tmp/src.cpp.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i386-pc-linux-gnu"

%struct.vec2 = type { float, float }

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00" ; [#uses=1]

; [#uses=1]
declare i32 @printf(i8* noalias, ...)

define linkonce_odr i32 @vec2Length(%struct.vec2* %this) nounwind align 2 {
entry:
  %__first.addr.i = alloca %struct.vec2*, align 4 ; [#uses=3 type=%struct.vec2**]
  %__last.addr.i = alloca %struct.vec2*, align 4 ; [#uses=3 type=%struct.vec2**]
  %__comp.addr.i = alloca %struct.vec2*, align 4 ; [#uses=2 type=%struct.vec2**]
  %a13 = bitcast %struct.vec2** %__first.addr.i to i8* ; [#uses=1 type=i8*]
  call void @llvm.lifetime.start(i64 -1, i8* %a13)
  %a14 = bitcast %struct.vec2** %__last.addr.i to i8* ; [#uses=1 type=i8*]
  call void @llvm.lifetime.start(i64 -1, i8* %a14)
  %a18 = bitcast %struct.vec2** %__first.addr.i to i8* ; [#uses=1 type=i8*]
  call void @llvm.lifetime.end(i64 -1, i8* %a18)
  %a19 = bitcast %struct.vec2** %__last.addr.i to i8* ; [#uses=1 type=i8*]
  call void @llvm.lifetime.end(i64 -1, i8* %a19)
  ret i32 0
}

define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1]
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  ret i32 0
}

declare void @llvm.lifetime.start(i64, i8*)
declare void @llvm.lifetime.end(i64, i8*)

