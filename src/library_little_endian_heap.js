/*  Stores value into the heap enforcing LE byte order. Uses DataView methods
 *  as proxy.
 *  size is in bytes.
 *  type can be: 0=unsigned,1=signed,2=float.
 *
 *  @param {number} byteOffset
 *  @param {ptr} value
 *  @param {number} size
 *  @param {number} type
 */
function LE_HEAP_STORE(byteOffset, value, size, type) {
  var typeVals = [0, 1, 2];
  if (!typeVals.includes(type)) {
    throw 'error: type must be one of ' + typeVals;
  }
  var sizeVals = [2, 4, 8];
  if (!sizeVals.includes(size)) {
    throw 'error: size must be one of ' + sizeVals;
  }
  if (size === 2 && type === 0) {
    HEAP_DATA_VIEW.setUint16(byteOffset, value, true);
  } else if (size === 2 && type === 1) {
    HEAP_DATA_VIEW.setInt16(byteOffset, value, true);
  } else if (size === 4 && type === 0) {
    HEAP_DATA_VIEW.setUint32(byteOffset, value, true);
  } else if (size === 4 && type === 1) {
    HEAP_DATA_VIEW.setInt32(byteOffset, value, true);
  } else if (size === 4 && type === 2) {
    HEAP_DATA_VIEW.setFloat32(byteOffset, value, true);
  } else if (size === 8 && type === 2) {
    HEAP_DATA_VIEW.setFloat64(byteOffset, value, true);
  } else if (size === 8 && type === 1) {
    HEAP_DATA_VIEW.setBigInt64(byteOffset, value, true);
  } else {
    throw 'error: invalid size & type combination: size='+size +', type=' + type;
  }
}

/*  Loads value from the heap enforcing LE byte order. Uses DataView methods
 *  as proxy.
 *  size is in bytes.
 *  type can be: 0=unsigned,1=signed,2=float
 *
 *  @param {number} byteOffset
 *  @param {number} size
 *  @param {number} type
 */
function LE_HEAP_LOAD(byteOffset, size, type) {
  var typeVals = [0, 1, 2];
  if (!typeVals.includes(type)) {
    throw 'error: type must be one of ' + typeVals;
  }
  var sizeVals = [2, 4, 8];
  if (!sizeVals.includes(size)) {
    throw 'error: size must be one of ' + sizeVals;
  }
  if (size === 2 && type === 0) {
    return HEAP_DATA_VIEW.getUint16(byteOffset, true);
  } else if (size === 2 && type === 1) {
    return HEAP_DATA_VIEW.getInt16(byteOffset, true);
  } else if (size === 4 && type === 0) {
    return HEAP_DATA_VIEW.getUint32(byteOffset, true);
  } else if (size === 4 && type === 1) {
    return HEAP_DATA_VIEW.getInt32(byteOffset, true);
  } else if (size === 4 && type === 2) {
    return HEAP_DATA_VIEW.getFloat32(byteOffset, true);
  } else if (size === 8 && type === 2) {
    return HEAP_DATA_VIEW.getFloat64(byteOffset, true);
  } else if (size === 8 && type === 1) {
    return HEAP_DATA_VIEW.getBigInt64(byteOffset, true);
  } else {
    throw 'error: invalid size & type combination: size='+size +', type=' + type;
  }
}
