// The Computer Language Benchmarks Game
//  http://shootout.alioth.debian.org
//
//  Contributed by Ian Osgood
//  Largely rewritten by Matthew Wilson

function fastaRepeat(n, seq) {
  var seqi = 0, len = seq.length, i, j, k, l, block,
    str = Array(len*60+1).join(seq), lines = Array(i=j=len*len);
  while (--j>-1) { lines[j] = str.substr(60*j, 60) }
  block = lines.join("\n");
  for (j=0, k=Math.floor((l=Math.floor(n/60))/i); j<k; ++j) { print(block) }
  for (j = 0, k = l % i; j < k; ++j) { print(lines[j]) }
  if (n % 60 > 0) { print(lines[k].substr(0, n % 60)) }
}

var rand=(function() {
  var Last = 42;
  return function() { return (Last=(Last * 3877 + 29573) % 139968) / 139968 }
})();

function printLineMaker(table) {
  var h = 0, k = [], v = [], c, l=0;
  for (c in table) { l = v[h] = table[k[h++] = c]+=l; }
  return function(x) {
    var line = "";
    next: for (var i=0; i<x; ++i) {
      var r = rand(), j=0;
      for (;;++j) {
        if (r < v[j]) {
          line += k[j];
          continue next;
        }
      }
    }
    print(line);
  }
}

function fastaRandom(n, table) {
  var printLine=printLineMaker(table);
  while ((n -= 60) > -1) { printLine(60) }
  if (n<0 && n>-60) { printLine(60 + n) }
}

(function main(n) {
  var ALU = "GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGG" +
            "GAGGCCGAGGCGGGCGGATCACCTGAGGTCAGGAGTTCGAGA" +
            "CCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACTAAAAAT" +
            "ACAAAAATTAGCCGGGCGTGGTGGCGCGCGCCTGTAATCCCA" +
            "GCTACTCGGGAGGCTGAGGCAGGAGAATCGCTTGAACCCGGG" +
            "AGGCGGAGGTTGCAGTGAGCCGAGATCGCGCCACTGCACTCC" +
            "AGCCTGGGCGACAGAGCGAGACTCCGTCTCAAAAA";

  var IUB = { a:0.27, c:0.12, g:0.12, t:0.27, B:0.02, D:0.02, H:0.02, K:0.02,
              M:0.02, N:0.02, R:0.02, S:0.02, V:0.02, W:0.02, Y:0.02 }

  var HomoSap = {
    a:0.3029549426680, c:0.1979883004921, g:0.1975473066391, t:0.3015094502008
  }

  print(">ONE Homo sapiens alu")
  fastaRepeat(2*n, ALU)

  print(">TWO IUB ambiguity codes")
  fastaRandom(3*n, IUB)

  print(">THREE Homo sapiens frequency")
  fastaRandom(5*n, HomoSap)
}).call(this, 1*arguments[0]*1)

