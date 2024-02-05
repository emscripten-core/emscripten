#if !POLYFILL
#error "this file should never be included unless POLYFILL is set"
#endif

if (typeof globalThis.BigInt64Array === "undefined") {
  // BigInt64Array polyfill for Safari versions between v14.0 and v15.0.
  // All browsers other than Safari added BigInt and BigInt64Array at the same
  // time, but Safari introduced BigInt in v14.0 and introduced BigInt64Array in
  // v15.0

  function partsToBigIntSigned(lower, upper) {
    return BigInt(lower) | (BigInt(upper + 2 * (upper & 0x80000000)) << 32n);
  }

  function partsToBigIntUnsigned(lower, upper) {
    return BigInt(lower) | (BigInt(upper) << 32n);
  }

  function bigIntToParts(value) {
    var lower = Number(BigInt(value) & BigInt(0xffffffff)) | 0;
    var upper = Number(BigInt(value) >> 32n) | 0;
    return [lower, upper];
  }

  function createBigIntArrayShim(partsToBigInt) {
    function createBigInt64Array(array) {
      if (typeof array === "number") {
        array = new Uint32Array(2 * array);
      }
      var orig_array;
      if (!ArrayBuffer.isView(array)) {
        if (array.constructor && array.constructor.name === "ArrayBuffer") {
          array = new Uint32Array(array);
        } else {
          orig_array = array;
          array = new Uint32Array(array.length * 2);
        }
      }
      var proxy = new Proxy(
        {
          slice(min, max) {
            max ??= array.length;
            var new_buf = array.slice(min * 2, max * 2);
            return createBigInt64Array(new_buf);
          },
          subarray(min, max) {
            var new_buf = array.subarray(min * 2, max * 2);
            return createBigInt64Array(new_buf);
          },
          [Symbol.iterator]: function* () {
            for (var i = 0; i < array.length / 2; i++) {
              yield partsToBigInt(array[2 * i], array[2 * i + 1]);
            }
          },
          BYTES_PER_ELEMENT: 2 * array.BYTES_PER_ELEMENT,
          buffer: array.buffer,
          byteLength: array.byteLength,
          byteOffset: array.byteOffset,
          length: array.length / 2,
          copyWithin: function (target, start, end) {
            array.copyWithin(target * 2, start * 2, end * 2);
            return proxy;
          },
          set(source, targetOffset) {
            targetOffset ??= 0;
            if (2 * (source.length + targetOffset) > array.length) {
              // This is the Chrome error message
              // Firefox: "invalid or out-of-range index"
              throw new RangeError("offset is out of bounds");
            }
            for (var i = 0; i < source.length; i++) {
              var value = source[i];
              var pair = bigIntToParts(value);
              array.set(pair, 2 * (targetOffset + i));
            }
          },
        },
        {
          get(target, idx, receiver) {
            if (typeof idx !== "string" || !/^\d+$/.test(idx)) {
              return Reflect.get(target, idx, receiver);
            }
            var lower = array[idx * 2];
            var upper = array[idx * 2 + 1];
            return partsToBigInt(lower, upper);
          },
          set(target, idx, value, receiver) {
            if (typeof idx !== "string" || !/^\d+$/.test(idx)) {
              return Reflect.set(target, idx, value, receiver);
            }
            if (typeof value !== "bigint") {
              // Chrome error message, Firefox has no "a" in front if "BigInt".
              throw new TypeError(`Cannot convert ${value} to a BigInt`);
            }
            var pair = bigIntToParts(value);
            array.set(pair, 2 * idx);
            return true;
          },
        }
      );
      if (orig_array) {
        proxy.set(orig_array);
      }
      return proxy;
    }
    return createBigInt64Array;
  }

  globalThis.BigUint64Array = createBigIntArrayShim(partsToBigIntUnsigned);
  globalThis.BigInt64Array = createBigIntArrayShim(partsToBigIntSigned);
}
