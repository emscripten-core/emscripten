; ModuleID = 'new.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

declare i32 @printf(i8* noalias, ...) nounwind

@x = common global [4194304 x i8] zeroinitializer, align 4
@.str = private constant [6 x i8] c"*%d*\0A\00", align 1

define i8* @test_gep(i32 %y) nounwind readnone {
  ; JavaScript uses double precision 64-bit floating point values, with
  ; a 53 bit mantissa. The maximum precisely representable integer is
  ; 9007199254740992. A number close to that limit is constructed here
  ; for the constant part of the getelementptr instruction:
  ; 4194304 * 2147483647 == 9007199250546688 == 9007199254740992 - 4194304
  ; If that number appears in JavaScript source instead of being properly
  ; limited to 32 bits, the %y parameter can be used to exceed the maximum 
  ; precisely representable integer, and make the computation inexact.
  %test_res = getelementptr [4194304 x i8], [4194304 x i8]* @x, i32 2147483647, i32 %y
  ret i8* %test_res
}

define i32 @main() {
  %res_0 = call i8* (i32) @test_gep(i32 1000000000)
  %res_1 = call i8* (i32) @test_gep(i32 1000000001)
  %res_0_i = ptrtoint i8* %res_0 to i32
  %res_1_i = ptrtoint i8* %res_1 to i32

  ; If getelementptr limited the constant part of the offset to 32 bits,
  ; result will be 1. Otherwise, it cannot be 1 because the large numbers in
  ; the calculation cannot be accurately represented by floating point math.
  %res_diff = sub i32 %res_1_i, %res_0_i
  %printf_res = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i32 %res_diff)

  ret i32 0
}

