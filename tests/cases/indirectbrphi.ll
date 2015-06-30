; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@_ZZ4mainE5addrs = private unnamed_addr constant [2 x i8*] [i8* blockaddress(@main, %14), i8* blockaddress(@main, %19)], align 4
@.str = private unnamed_addr constant [8 x i8] c"bad %d\0A\00", align 1
@.str1 = private unnamed_addr constant [9 x i8] c"good %d\0A\00", align 1

define i32 @main(i32 %argc, i8** nocapture %argv) nounwind {
  %1 = add i32 %argc, 12
  %2 = mul i32 %1, %argc
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %.lr.ph, label %8

.lr.ph:                                           ; preds = %.lr.ph, %0
  %which.010 = phi i32 [ %6, %.lr.ph ], [ 0, %0 ]
  %x.09 = phi i32 [ %7, %.lr.ph ], [ 0, %0 ]
  %4 = mul nsw i32 %x.09, %x.09
  %5 = add nsw i32 %which.010, %4
  %6 = srem i32 %5, 7
  %7 = add nsw i32 %x.09, 1
  %exitcond = icmp eq i32 %7, %2
  br i1 %exitcond, label %._crit_edge, label %.lr.ph

._crit_edge:                                      ; preds = %.lr.ph
  %phitmp = srem i32 %6, 2
  %phitmp11 = add i32 %phitmp, 1
  br label %8

; <label>:8                                       ; preds = %._crit_edge, %0
  %which.0.lcssa = phi i32 [ %phitmp11, %._crit_edge ], [ 1, %0 ]
  %9 = icmp eq i32 %argc, 1121
  br i1 %9, label %14, label %10

; <label>:10                                      ; preds = %8
  %11 = getelementptr inbounds [2 x i8*], [2 x i8*]* @_ZZ4mainE5addrs, i32 0, i32 %which.0.lcssa
  %12 = load i8*, i8** %11, align 4
  %13 = add nsw i32 %argc, 111
  br label %17

; <label>:14                                      ; preds = %17, %8
  %15 = phi i32 [ 100, %17 ], [ 222, %8 ]
  %16 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 %15)
  ret i32 0

; <label>:17                                      ; preds = %19, %10
  %18 = phi i8* [ %12, %10 ], [ blockaddress(@main, %14), %19 ]
  indirectbr i8* %18, [label %14, label %19]

; <label>:19                                      ; preds = %17
  %20 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str1, i32 0, i32 0), i32 %13)
  br label %17
}

declare i32 @printf(i8* nocapture, ...) nounwind

define i8* @memcpy(i8* noalias %dest, i8* noalias %src, i32 %n) nounwind {
  %1 = ptrtoint i8* %dest to i32
  %2 = ptrtoint i8* %src to i32
  %3 = xor i32 %2, %1
  %4 = and i32 %3, 3
  %5 = icmp eq i32 %4, 0
  br i1 %5, label %.preheader28, label %.preheader

.preheader28:                                     ; preds = %0
  %6 = and i32 %1, 3
  %7 = icmp eq i32 %6, 0
  %8 = icmp eq i32 %n, 0
  %or.cond29 = or i1 %7, %8
  br i1 %or.cond29, label %.critedge, label %.lr.ph33

.lr.ph33:                                         ; preds = %.lr.ph33, %.preheader28
  %s.032 = phi i8* [ %9, %.lr.ph33 ], [ %src, %.preheader28 ]
  %d.031 = phi i8* [ %11, %.lr.ph33 ], [ %dest, %.preheader28 ]
  %.030 = phi i32 [ %12, %.lr.ph33 ], [ %n, %.preheader28 ]
  %9 = getelementptr inbounds i8, i8* %s.032, i32 1
  %10 = load i8, i8* %s.032, align 1
  %11 = getelementptr inbounds i8, i8* %d.031, i32 1
  store i8 %10, i8* %d.031, align 1
  %12 = add i32 %.030, -1
  %13 = ptrtoint i8* %11 to i32
  %14 = and i32 %13, 3
  %15 = icmp eq i32 %14, 0
  %16 = icmp eq i32 %12, 0
  %or.cond = or i1 %15, %16
  br i1 %or.cond, label %.critedge, label %.lr.ph33

.critedge:                                        ; preds = %.lr.ph33, %.preheader28
  %.lcssa = phi i1 [ %8, %.preheader28 ], [ %16, %.lr.ph33 ]
  %s.0.lcssa = phi i8* [ %src, %.preheader28 ], [ %9, %.lr.ph33 ]
  %d.0.lcssa = phi i8* [ %dest, %.preheader28 ], [ %11, %.lr.ph33 ]
  %.0.lcssa = phi i32 [ %n, %.preheader28 ], [ %12, %.lr.ph33 ]
  br i1 %.lcssa, label %.loopexit, label %17

; <label>:17                                      ; preds = %.critedge
  %18 = bitcast i8* %d.0.lcssa to i32*
  %19 = bitcast i8* %s.0.lcssa to i32*
  %20 = icmp ugt i32 %.0.lcssa, 3
  br i1 %20, label %.lr.ph25, label %._crit_edge

.lr.ph25:                                         ; preds = %.lr.ph25, %17
  %ws.024 = phi i32* [ %21, %.lr.ph25 ], [ %19, %17 ]
  %wd.023 = phi i32* [ %23, %.lr.ph25 ], [ %18, %17 ]
  %.122 = phi i32 [ %24, %.lr.ph25 ], [ %.0.lcssa, %17 ]
  %21 = getelementptr inbounds i32, i32* %ws.024, i32 1
  %22 = load i32, i32* %ws.024, align 4
  %23 = getelementptr inbounds i32, i32* %wd.023, i32 1
  store i32 %22, i32* %wd.023, align 4
  %24 = add i32 %.122, -4
  %25 = icmp ugt i32 %24, 3
  br i1 %25, label %.lr.ph25, label %._crit_edge

._crit_edge:                                      ; preds = %.lr.ph25, %17
  %ws.0.lcssa = phi i32* [ %19, %17 ], [ %21, %.lr.ph25 ]
  %wd.0.lcssa = phi i32* [ %18, %17 ], [ %23, %.lr.ph25 ]
  %.1.lcssa = phi i32 [ %.0.lcssa, %17 ], [ %24, %.lr.ph25 ]
  %26 = bitcast i32* %wd.0.lcssa to i8*
  %27 = bitcast i32* %ws.0.lcssa to i8*
  br label %.preheader

.preheader:                                       ; preds = %._crit_edge, %0
  %.2.ph = phi i32 [ %n, %0 ], [ %.1.lcssa, %._crit_edge ]
  %d.1.ph = phi i8* [ %dest, %0 ], [ %26, %._crit_edge ]
  %s.1.ph = phi i8* [ %src, %0 ], [ %27, %._crit_edge ]
  %28 = icmp eq i32 %.2.ph, 0
  br i1 %28, label %.loopexit, label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph, %.preheader
  %s.121 = phi i8* [ %29, %.lr.ph ], [ %s.1.ph, %.preheader ]
  %d.120 = phi i8* [ %31, %.lr.ph ], [ %d.1.ph, %.preheader ]
  %.219 = phi i32 [ %32, %.lr.ph ], [ %.2.ph, %.preheader ]
  %29 = getelementptr inbounds i8, i8* %s.121, i32 1
  %30 = load i8, i8* %s.121, align 1
  %31 = getelementptr inbounds i8, i8* %d.120, i32 1
  store i8 %30, i8* %d.120, align 1
  %32 = add i32 %.219, -1
  %33 = icmp eq i32 %32, 0
  br i1 %33, label %.loopexit, label %.lr.ph

.loopexit:                                        ; preds = %.lr.ph, %.preheader, %.critedge
  ret i8* %dest
}
