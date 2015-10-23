; ModuleID = '/dev/shm/tmp/src.cpp.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%struct.CPU_Regs = type { [8 x %union.GenReg32] }
%union.GenReg32 = type { [1 x i32] }

@cpu_regs = unnamed_addr global %struct.CPU_Regs zeroinitializer, align 32 ; [#uses=2]
@.str = private unnamed_addr constant [14 x i8] c"hello, world!\00", align 1 ; [#uses=1]

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32                            ; [#uses=2]
  %0 = alloca i32                                 ; [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; [#uses=0]
  %1 = load i32, i32* bitcast (i32* getelementptr inbounds (%struct.CPU_Regs, %struct.CPU_Regs* @cpu_regs, i32 0, i32 0, i32 1, i32 0, i32 0) to i32*), align 2 ; [#uses=1]
  %b = trunc i32 %1 to i16
  store i16 %b, i16* bitcast (%struct.CPU_Regs* @cpu_regs to i16*), align 2
  %2 = call i32 @puts(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  store i32 0, i32* %0, align 4
  %3 = load i32, i32* %0, align 4                      ; [#uses=1]
  store i32 %3, i32* %retval, align 4
  br label %return

return:                                           ; preds = %entry
  %retval1 = load i32, i32* %retval                    ; [#uses=1]
  ret i32 %retval1
}

; [#uses=1]
declare i32 @puts(i8*)
