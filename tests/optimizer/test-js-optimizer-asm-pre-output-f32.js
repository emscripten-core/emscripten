function badf() {
 var $9 = 0;
 $9 = $8 | 0;
 HEAP32[$gep23_asptr >> 2] = $9;
}
function badf2() {
 var $9 = f0;
 $9 = Math_fround($8);
 HEAPF32[$gep23_asptr >> 2] = $9;
}
function dupe() {
 x = Math_fround(x);
 x = Math_fround(x);
 x = Math_fround(x);
 x = Math_fround(x);
}
function zeros(x) {
 x = Math_fround(x);
 var y = f0;
 print(Math_fround(y) + f0);
 return Math_fround(0);
}

