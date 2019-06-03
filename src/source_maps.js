var sourceMap = {"version":3,"sources":["/usr/local/google/home/gzchen/waterfall/src/work/ubsan_tests/leak.cc"],"names":[],"mappings":"u2DASA,qCAEM,MAAF,YACA,YAEO,gBAAO,OAAE,eAAlB,yBACS,MAAL,OAAF,kBAAK,OADmB,yBAA1B,UAGK,YAEL"};

var vlqMap = {};
'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/='.split('').forEach(function (c, i) {
  vlqMap[c] = i;
});

// based on https://github.com/Rich-Harris/vlq/blob/master/src/vlq.ts
function decodeVLQ(string) {
  var result = [];
  var shift = 0;
  var value = 0;

  for (var i = 0; i < string.length; ++i) {
    var integer = vlqMap[string[i]];

    if (integer === undefined) {
      throw new Error('Invalid character (' + string[i] + ')');
    }

    value += (integer & 31) << shift;

    if (integer & 32) {
      shift += 5;
    } else {
      var shouldNegate = value & 1;
      value >>= 1;
      result.push( shouldNegate ? -value : value );
      value = shift = 0;
    }
  }

  return result;
}

function bisect_right(array, value) {
  var lo = 0, hi = array.length, mid;

  while (lo < hi) {
    mid = Math.floor((lo + hi) / 2);
    if (array[mid] > value) {
      hi = mid;
    } else {
      lo = mid + 1;
    }
  }
  return lo;
}

function WASMSourceMap(sourceMap) {
  this.version = sourceMap.version;
  this.sources = sourceMap.sources;
  this.names = sourceMap.names;

  this.mapping = {};
  this.offsets = [];

  var offset = 0, src = 0, line = 1, col = 1, name = 0;
  sourceMap.mappings.split(',').forEach(function (segment, index) {
    var data = decodeVLQ(segment);
    var info = {};

    offset += data[0];
    if (data.length >= 2) info.source = src += data[1];
    if (data.length >= 3) info.line = line += data[2];
    if (data.length >= 4) info.column = col += data[3];
    if (data.length >= 5) info.name = name += data[4];
    this.mapping[offset] = info;
    this.offsets.push(offset);
  }, this);
  this.offsets.sort();
}

WASMSourceMap.prototype.lookup = function (offset) {
  var info = this.mapping[this.offsets[bisect_right(this.offsets, offset) - 1]];
  if (!info)
    return null;
  return {
    source: this.sources[info.source],
    line: info.line,
    column: info.column,
    name: this.names[info.name],
  };
}
