function asmModule($a,$b,$c){'use asm';
var a=new $a.Int8Array($c);
var b=new $a.Uint8Array($c);
var c=new $a.Int16Array($c);
var d=new $a.Uint16Array($c);
var e=new $a.Int32Array($c);
var f=new $a.Uint32Array($c);
var g=new $a.Float32Array($c);
var h=new $a.Float64Array($c);
var i=$a.Math.imul;
var j=$a.Math.fround;
var $d=$a.Math.acos;
var $e=$a.Math.asin;
var $f=$a.Math.atan;
var $g=$a.Math.cos;
var $h=$a.Math.sin;
var $i=$a.Math.tan;
var $j=$a.Math.exp;
var $k=$a.Math.log;
var $l=$a.Math.ceil;
var $m=$a.Math.floor;
var $n=$a.Math.sqrt;
var $o=$a.Math.abs;
var $p=$a.Math.min;
var $q=$a.Math.max;
var $r=$a.Math.atan2;
var $s=$a.Math.pow;
var $t=$a.Math.clz32;
var $u=$a.NaN;
var $v=$a.Infinity;
function $w(){
var k=0.;
k=+$x();
k=+$x()+4.;
k=4.+ +$x();
$y();
return 1;
}
function $x(){
var k=0;
k=$w()|0;
k=($w()|0)+4|0;
k=($y(),1);
return 1.2;
}
function $y(){
var k=0,l=j(0);
+$z(1);
+$z($A(k+1|0,+$z(1)+1.)|0);
}
function $z(k){
k=k|0;
return +$x();
}
function $A(k,l){
k=k|0;l=+l;
return k|0;
}
function $B(k){
k=j(k);
j($B(k));
j($B(j($B(k))));
return k;
}
return $w;
}
