.text
.global sqrt
.type   sqrt,%function
sqrt:
	vsqrt.f64 d0, d0
	bx lr
