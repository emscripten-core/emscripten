let result = {};
result.BigInt64Array_name = BigInt64Array.name;
let arr2signed = new BigInt64Array(arr1signed.buffer);
let arr2unsigned = new BigUint64Array(arr1unsigned.buffer);

result.arr1_to_arr1_signed = [];
for(let x of bigint_list){
  arr1signed[0] = x;
  result.arr1_to_arr1_signed.push(arr1signed[0]);
}

result.arr1_to_arr1_unsigned = [];
for(let x of bigint_list){
  arr1unsigned[0] = x;
  result.arr1_to_arr1_unsigned.push(arr1unsigned[0]);
}

result.arr1_to_arr2_signed = [];
for(let x of bigint_list){
  arr1signed[0] = x;
  result.arr1_to_arr2_signed.push(arr2signed[0]);
}

result.arr1_to_arr2_unsigned = [];
for(let x of bigint_list){
  arr1unsigned[0] = x;
  result.arr1_to_arr2_unsigned.push(arr2unsigned[0]);
}

result.arr2_to_arr1_signed = [];
for(let x of bigint_list){
  arr2signed[0] = x;
  result.arr2_to_arr1_signed.push(arr1signed[0]);
}

result.arr2_to_arr1_unsigned = [];
for(let x of bigint_list){
  arr2unsigned[0] = x;
  result.arr2_to_arr1_unsigned.push(arr1unsigned[0]);
}


result.assertEquals = [];
function assertEqual(cb){
  result.assertEquals.push([cb.toString(), cb()]);
}

let arr1 = arr1unsigned;
let arr2 = arr2unsigned;
assertEqual(() => [arr2.BYTES_PER_ELEMENT, arr1.BYTES_PER_ELEMENT]);
assertEqual(() => [arr2.byteLength, arr1.byteLength]);
assertEqual(() => [arr2.length, arr1.length]);
assertEqual(() => [arr2.slice().length, arr1.slice().length]);
assertEqual(() => [arr2.slice(1, 5).length, arr1.slice(1, 5).length]);

arr1[0] = 1n; arr1[1] = 2n; arr1[2] = 3n; arr1[3] = 4n; arr1[4] = 5n;

result.arr2_slice = Array.from(arr2.slice(1, 5));
result.arr2_subarray = Array.from(arr2.subarray(1, 5));
let reducer = (k, v) => typeof v  === 'bigint' ? v.toString() + 'n' : v;
function arraytostring(arr){
  return JSON.stringify(Array.from(Array.from(arr)), reducer);
}

let sub = arr2.subarray(1, 5);
assertEqual(() => [sub.byteOffset, 8]);
sub[0] = 7n; sub[1] = 77n; sub[2] = 3n; sub[3] = 66n;
assertEqual(() => [arraytostring(arr1.slice(0, 5)), '["1n","7n","77n","3n","66n"]']);

arr1[2] = 62n;
assertEqual(() => [sub[1], 62n]);

let slice = arr2.slice(1, 5);
assertEqual(() => [slice.byteOffset, 0]);

slice[0] = 777n; slice[1] = 666n; slice[2] = 555n;
assertEqual(() => [arraytostring(arr1.slice(0, 5)), '["1n","7n","62n","3n","66n"]']);

arr2.set([2n, 4n, 8n]);
assertEqual(() => [arraytostring(arr1.slice(0, 3)), '["2n","4n","8n"]']);

arr2.set([1n, 3n, 7n], 6);
assertEqual(() => [arraytostring(arr1.slice(6, 9)), '["1n","3n","7n"]']);

arr1[15] = 111n; arr1[18] = 171n; arr1[19] = 629n;

assertEqual(() => [arraytostring(arr2.slice(-1)), '["629n"]']);
assertEqual(() => [arraytostring(arr2.slice(-5, -1)), '["111n","0n","0n","171n"]']);
assertEqual(() => [arraytostring(arr2.slice(-5, -6)), '[]']);

arr3 = new BigUint64Array(Array.from({length:5}, (_, idx) => BigInt(idx)));
assertEqual(() => [arraytostring(arr3), '["0n","1n","2n","3n","4n"]']);
arr3.copyWithin(0, 2, 10);
assertEqual(() => [arraytostring(arr3), '["2n","3n","4n","3n","4n"]']);

console.log(JSON.stringify(result, reducer));
