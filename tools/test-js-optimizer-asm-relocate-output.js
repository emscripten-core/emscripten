function leaveMeAlone(c) {}
function fixed(a, b) {}
function a(x, y) {
 fixed(34, 12);
 fixed(34 | 0, 12 | 0);
 leaveMeAlone(10 + x, 33 + y);
 leaveMeAlone(10 + x | 0, 33 + y | 0);
}

