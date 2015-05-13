; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%struct.pair = type { [5 x i8], [5 x i8] }

@.str = private unnamed_addr constant [6 x i8] c"|%d|\0A\00", align 1
@.str1 = private unnamed_addr constant [7 x i8] c"%s,%s\0A\00", align 1

define i32 @main() {
  %1 = alloca i32, align 4
  %pp = alloca [2 x i40], align 8
  %p = bitcast [2 x i40]* %pp to %struct.pair*
  store i32 0, i32* %1
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i32 10)
  %3 = bitcast %struct.pair* %p to i8*
  call void @llvm.memset.p0i8.i32(i8* %3, i8 120, i32 10, i32 1, i1 false)
  %4 = getelementptr inbounds [2 x i40], [2 x i40]* %pp, i32 0, i32 0
  %b4 = bitcast i40* %4 to [5 x i8]*
  %5 = getelementptr inbounds [5 x i8], [5 x i8]* %b4, i32 0, i32 2
  store i8 97, i8* %5, align 1
  %6 = getelementptr inbounds %struct.pair, %struct.pair* %p, i32 0, i32 0
  %7 = getelementptr inbounds [5 x i8], [5 x i8]* %6, i32 0, i32 4
  store i8 0, i8* %7, align 1
  %8 = getelementptr inbounds %struct.pair, %struct.pair* %p, i32 0, i32 1
  %9 = getelementptr inbounds [5 x i8], [5 x i8]* %8, i32 0, i32 3
  store i8 98, i8* %9, align 1
  %10 = getelementptr inbounds [2 x i40], [2 x i40]* %pp, i32 0, i32 1
  %b10 = bitcast i40* %10 to [5 x i8]*
  %11 = getelementptr inbounds [5 x i8], [5 x i8]* %b10, i32 0, i32 4
  store i8 0, i8* %11, align 1
  %12 = getelementptr inbounds %struct.pair, %struct.pair* %p, i32 0, i32 0
  %13 = getelementptr inbounds [5 x i8], [5 x i8]* %12, i32 0, i32 0
  %14 = getelementptr inbounds %struct.pair, %struct.pair* %p, i32 0, i32 1
  %15 = getelementptr inbounds [5 x i8], [5 x i8]* %14, i32 0, i32 0
  %16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str1, i32 0, i32 0), i8* %13, i8* %15)
  ret i32 0
}

declare i32 @printf(i8*, ...)

declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) nounwind
