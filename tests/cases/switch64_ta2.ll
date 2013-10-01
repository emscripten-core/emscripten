@.str = private constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1]

define linkonce_odr i32 @main() align 2 {
  %333 = call i32 @printf(i8* getelementptr inbounds ([15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  %444 = zext i32 %333 to i64
  %199 = trunc i8 1 to i1                        ; [#uses=1]
  switch i64 %444, label %label999 [
    i64 1000, label %label9950
    i64 1001, label %label9951
    i64 1002, label %label9952
    i64 1003, label %label9953
    i64 1004, label %label9954
    i64 1005, label %label9955
    i64 1006, label %label9956
    i64 1007, label %label9957
    i64 1008, label %label9958
    i64 1009, label %label9959
  ] ; switch should ignore all code after it in the block
                                              ; No predecessors!
  %a472 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          cleanup
  %a473 = extractvalue { i8*, i32 } %a472, 0
  %a474 = extractvalue { i8*, i32 } %a472, 1
  br label %label999

label9950:
  %333b = call i32 @printf(i8* getelementptr inbounds ([15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0]
  br label %label999

label9951:
  br label %label999
label9952:
  br label %label999
label9953:
  br label %label999
label9954:
  br label %label999
label9955:
  br label %label999
label9956:
  br label %label999
label9957:
  br label %label999
label9958:
  br label %label999
label9959:
  br label %label999

label999:                                     ; preds = %555
  ret i32 0
}

declare i32 @printf(i8*)
declare i32 @__gxx_personality_v0(...)

