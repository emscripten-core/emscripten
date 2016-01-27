target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%structy = type { i32, i32, i32 }

@.str = private unnamed_addr constant [24 x i8] c"hello, world! %d %d %d\0A\00", align 1

@vtable38087 = internal unnamed_addr constant { void (i32, {})* } { void (i32, {})* @just_nil }, align 4

declare i32 @printf(i8*, ...)

define void @print_structy(%structy %s) {
entry:
  %x = extractvalue %structy %s, 0
  %y = extractvalue %structy %s, 1
  %z = extractvalue %structy %s, 2
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 %x, i32 %y, i32 %z)
  ret void
}

define %structy @structy_user(%structy %s) {
entry:
  call void (%structy) @print_structy(%structy %s)
  ret %structy %s
}

define void @caller(%structy (%structy)* %x) {
entry:
  %temp = call %structy (%structy) %x(%structy { i32 0, i32 1, i32 5 })
  call void (%structy) @print_structy(%structy %temp)
  ret void
}

define void @print_structy_nil(%structy %s, {} %nil) {
entry:
  %x = extractvalue %structy %s, 0
  %y = extractvalue %structy %s, 1
  %z = extractvalue %structy %s, 2
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 %x, i32 %y, i32 %z)
  ret void
}

define %structy @structy_user_nil(%structy %s, {} %nil) {
entry:
  call void (%structy, {}) @print_structy_nil(%structy %s, {} undef)
  ret %structy %s
}

define void @caller_nil(%structy (%structy, {})* %x, {} %nil) {
entry:
  %temp = call %structy (%structy, {}) %x(%structy { i32 9, i32 15, i32 77 }, {} undef)
  call void (%structy, {}) @print_structy_nil(%structy %temp, {} undef)
  ret void
}

define void @just_nil(i32 %w, {} %nil) {
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 10, i32 21, i32 %w)
  ret void
}

declare i1 @llvm.expect.i1(i1, i1)

define internal void @rust_begin_unwind([6 x i32], [2 x i32], i32) unnamed_addr #4 {
"the block":
  %3 = alloca [6 x i32]
  store [6 x i32] %0, [6 x i32]* %3
  %4 = bitcast [6 x i32]* %3 to i32*
  %5 = alloca [2 x i32]
  store [2 x i32] %1, [2 x i32]* %5
  %6 = getelementptr inbounds [2 x i32], [2 x i32]* %5, i32 0, i32 0
  %7 = bitcast i32* %6 to i8**
  %8 = load i8*, i8** %7
  %9 = getelementptr inbounds [2 x i32], [2 x i32]* %5, i32 0, i32 1
  %10 = load i32, i32* %9
  %11 = alloca i32
  store i32 %2, i32* %11
  %12 = load i32, i32* %11
  ret void
}

define i32 @main() {
entry:
  call void @just_nil(i32 33, {} undef)

  %fp1 = bitcast void (i32, {})* @just_nil to void (i32)*
  %fp2 = bitcast void (i32)*     %fp1      to void (i32, {})*
  call void (i32, {}) %fp2(i32 44, {} undef)

  call void (%structy (%structy)*) @caller(%structy (%structy)* @structy_user)

  call void (%structy (%structy, {})*, {}) @caller_nil(%structy (%structy, {})* @structy_user_nil, {} undef)

  %x = getelementptr inbounds { void (i32, {})* }, { void (i32, {})* }* @vtable38087, i32 0, i32 0
  %y = ptrtoint void (i32, {})** %x to i32

  %y1 = trunc i32 %y to i1
  %z = call i1 @llvm.expect.i1(i1 %y1, i1 false)
  %z32 = zext i1 %z to i32

  call void ([6 x i32], [2 x i32], i32) @rust_begin_unwind([6 x i32] undef, [2 x i32] undef, i32 0)

  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i32 -12, i32 11, i32 -10)

  ret i32 %z32
}
