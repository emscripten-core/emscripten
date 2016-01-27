; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@globaliz = global [300 x i8] zeroinitializer

define i64 @retter(i64 %x) {
  store i128 0, i128* bitcast ([300 x i8]* @globaliz to i128*), align 4 ; wipe it out
  store i64 %x, i64* bitcast ([300 x i8]* @globaliz to i64*), align 4
  call i32 (i8*) @puts(i8* bitcast ([300 x i8]* @globaliz to i8*))
  ret i64 7017280452245743464
}

define i32 @main() personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %buffer = alloca i8, i32 1000, align 4
  %bundled = bitcast i8* %buffer to i128*
  store i128 8583909746840200552, i128* %bundled, align 4 ; hello world in there
  call i32 (i8*) @puts(i8* %buffer)

  %loaded = load i128, i128* %bundled, align 4 ; save for later

  %backcast = bitcast i128* %bundled to i8*
  call i32 (i8*) @puts(i8* %backcast)

  %temp.buffer = bitcast i8* %buffer to [0 x i8]*
  %buffer1 = getelementptr [0 x i8], [0 x i8]* %temp.buffer, i32 0, i32 1
  %bundled1 = bitcast i8* %buffer1 to i128*
  store i128 2414941923668160609, i128* %bundled1, align 1 ; unaligned
  call i32 (i8*) @puts(i8* %buffer)

; shifts
  %shifted = lshr i128 %loaded, 16
  store i128 %shifted, i128* %bundled, align 4
  call i32 (i8*) @puts(i8* %buffer)
  %shifted2 = lshr i128 %loaded, 32
  store i128 %shifted2, i128* %bundled, align 4
  call i32 (i8*) @puts(i8* %buffer)

; store %loaded, make sure has not been modified
  store i128 %loaded, i128* %bundled, align 4
  call i32 (i8*) @puts(i8* %buffer)

  %shifted3 = shl i128 %loaded, 8
  store i128 %shifted3, i128* %bundled, align 4
  store i8 113, i8* %buffer ; remove initial 0 ; 'q'
  call i32 (i8*) @puts(i8* %buffer)

; trunc
  %shifted4 = shl i128 %loaded, 64
  store i128 %shifted4, i128* %bundled, align 4
  %nonzero64 = trunc i128 %loaded to i64 ; remove initial zeros
  %bundled64 = bitcast i128* %bundled to i64*
  store i64 %nonzero64, i64* %bundled64, align 4
;  call i32 (i8*) @puts(i8* %buffer)

  store i128 0, i128* %bundled, align 4 ; wipe it out
  %small32 = trunc i128 %loaded to i32
  %buffer32 = bitcast i8* %buffer to i32*
  store i32 %small32, i32* %buffer32, align 4
;  call i32 (i8*) @puts(i8* %buffer)

  store i128 0, i128* %bundled, align 4 ; wipe it out
  %small16 = trunc i128 %loaded to i16
  %buffer16 = bitcast i8* %buffer to i16*
  store i16 %small16, i16* %buffer16, align 4
  call i32 (i8*) @puts(i8* %buffer)

  store i128 0, i128* %bundled, align 4 ; wipe it out
  %small64 = trunc i128 %loaded to i64
  %buffer64 = bitcast i8* %buffer to i64*
  store i64 %small64, i64* %buffer64, align 4
  call i32 (i8*) @puts(i8* %buffer)

; zext
  store i128 0, i128* %bundled, align 4 ; wipe it out
  %pre32 = or i32 6382179, 0
  %big = zext i32 %pre32 to i128
  store i128 %big, i128* %bundled, align 4
  call i32 (i8*) @puts(i8* %buffer)

  store i128 0, i128* %bundled, align 4 ; wipe it out
  %pre64 = zext i32 1684366951 to i64
  %post64 = shl i64 %pre64, 32
  %big64 = or i64 %pre64, %post64
  %bigb = zext i64 %big64 to i128
  store i128 %bigb, i128* %bundled, align 4
  call i32 (i8*) @puts(i8* %buffer)

; or, and, xor
  %ored = or i128 %loaded, 107752139522048 ; constant
  store i128 %ored, i128* %bundled, align 4
  call i32 (i8*) @puts(i8* %buffer)

  %ander = add i128 18402271027389267967, 0
  %anded = and i128 %loaded, %ander ; variable
  store i128 %anded, i128* %bundled, align 4
  call i32 (i8*) @puts(i8* %buffer)

  %xored = xor i128 %loaded, 0
  store i128 %xored, i128* %bundled, align 4
  call i32 (i8*) @puts(i8* %buffer)

; unfolding
  store i128 %loaded, i128* bitcast ([300 x i8]* @globaliz to i128*), align 4
  %loaded.short = load i96, i96* bitcast ([300 x i8]* @globaliz to i96*), align 4
  store i128 0, i128* bitcast ([300 x i8]* @globaliz to i128*), align 4
  store i96 %loaded.short, i96* bitcast ([300 x i8]* @globaliz to i96*), align 4
  call i32 (i8*) @puts(i8* bitcast ([300 x i8]* @globaliz to i8*))

; phi
  %if = trunc i128 %ander to i1
  %first = trunc i128 %xored to i96
  br i1 %if, label %a17, label %a26

a17:
  %second = trunc i128 %loaded to i96
  br label %a26

a26:
  %a27 = phi i96 [ %first, %entry ], [ %second, %a17 ]
  store i128 0, i128* %bundled, align 4 ; wipe it out
  store i96 %a27, i96* bitcast ([300 x i8]* @globaliz to i96*), align 4
  call i32 (i8*) @puts(i8* bitcast ([300 x i8]* @globaliz to i8*))

; phi with constants
  br i1 %if, label %a17b, label %a26b

a17b:
  br label %a26b

a26b:
  %a27b = phi i64 [ 55, %a26 ], [ 57, %a17b ]
  store i128 0, i128* %bundled, align 4 ; wipe it out
  store i64 %a27b, i64* bitcast ([300 x i8]* @globaliz to i64*), align 4
  call i32 (i8*) @puts(i8* bitcast ([300 x i8]* @globaliz to i8*))

  store i128 %ored, i128* %bundled, align 4
  %iff = zext i1 %if to i64
  switch i64 %iff, label %a50 [
    i64 1, label %a30
    i64 0, label %a40
  ]

a50:
  store i128 %xored, i128* %bundled, align 4
  br label %a40

a30:
  store i128 %anded, i128* %bundled, align 4
  br label %a40

a40:
  call i32 (i8*) @puts(i8* %buffer)

; invoke return value

  %inv64 = invoke i64 @retter(i64 8174723217654970232)
            to label %a100 unwind label %a111

a100:
  store i128 0, i128* bitcast ([300 x i8]* @globaliz to i128*), align 4 ; wipe it out
  store i64 %inv64, i64* bitcast ([300 x i8]* @globaliz to i64*), align 4
  call i32 (i8*) @puts(i8* bitcast ([300 x i8]* @globaliz to i8*))
  br label %done

a111:
  %aaaa79 = landingpad { i8*, i32 }
          cleanup
  br label %done

done:
  ret i32 1
}

declare i32 @puts(i8*)
declare i32 @__gxx_personality_v0(...)
declare void @__cxa_throw(i32, i32, i32) ; for asm1, where exceptions are enabled but this test needs a throw to bring in lib stuff

