; ModuleID = '/tmp/tmpt0JpDh/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [7 x i8] c"20\0A91\0A\00", align 1
@.str1 = private unnamed_addr constant [6 x i8] c"%u %u\00", align 1
@.str2 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

define i32 @main() nounwind {
  %j = alloca i32, align 4
  %k4 = alloca i32, align 4
  %t = alloca [100 x [100 x i32]], align 4
  store i32 20, i32* %j, align 4
  store i32 91, i32* %k4, align 4
  br label %.lr.ph.i

.lr.ph.i:                                         ; preds = %.lr.ph.i, %0
  %j.07.i = phi i32 [ %7, %.lr.ph.i ], [ 0, %0 ]
  %1 = and i32 %j.07.i, 1
  %2 = icmp eq i32 %1, 0
  %3 = sub i32 0, %j.07.i
  %.p.i = select i1 %2, i32 %j.07.i, i32 %3
  %4 = add i32 %.p.i, 8
  %5 = urem i32 %4, 101
  %6 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %t, i32 0, i32 0, i32 %j.07.i
  store i32 %5, i32* %6, align 4
  %7 = add i32 %j.07.i, 1
  %8 = icmp ult i32 %7, 10000
  br i1 %8, label %.lr.ph.i, label %init.exit

init.exit:                                        ; preds = %.lr.ph.i
  %9 = call i32 (i8*, i8*, ...) @sscanf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str1, i32 0, i32 0), i32* %j, i32* %k4) nounwind
  store i32 53, i32* %j, align 4
  br label %.preheader

.preheader:                                       ; preds = %29, %init.exit
  %indvars.iv23 = phi i32 [ -29, %init.exit ], [ %indvars.iv.next24, %29 ]
  %indvars.iv21 = phi i32 [ -28, %init.exit ], [ %indvars.iv.next22, %29 ]
  %indvars.iv19 = phi i32 [ -27, %init.exit ], [ %indvars.iv.next20, %29 ]
  %indvars.iv17 = phi i32 [ 109, %init.exit ], [ %indvars.iv.next18, %29 ]
  %indvars.iv15 = phi i32 [ -75923, %init.exit ], [ %indvars.iv.next16, %29 ]
  %indvars.iv13 = phi i32 [ 5593, %init.exit ], [ %indvars.iv.next14, %29 ]
  %indvars.iv = phi i32 [ -262, %init.exit ], [ %indvars.iv.next, %29 ]
  %10 = phi i32 [ 53, %init.exit ], [ %.pre-phi, %29 ]
  %11 = zext i32 %indvars.iv19 to i33
  %12 = zext i32 %indvars.iv21 to i33
  %13 = mul i33 %11, %12
  %14 = icmp ult i32 %10, 27
  br i1 %14, label %.lr.ph, label %.preheader._crit_edge

.preheader._crit_edge:                            ; preds = %.preheader
  %.pre = add i32 %10, -1
  br label %29

.lr.ph:                                           ; preds = %.preheader
  %15 = zext i32 %indvars.iv23 to i33
  %16 = mul i33 %13, %15
  %17 = lshr i33 %16, 1
  %18 = lshr i33 %13, 1
  %19 = trunc i33 %17 to i32
  %20 = trunc i33 %18 to i32
  %21 = mul i32 %19, 1431655766
  %22 = mul i32 %indvars.iv17, %20
  %23 = add i32 %10, -1
  %24 = add i32 %10, 1
  %25 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %t, i32 0, i32 %24, i32 %23
  %.promoted = load i32, i32* %25, align 4
  %26 = add i32 %.promoted, %indvars.iv15
  %27 = add i32 %26, %22
  %28 = add i32 %27, %21
  store i32 %28, i32* %25, align 4
  br label %29

.lr.ph.i3.preheader:                              ; preds = %29
  store i32 2, i32* %j, align 4
  store i32 %.lcssa69, i32* %k4, align 4
  br label %.lr.ph.i3

; <label>:29                                      ; preds = %.lr.ph, %.preheader._crit_edge
  %.pre-phi = phi i32 [ %.pre, %.preheader._crit_edge ], [ %23, %.lr.ph ]
  %.lcssa69 = phi i32 [ %10, %.preheader._crit_edge ], [ 27, %.lr.ph ]
  %30 = icmp ugt i32 %.pre-phi, 2
  %indvars.iv.next = add i32 %indvars.iv, 6
  %indvars.iv.next14 = add i32 %indvars.iv13, %indvars.iv
  %indvars.iv.next16 = add i32 %indvars.iv15, %indvars.iv13
  %indvars.iv.next18 = add i32 %indvars.iv17, -2
  %indvars.iv.next20 = add i32 %indvars.iv19, 1
  %indvars.iv.next22 = add i32 %indvars.iv21, 1
  %indvars.iv.next24 = add i32 %indvars.iv23, 1
  br i1 %30, label %.preheader, label %.lr.ph.i3.preheader

.lr.ph.i3:                                        ; preds = %.lr.ph.i3, %.lr.ph.i3.preheader
  %sum.07.i = phi i32 [ %37, %.lr.ph.i3 ], [ 0, %.lr.ph.i3.preheader ]
  %j.06.i = phi i32 [ %38, %.lr.ph.i3 ], [ 0, %.lr.ph.i3.preheader ]
  %31 = and i32 %j.06.i, 1
  %32 = icmp eq i32 %31, 0
  %33 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %t, i32 0, i32 0, i32 %j.06.i
  %34 = load i32, i32* %33, align 4
  %35 = sub i32 0, %34
  %36 = select i1 %32, i32 %34, i32 %35
  %37 = add i32 %36, %sum.07.i
  %38 = add i32 %j.06.i, 1
  %39 = icmp ult i32 %38, 10000
  br i1 %39, label %.lr.ph.i3, label %checkSum.exit

checkSum.exit:                                    ; preds = %.lr.ph.i3
  %40 = add i32 %37, 2
  %41 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str2, i32 0, i32 0), i32 %40) nounwind
  ret i32 0
}

declare i32 @sscanf(i8* nocapture, i8* nocapture, ...) nounwind

declare i32 @printf(i8* nocapture, ...) nounwind
