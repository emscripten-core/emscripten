/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Chrome does not allow TypedArrays with more than 4294967296 elements
// We'll create proxy objects for HEAP(U)8 when memory is > 4gb and for HEAP(U)16 when > 8gb
// https://bugs.chromium.org/p/v8/issues/detail?id=4153
var maxArraySize = Math.min(b.byteLength, 4 * 1024 * 1024 * 1024 - 2);
/**
 * @param {string} type - Heap type
 * @param {number} [offset] - Heap offset
 * @param {number} [length] - typed array length
 */
function getHeapBlock(type, offset, length) {
  if (!offset) {
    offset = 0
  }

  let heap = wasmMemory.buffer

  // we should always limit the length to maxArraySize
  function createTypedArray(arrayType, offset, length) {
    let bpe = arrayType.BYTES_PER_ELEMENT;
    return new arrayType(heap, offset, length || Math.min((heap.byteLength - offset) / bpe, maxArraySize));
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
    default:
      throw new Error('Invalid type');
  }
}

function createProxyHandler(type, heapBlocks) {
  let firstHeapBlock = heapBlocks[0]
  let bpe = firstHeapBlock.BYTES_PER_ELEMENT

  function getRealStartAndEnd(start, end) {
    let startReal = (start || 0) * bpe
    let endReal = end ? end * bpe : wasmMemory.buffer.byteLength
    return [startReal, endReal]
  }

  function copyWithin(target, start, end) {
    if (target * bpe >= maxArraySize || start * bpe >= maxArraySize || (end && end * bpe >= maxArraySize)) {
      let len = end - start
      let targetArray = getHeapBlock(type, target * bpe, len)
      let sourceArray = getHeapBlock(type, start * bpe, len)
      targetArray.set(sourceArray)
      return heapBlocks[0]
    }
    return heapBlocks[0].copyWithin(target, start, end)
  }

  function setOverridden(array, offset) {
    let offsetReal = (offset || 0) * bpe
    if (offsetReal >= maxArraySize || array.byteLength + offsetReal >= maxArraySize) {
      let targetArray = getHeapBlock(type, offsetReal, array.length)
      targetArray.set(array)
    } else {
      firstHeapBlock.set(array, offset)
    }
  }

  function subarray(start, end) {
    let [startReal, endReal] = getRealStartAndEnd(start, end)
    if (startReal >= maxArraySize || endReal >= maxArraySize) {
      return getHeapBlock(type, startReal, endReal - startReal)
    }
    return firstHeapBlock.subarray(start, end)
  }

  function fill(value, start, end) {
    let [startReal, endReal] = getRealStartAndEnd(start, end)
    if (startReal >= maxArraySize || endReal >= maxArraySize) {
      let hb = getHeapBlock(type, startReal, endReal - startReal)
      hb.fill(value, 0, end - start)
      return firstHeapBlock
    }
    return firstHeapBlock.fill(value, start, end)
  }
  function slice(start, end) {
    let [startReal, endReal] = getRealStartAndEnd(start, end)
    if (startReal >= maxArraySize || endReal >= maxArraySize) {
      let hb = getHeapBlock(type, startReal, endReal - startReal)
      return hb.slice(0, end - start)
    }
    return firstHeapBlock.slice(start, end)
  }

  return {
    get(target, property) {
      if (parseInt(property, 10) == property) {
        let memoryOffset = property * bpe
        let blockNumber = Math.floor(memoryOffset / maxArraySize)
        return heapBlocks[blockNumber][property - blockNumber * maxArraySize]
      }
      switch (property) {
        case 'copyWithin': return copyWithin;
        case 'set': return setOverridden;
        case 'subarray': return subarray;
        case 'fill': return fill;
        case 'slice': return slice;
        case 'length': return wasmMemory.buffer.byteLength;
      }
      return firstHeapBlock[property]
    },
    set(target, property, value) {
      if (parseInt(property, 10) == property) {
        let memoryOffset = property * bpe
        let blockNumber = Math.floor(memoryOffset / maxArraySize)
        heapBlocks[blockNumber][property - blockNumber * maxArraySize] = value
        return true
      }

      firstHeapBlock[property] = value
      return true;
    },
  }
}

function createMemoryProxy(type) {
  let heapBlocks = [];
  let bpe = type === 'i16' || type === 'u16' ? 2 : 1
  let numberOfBlocks = Math.ceil(b.byteLength / maxArraySize / bpe)
  for (let i = 0; i < numberOfBlocks; i++) {
    heapBlocks.push(getHeapBlock(type, i * maxArraySize * bpe))
  }
  return new Proxy(heapBlocks[0], createProxyHandler(type, heapBlocks));
}

if (b.byteLength > maxArraySize) {
  Module['HEAP8'] = HEAP8 = createMemoryProxy('i8')
  Module['HEAPU8'] = HEAPU8 = createMemoryProxy('u8')
} else {
  Module['HEAP8'] = HEAP8 = new Int8Array(b);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(b);
}
if (b.byteLength > maxArraySize * 2) {
  Module['HEAP16'] = HEAP16 = createMemoryProxy('i16')
  Module['HEAPU16'] = HEAPU16 = createMemoryProxy('u16')
} else {
  Module['HEAP16'] = HEAP16 = new Int16Array(b);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(b);
}
