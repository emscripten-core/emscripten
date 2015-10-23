; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [17 x i8] c"hello %d world!\0A\00", align 1 ; [#uses=1 type=[17 x i8]*]

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  store i32 0, i32* %retval
  %access_virt_barray = inttoptr i32 100 to [64 x i16]* (i32*, i32)**
  store [64 x i16]* (i32*, i32)* @access_virt_barray, [64 x i16]* (i32*, i32)** %access_virt_barray, align 4
  %wakaptr = bitcast [64 x i16]* (i32*, i32)** %access_virt_barray to i32*
  %waka = load i32, i32* %wakaptr
  %waka2 = icmp eq i32 %waka, 0
  %waka3 = zext i1 %waka2 to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i32 0, i32 0), i32 %waka3) ; [#uses=0 type=i32]
  ret i32 1
}

define [64 x i16]* @access_virt_barray(i32*, i32) {
  ret [64 x i16]* inttoptr (i32 0 to [64 x i16]*)
}

; [#uses=1]
declare i32 @printf(i8*, ...)
