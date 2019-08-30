.text
.global fabs
.type   fabs,%function
fabs:
	vabs.f64 d0, d0
	bx lr
