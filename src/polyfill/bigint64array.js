// BigInt ==> globalThis so we can use globalThis.
var BigInt64Array;
var BigUint64Array;
if (typeof globalThis.BigInt64Array !== "undefined") {
    BigInt64Array = globalThis.BigInt64Array;
    BigUint64Array = globalThis.BigUint64Array;
} else {
    // BigInt64Array polyfill for Safari versions between v14.0 and v15.0.
    // All browsers other than Safari added BigInt and BigInt64Array at the same
    // time, but Safari introduced BigInt in v14.0 and introduced BigInt64Array in
    // v15.0

    function partsToBigIntSigned(lower, upper) {
        return BigInt(lower) | (BigInt(upper + 2 * (upper & 0x80000000)) << BigInt(32));
    }

    function partsToBigIntUnsigned(lower, upper) {
        return BigInt(lower) | (BigInt(upper) << BigInt(32));
    }

    function bigIntToParts(value) {
        var lower = Number(BigInt(value) & BigInt(0xffffffff)) | 0;
        var upper = (Number(BigInt(value) >> BigInt(32)) | 0);
        return [lower, upper];
    }

    function createBigIntArrayShim(partsToBigInt) {
        function createBigInt64Array(array) {
            if(!ArrayBuffer.isView(array)){
                array = new Uint32Array(array);
            }
            let proxy = new Proxy({
                slice: function(min, max) {
                    var new_buf = array.slice(min * 2, max *2);
                    return createBigInt64Array(new_buf);
                },
                subarray: function(min, max) {
                    var new_buf = array.subarray(min * 2, max *2);
                    return createBigInt64Array(new_buf);
                },
                [Symbol.iterator]: function*() {
                    for (var i = 0; i < (array.length)/2; i++) {
                        yield partsToBigInt(array[2*i], array[2*i+1]);
                    }
                },
                buffer : array.buffer,
                byteLength : array.byteLength,
                offset : array.byteOffset / 2,
                copyWithin: function(target, start, end) {
                    array.copyWithin(target*2, start * 2, end*2);
                    return proxy;
                },
                set: function(source, targetOffset) {
                    if (2*(source.length + targetOffset) > array.length) {
                        // This is the Chrome error message
                        // Firefox: "invalid or out-of-range index"
                        throw new RangeError("offset is out of bounds");
                    }
                    for (var i = 0; i < array.length; i++) {
                        var value = source[i];
                        var pair = bigIntToParts(BigInt(value));
                        array.set(pair, 2*(targetOffset + i));
                    }
                }
                }, {
                get: function(target, idx, receiver) {
                    if (typeof idx !== "string" || !/^\d+$/.test(idx)) {
                        return Reflect.get(target, idx, receiver);
                    }
                    var lower = array[idx * 2];
                    var upper = array[idx * 2 + 1];
                    return partsToBigInt(lower, upper);
                },
                set: function(target, idx, value, receiver) {
                    if (typeof idx !== "string" || !/^\d+$/.test(idx)) {
                        return Reflect.set(target, idx, value, receiver);
                    }
                    if (typeof value !== "bigint") {
                        // Chrome error message, Firefox has no "a" in front if "BigInt".
                        throw new TypeError(`Cannot convert ${value} to a BigInt`);
                    }
                    var pair = bigIntToParts(value);
                    array.set(pair, 2*idx);
                }
            });
            return proxy;
        }
        return createBigInt64Array;
    }

    var BigUint64Array = createBigIntArrayShim(partsToBigIntUnsigned);
    var BigInt64Array = createBigIntArrayShim(partsToBigIntSigned);
}
