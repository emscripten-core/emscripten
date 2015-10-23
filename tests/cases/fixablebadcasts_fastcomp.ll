; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [18 x i8] c"hello, world %d!\0A\00", align 1

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  store i32 0, i32* %retval
  %a = call i32 bitcast (i32 (i32)* @twoparam to i32 (i32, i32)*)(i32 5, i32 6)
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i32 %a) ; [#uses=0 type=i32]
  call void bitcast (void (i32, i32*, i32*)* @_ZN7WebCore33signedPublicKeyAndChallengeStringEjRKN3WTF6StringERKNS_3URLE to void (i32*, i32, i32*, i32*)*)(i32* sret null, i32 0, i32* null, i32* null)
  ret i32 1
}

define i32 @twoparam(i32 %x) {
  ret i32 %x
}

define void @_ZN7WebCore33signedPublicKeyAndChallengeStringEjRKN3WTF6StringERKNS_3URLE(i32, i32* nocapture, i32* nocapture) {
  ret void
}

; [#uses=1]
declare i32 @printf(i8*, ...)
