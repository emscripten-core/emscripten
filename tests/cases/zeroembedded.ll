target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%struct.pypy_str = type { i32, [0 x i8] }
%struct.pypy_strval = type { i32, [13 x i8] }

%union.pypy_array3_len0u = type { %struct.pypy_array3_len0 }
%struct.pypy_array3_len0 = type { i32, i32, [0 x i8] }

@pypy_g_strval = global %struct.pypy_strval { i32 13, [13 x i8] c"hello world\0A\00" }
@pypy_g_strval2 = global %struct.pypy_array3_len0 { i32 13, i32 111, [0 x i8] c"" }

declare i32 @printf(i8*, ...)

define i32 @main(i32 %argc, i8** nocapture %argv)  {
  %waka = alloca %struct.pypy_array3_len0
  %1 = bitcast %struct.pypy_strval* @pypy_g_strval to %struct.pypy_str*
  %2 = getelementptr inbounds %struct.pypy_str, %struct.pypy_str* %1, i32 1
  %3 = bitcast %struct.pypy_str* %2 to i8*
  call i32 (i8*, ...) @printf(i8* %3)
  %unneeded = bitcast %struct.pypy_str* %2 to %struct.pypy_array3_len0*
  call i32 (i8*, ...) @printf(i8* %3, %struct.pypy_array3_len0* %unneeded)
  ret i32 0
}

