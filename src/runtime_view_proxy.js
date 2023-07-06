/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Chrome does not allow TypedArrays with more than 4294967296 elements
// We'll create proxy objects for HEAP(U)8 when memory is > 4gb and for HEAP(U)16 when > 8gb
// https://bugs.chromium.org/p/v8/issues/detail?id=4153
var maxArraySize = Math.min(b.byteLength, 4 * 1024 * 1024 * 1024 - 8);
/**
 * @param {string} type - Heap type
 * @param {number} [offset] - Heap offset
 * @param {number} [length] - typed array length
 */
const getHeapBlock = (type, offset, length) => {
  if (!offset) {
    offset = 0
  }

  const heap = wasmMemory.buffer
  switch (type) {
    case 'i1':
    case 'i8':
      return new Int8Array(heap, offset, length || maxArraySize / Int8Array.BYTES_PER_ELEMENT);
    case 'u1':
    case 'u8':
      return new Uint8Array(heap, offset, length || maxArraySize / Uint8Array.BYTES_PER_ELEMENT);
    case 'i16':
      return new Int16Array(heap, offset, length || maxArraySize / Int16Array.BYTES_PER_ELEMENT);
    case 'u16':
      return new Uint16Array(heap, offset, length || maxArraySize / Uint16Array.BYTES_PER_ELEMENT);
    case 'i32':
      return new Int32Array(heap, offset, length || maxArraySize / Int32Array.BYTES_PER_ELEMENT);
    case 'u32':
      return new Uint32Array(heap, offset, length || maxArraySize / Uint32Array.BYTES_PER_ELEMENT);
    case 'f32':
      return new Float32Array(heap, offset, length || maxArraySize / Float32Array.BYTES_PER_ELEMENT);
    case 'f64':
      return new Float64Array(heap, offset, length || maxArraySize / Float64Array.BYTES_PER_ELEMENT);
    case 'i64':
      return new BigInt64Array(heap, offset, length || maxArraySize / BigInt64Array.BYTES_PER_ELEMENT);
    case '*':
    case 'u64':
      return new BigUint64Array(heap, offset, length || maxArraySize / BigUint64Array.BYTES_PER_ELEMENT);
    default:
      throw new Error('Invalid type');
  }
}

var proxyHandler = (type, heapBlock) => ({
  heapBlock,
  copyWithin (target, start, end) {
    const bpe = heapBlock.BYTES_PER_ELEMENT
    if (target * bpe >= maxArraySize || start * bpe >= maxArraySize || (end && end * bpe >= maxArraySize)) {
      var len = end - start
      var targetArray = getHeapBlock(type, target * bpe, len)
      var sourceArray = getHeapBlock(type, start * bpe, len)
      targetArray.set(sourceArray)
      return heapBlock
    } else {
      return heapBlock.copyWithin(target, start, end)
    }
  },
  setOverridden(array, offset) {
    var offsetReal = (offset || 0) * heapBlock.BYTES_PER_ELEMENT
    if (offsetReal >= maxArraySize || array.byteLength + offsetReal >= maxArraySize) {
      var targetArray = getHeapBlock(type, offsetReal, array.length)
      targetArray.set(array)
    } else {
      heapBlock.set(array, offset)
    }
  },
  subarray(start, end) {
    var startReal = (start || 0) * heapBlock.BYTES_PER_ELEMENT
    var endReal = end ? end * heapBlock.BYTES_PER_ELEMENT : wasmMemory.byteLength
    if (startReal >= maxArraySize || endReal >= maxArraySize) {
      return getHeapBlock(type, startReal, endReal - startReal)
    } else {
      return heapBlock.subarray(start, end)
    }
  },
  fill(value, start, end) {
    var startReal = (start || 0) * heapBlock.BYTES_PER_ELEMENT
    var endReal = end ? end * heapBlock.BYTES_PER_ELEMENT : wasmMemory.byteLength
    if (startReal >= maxArraySize || endReal >= maxArraySize) {
      const hb = getHeapBlock(type, startReal, endReal - startReal)
      hb.fill(0, 0)
      return heapBlock
    } else {
      return heapBlock.fill(value, start, end)
    }
  },
  slice(start, end) {
    var startReal = (start || 0) * heapBlock.BYTES_PER_ELEMENT
    var endReal = end ? end * heapBlock.BYTES_PER_ELEMENT : wasmMemory.byteLength
    if (startReal >= maxArraySize || endReal >= maxArraySize) {
      const hb = getHeapBlock(type, startReal, endReal - startReal)
      return hb.slice(start, end)
    } else {
      return heapBlock.slice(start, end)
    }
  },
  get(target, property) {
    if (typeof property === 'number' || typeof property === 'bigint') {
      var memoryOffset = property * target.BYTES_PER_ELEMENT
      if (memoryOffset >= maxArraySize) {
        var heap = getHeapBlock(type, memoryOffset, 1);
        return heap[0];
      } else {
        return this.heapBlock[property]
      }
    }

    if (property === 'copyWithin') {
      return this.copyWithin
    }

    if (property === 'set') {
      return this.setOverridden
    }

    if (property === 'subarray') {
      return this.subarray
    }

    if (property === 'fill') {
      return this.fill
    }

    if (property === 'slice') {
      return this.slice
    }

    return heapBlock[property]
  },
  set(target, property, value) {
    if (typeof property === 'number' || typeof property === 'bigint') {
      var memoryOffset = property * target.BYTES_PER_ELEMENT
      if (memoryOffset >= maxArraySize) {
        var heap = getHeapBlock(type, memoryOffset, 1);
        heap[0] = value;
        return true;
      }
    }

    heapBlock[property] = value
    return true;
  },
})

function createMemoryProxy(type) {
  const block = getHeapBlock(type, 0)
  return new Proxy(block, proxyHandler(type, block));
}

if (b.byteLength > maxArraySize) {
  Module["HEAP8"] = HEAP8 = createMemoryProxy('i8')
  Module["HEAPU8"] = HEAPU8 = createMemoryProxy('u8')
} else {
  Module['HEAP8'] = HEAP8 = new Int8Array(b);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(b);
}
if (b.byteLength > maxArraySize * 2) {
  Module["HEAP16"] = HEAP16 = createMemoryProxy('i16')
  Module["HEAPU16"] = HEAPU16 = createMemoryProxy('u16')
} else {
  Module['HEAP16'] = HEAP16 = new Int16Array(b);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(b);
}
