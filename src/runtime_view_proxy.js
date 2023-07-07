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
function getHeapBlock(type, offset, length) {
  if (!offset) {
    offset = 0
  }

  const heap = wasmMemory.buffer

  // we should always limit the length to maxArraySize
  function createTypedArray (arrayType, offset, length) {
    const bpe = arrayType.BYTES_PER_ELEMENT;
    return new arrayType(heap, offset, length || Math.min(heap.byteLength - offset * bpe, maxArraySize / bpe));
  }

  switch (type) {
    case 'i1':
    case 'i8':
      return createTypedArray(Int8Array, offset, length);
    case 'u1':
    case 'u8':
      return createTypedArray(Uint8Array, offset, length);
    case 'i16':
      return createTypedArray(Int16Array, offset, length);
    case 'u16':
      return createTypedArray(Uint16Array, offset, length);
    case 'i32':
      return createTypedArray(Int32Array, offset, length);
    case 'u32':
      return createTypedArray(Uint32Array, offset, length);
    case 'f32':
      return createTypedArray(Float32Array, offset, length);
    case 'f64':
      return createTypedArray(Float64Array, offset, length);
    case 'i64':
      return createTypedArray(BigInt64Array, offset, length);
    case '*':
    case 'u64':
      return createTypedArray(BigUint64Array, offset, length);
    default:
      throw new Error('Invalid type');
  }
}

function createProxyHandler (type, heapBlocks) {
  const firstHeapBlock = heapBlocks[0]
  const bpe = firstHeapBlock.BYTES_PER_ELEMENT

  function getRealStartAndEnd(start, end) {
    const startReal = (start || 0) * bpe
    const endReal = end ? end * bpe : wasmMemory.byteLength
    return [startReal, endReal]
  }

  function copyWithin (target, start, end) {
    if (target * bpe >= maxArraySize || start * bpe >= maxArraySize || (end && end * bpe >= maxArraySize)) {
      const len = end - start
      const targetArray = getHeapBlock(type, target * bpe, len)
      const sourceArray = getHeapBlock(type, start * bpe, len)
      targetArray.set(sourceArray)
      return heapBlocks[0]
    } else {
      return heapBlocks[0].copyWithin(target, start, end)
    }
  }

  function setOverridden(array, offset) {
    const offsetReal = (offset || 0) * bpe
    if (offsetReal >= maxArraySize || array.byteLength + offsetReal >= maxArraySize) {
      const targetArray = getHeapBlock(type, offsetReal, array.length)
      targetArray.set(array)
    } else {
      firstHeapBlock.set(array, offset)
    }
  }

  function subarray(start, end) {
    const [startReal, endReal] = getRealStartAndEnd(start, end)
    if (startReal >= maxArraySize || endReal >= maxArraySize) {
      return getHeapBlock(type, startReal, endReal - startReal)
    } else {
      return firstHeapBlock.subarray(start, end)
    }
  }

  function fill(value, start, end) {
    const [startReal, endReal] = getRealStartAndEnd(start, end)
    if (startReal >= maxArraySize || endReal >= maxArraySize) {
      const hb = getHeapBlock(type, startReal, endReal - startReal)
      hb.fill(value, 0, end - start)
      return firstHeapBlock
    } else {
      return firstHeapBlock.fill(value, start, end)
    }
  }
  function slice(start, end) {
    const [startReal, endReal] = getRealStartAndEnd(start, end)
    if (startReal >= maxArraySize || endReal >= maxArraySize) {
      const hb = getHeapBlock(type, startReal, endReal - startReal)
      return hb.slice(start, end)
    } else {
      return firstHeapBlock.slice(start, end)
    }
  }

  return {
    get(target, property) {
      if (parseInt(property, 10) == property) {
        const memoryOffset = property * bpe
        const blockNumber = Math.floor(memoryOffset / maxArraySize)
        return heapBlocks[blockNumber][property - blockNumber * maxArraySize]
      }

      if (property === 'copyWithin') {
        return copyWithin
      }

      if (property === 'set') {
        return setOverridden
      }

      if (property === 'subarray') {
        return subarray
      }

      if (property === 'fill') {
        return fill
      }

      if (property === 'slice') {
        return slice
      }

      return firstHeapBlock[property]
    },
    set(target, property, value) {
      if (parseInt(property, 10) == property) {
        const memoryOffset = property * bpe
        const blockNumber = Math.floor(memoryOffset / maxArraySize)
        heapBlocks[blockNumber][property - blockNumber * maxArraySize] = value
        return true
      }

      firstHeapBlock[property] = value
      return true;
    },
  }
}

function createMemoryProxy(type) {
  const heapBlocks = [
    getHeapBlock(type, 0),
  ];
  const numberOfBlocks = Math.ceil(b.byteLength / maxArraySize)
  for (let i = 1; i < numberOfBlocks; i++) {
    heapBlocks.push(getHeapBlock(type, i * maxArraySize))
  }
  return new Proxy(heapBlocks[0], createProxyHandler(type, heapBlocks));
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
