target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%structy = type { i32, i32, i32 }

@.str = private unnamed_addr constant [24 x i8] c"hello, world! %d %d %d\0A\00", align 1

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

define i32 @main() {
entry:
  call void (%structy (%structy)*) @caller(%structy (%structy)* @structy_user)
  ret i32 0
}
