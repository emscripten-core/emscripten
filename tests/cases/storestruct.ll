; ModuleID = '/dev/shm/tmp/src.cpp.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

; Load and store an entire structure as a whole (and also load as a whole, extract values and save separately, etc.)

%struct.X = type { i32, i32 }

@.str = private unnamed_addr constant [9 x i8] c"*%d,%d*\0A\00" ; [#uses=1]

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1]
  %x = alloca %struct.X, align 4                  ; [#uses=2]
  %y = alloca %struct.X, align 4                  ; [#uses=2]
  store i32 0, i32* %retval
  %a = getelementptr inbounds %struct.X, %struct.X* %x, i32 0, i32 0  store i32 5, i32* %a, align 4  %b = getelementptr inbounds %struct.X, %struct.X* %x, i32 0, i32 1  store i32 22, i32* %b, align 4
  %allx = load %struct.X, %struct.X* %x, align 4  store %struct.X %allx, %struct.X* %y, align 4
  %a1 = getelementptr inbounds %struct.X, %struct.X* %y, i32 0, i32 0  %tmp = load i32, i32* %a1, align 4  %b2 = getelementptr inbounds %struct.X, %struct.X* %y, i32 0, i32 1  %tmp3 = load i32, i32* %b2, align 4  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i32 %tmp, i32 %tmp3)
  store i32 7, i32* %a, align 4  store i32 96, i32* %b, align 4  %allx2 = load %struct.X, %struct.X* %x, align 4
  %x_a = extractvalue %struct.X %allx2, 0            ; [#uses=1]
  store i32 %x_a, i32* %a1, align 4
  %x_b = extractvalue %struct.X %allx2, 1            ; [#uses=1]
  store i32 %x_b, i32* %b2, align 4

  %tmp5 = load i32, i32* %a1, align 4  %tmp6 = load i32, i32* %b2, align 4  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i32 %tmp5, i32 %tmp6)
  %ptr = inttoptr i32 52 to i32*                  ; [#uses=1]
  %ptrgep = getelementptr inbounds i32, i32* %ptr, i32 1
  %ptrgepint = ptrtoint i32* %ptrgep to i32
  %ss1 = insertvalue %struct.X undef, i32 %ptrgepint, 0
  %ss2 = insertvalue %struct.X %ss1, i32 3, 1
  store %struct.X %ss2, %struct.X* %y, align 4 ; store entire struct at once

  %tmp5b = load i32, i32* %a1, align 4  %tmp6b = load i32, i32* %b2, align 4  %call3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i32 %tmp5b, i32 %tmp6b)
  ret i32 0}

; [#uses=1]
declare i32 @printf(i8*, ...)


