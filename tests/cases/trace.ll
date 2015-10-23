; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

; A [block] type is used in a function def here. If we investigate types in the very first pass, we will
; make a bad guess as to the size - we assume undef'd types are [int32]. For this code to compile, we must
; only investigate in the first side pass, which is correct since there the type defs are handled.

%struct.TraceKindPair = type { i8*, i32 }

@_ZL14traceKindNames = internal constant [4 x %struct.TraceKindPair] [%struct.TraceKindPair { i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i32 -1 }, %struct.TraceKindPair { i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i32 0 }, %struct.TraceKindPair { i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i32 1 }, %struct.TraceKindPair { i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i32 3 }], align 4 ; [#uses=3 type=[4 x %struct.TraceKindPair]*]

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

define linkonce_odr hidden i32 @_ZN7mozilla11ArrayLengthIK13TraceKindPairLj4EEEjRAT0__T_([4 x %struct.TraceKindPair]* %arr) nounwind {
entry:
  ret i32 1
}

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  store i32 0, i32* %retval
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0 type=i32]
  %0 = getelementptr inbounds [4 x %struct.TraceKindPair], [4 x %struct.TraceKindPair]* @_ZL14traceKindNames, i32 0, i32 1 ; [#uses=1 type=%struct.TraceKindPair*] [debug line = 1473:17]
  %1 = getelementptr inbounds %struct.TraceKindPair, %struct.TraceKindPair* %0, i32 0, i32 0 ; [#uses=1 type=i8**] [debug line = 1473:17]
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
