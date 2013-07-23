; a.ll
%struct.pypy_str = type { i32, [0 x i8] }
%struct.pypy_strval = type { i32, [13 x i8] }

@pypy_g_strval = global %struct.pypy_strval { i32 13, [13 x i8] c"hello world\0A\00" }

declare i32 @printf(i8*, ...)

define i32 @main(i32 %argc, i8** nocapture %argv)  {
  %1 = bitcast %struct.pypy_strval* @pypy_g_strval to %struct.pypy_str*
  %2 = getelementptr inbounds %struct.pypy_str* %1, i32 1
  %3 = bitcast %struct.pypy_str* %2 to i8*
  call i32 (i8*, ...)* @printf(i8* %3)
  ret i32 0
}

