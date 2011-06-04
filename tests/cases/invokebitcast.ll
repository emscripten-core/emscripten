; ModuleID = '/dev/shm/tmp/src.cpp.o'
; Just test for compilation here
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-f128:128:128-n8:16:32"
target triple = "i386-pc-linux-gnu"

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
  %1 = load i32* bitcast (i32* getelementptr inbounds (%struct.CPU_Regs* @cpu_regs, i32 0, i32 0, i32 1, i32 0, i32 0) to i32*), align 2 ; [#uses=1]
  store i16 %1, i16* bitcast (%struct.CPU_Regs* @cpu_regs to i16*), align 2
  %2 = call i32 @puts(i8* getelementptr inbounds ([14 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  store i32 0, i32* %0, align 4
  %3 = load i32* %0, align 4                      ; [#uses=1]
  store i32 %3, i32* %retval, align 4
  br label %return

  invoke void bitcast (void (i32*, i32)* @_Z8toStringj to void (i64*, i32)*)(%struct.CPU_Regs* noalias @cpu_regs, i32 %99)
          to label %invcont33 unwind label %lpad106

invcont33:
  ret i32 %retval1

lpad106:
  ret i32 %retval1

return:                                           ; preds = %entry
  %retval1 = load i32* %retval                    ; [#uses=1]
  ret i32 %retval1
}

; [#uses=1]
declare i32 @puts(i8*)
