mergeInto(LibraryManager.library, {
  // Performs printf-style formatting.
  //   format: A pointer to the format string.
  //   varargs: A pointer to the start of the arguments list.
  // Returns the resulting string string as a character array.
  _formatString__deps: ['strlen', '_reallyNegative'],
  _formatString: function(format, varargs) {
    var textIndex = format;
    var argIndex = 0;
    function getNextArg(type) {
      // NOTE: Explicitly ignoring type safety. Otherwise this fails:
      //       int x = 4; printf("%c\n", (char)x);
      var ret;
      if (type === 'double') {
#if TARGET_ASMJS_UNKNOWN_EMSCRIPTEN == 2
        ret = {{{ makeGetValue('varargs', 'argIndex', 'double', undefined, undefined, true, 4) }}};
#else
        ret = {{{ makeGetValue('varargs', 'argIndex', 'double', undefined, undefined, true) }}};
#endif
#if USE_TYPED_ARRAYS == 2
      } else if (type == 'i64') {
#if TARGET_ASMJS_UNKNOWN_EMSCRIPTEN == 1
        ret = [{{{ makeGetValue('varargs', 'argIndex', 'i32', undefined, undefined, true) }}},
               {{{ makeGetValue('varargs', 'argIndex+8', 'i32', undefined, undefined, true) }}}];
        argIndex += {{{ STACK_ALIGN }}}; // each 32-bit chunk is in a 64-bit block
#else
        ret = [{{{ makeGetValue('varargs', 'argIndex', 'i32', undefined, undefined, true, 4) }}},
               {{{ makeGetValue('varargs', 'argIndex+4', 'i32', undefined, undefined, true, 4) }}}];
#endif

#else
      } else if (type == 'i64') {
        ret = {{{ makeGetValue('varargs', 'argIndex', 'i64', undefined, undefined, true) }}};
#endif
      } else {
        type = 'i32'; // varargs are always i32, i64, or double
        ret = {{{ makeGetValue('varargs', 'argIndex', 'i32', undefined, undefined, true) }}};
      }
#if TARGET_ASMJS_UNKNOWN_EMSCRIPTEN == 2
      argIndex += Runtime.getNativeFieldSize(type);
#else
      argIndex += Math.max(Runtime.getNativeFieldSize(type), Runtime.getAlignSize(type, null, true));
#endif
      return ret;
    }

    var ret = [];
    var curr, next, currArg;
    while(1) {
      var startTextIndex = textIndex;
      curr = {{{ makeGetValue(0, 'textIndex', 'i8') }}};
      if (curr === 0) break;
      next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
      if (curr == {{{ charCode('%') }}}) {
        // Handle flags.
        var flagAlwaysSigned = false;
        var flagLeftAlign = false;
        var flagAlternative = false;
        var flagZeroPad = false;
        var flagPadSign = false;
        flagsLoop: while (1) {
          switch (next) {
            case {{{ charCode('+') }}}:
              flagAlwaysSigned = true;
              break;
            case {{{ charCode('-') }}}:
              flagLeftAlign = true;
              break;
            case {{{ charCode('#') }}}:
              flagAlternative = true;
              break;
            case {{{ charCode('0') }}}:
              if (flagZeroPad) {
                break flagsLoop;
              } else {
                flagZeroPad = true;
                break;
              }
            case {{{ charCode(' ') }}}:
              flagPadSign = true;
              break;
            default:
              break flagsLoop;
          }
          textIndex++;
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
        }

        // Handle width.
        var width = 0;
        if (next == {{{ charCode('*') }}}) {
          width = getNextArg('i32');
          textIndex++;
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
        } else {
          while (next >= {{{ charCode('0') }}} && next <= {{{ charCode('9') }}}) {
            width = width * 10 + (next - {{{ charCode('0') }}});
            textIndex++;
            next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
          }
        }

        // Handle precision.
        var precisionSet = false, precision = -1;
        if (next == {{{ charCode('.') }}}) {
          precision = 0;
          precisionSet = true;
          textIndex++;
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
          if (next == {{{ charCode('*') }}}) {
            precision = getNextArg('i32');
            textIndex++;
          } else {
            while(1) {
              var precisionChr = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
              if (precisionChr < {{{ charCode('0') }}} ||
                  precisionChr > {{{ charCode('9') }}}) break;
              precision = precision * 10 + (precisionChr - {{{ charCode('0') }}});
              textIndex++;
            }
          }
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
        }
        if (precision < 0) {
          precision = 6; // Standard default.
          precisionSet = false;
        }

        // Handle integer sizes. WARNING: These assume a 32-bit architecture!
        var argSize;
        switch (String.fromCharCode(next)) {
          case 'h':
            var nextNext = {{{ makeGetValue(0, 'textIndex+2', 'i8') }}};
            if (nextNext == {{{ charCode('h') }}}) {
              textIndex++;
              argSize = 1; // char (actually i32 in varargs)
            } else {
              argSize = 2; // short (actually i32 in varargs)
            }
            break;
          case 'l':
            var nextNext = {{{ makeGetValue(0, 'textIndex+2', 'i8') }}};
            if (nextNext == {{{ charCode('l') }}}) {
              textIndex++;
              argSize = 8; // long long
            } else {
              argSize = 4; // long
            }
            break;
          case 'L': // long long
          case 'q': // int64_t
          case 'j': // intmax_t
            argSize = 8;
            break;
          case 'z': // size_t
          case 't': // ptrdiff_t
          case 'I': // signed ptrdiff_t or unsigned size_t
            argSize = 4;
            break;
          default:
            argSize = null;
        }
        if (argSize) textIndex++;
        next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};

        // Handle type specifier.
        switch (String.fromCharCode(next)) {
          case 'd': case 'i': case 'u': case 'o': case 'x': case 'X': case 'p': {
            // Integer.
            var signed = next == {{{ charCode('d') }}} || next == {{{ charCode('i') }}};
            argSize = argSize || 4;
            var currArg = getNextArg('i' + (argSize * 8));
#if PRECISE_I64_MATH
            var origArg = currArg;
#endif
            var argText;
#if USE_TYPED_ARRAYS == 2
            // Flatten i64-1 [low, high] into a (slightly rounded) double
            if (argSize == 8) {
              currArg = Runtime.makeBigInt(currArg[0], currArg[1], next == {{{ charCode('u') }}});
            }
#endif
            // Truncate to requested size.
            if (argSize <= 4) {
              var limit = Math.pow(256, argSize) - 1;
              currArg = (signed ? reSign : unSign)(currArg & limit, argSize * 8);
            }
            // Format the number.
            var currAbsArg = Math.abs(currArg);
            var prefix = '';
            if (next == {{{ charCode('d') }}} || next == {{{ charCode('i') }}}) {
#if PRECISE_I64_MATH
              if (argSize == 8 && i64Math) argText = i64Math.stringify(origArg[0], origArg[1], null); else
#endif
              argText = reSign(currArg, 8 * argSize, 1).toString(10);
            } else if (next == {{{ charCode('u') }}}) {
#if PRECISE_I64_MATH
              if (argSize == 8 && i64Math) argText = i64Math.stringify(origArg[0], origArg[1], true); else
#endif
              argText = unSign(currArg, 8 * argSize, 1).toString(10);
              currArg = Math.abs(currArg);
            } else if (next == {{{ charCode('o') }}}) {
              argText = (flagAlternative ? '0' : '') + currAbsArg.toString(8);
            } else if (next == {{{ charCode('x') }}} || next == {{{ charCode('X') }}}) {
              prefix = (flagAlternative && currArg != 0) ? '0x' : '';
#if PRECISE_I64_MATH
              if (argSize == 8 && i64Math) {
                if (origArg[1]) {
                  argText = (origArg[1]>>>0).toString(16);
                  var lower = (origArg[0]>>>0).toString(16);
                  while (lower.length < 8) lower = '0' + lower;
                  argText += lower;
                } else {
                  argText = (origArg[0]>>>0).toString(16);
                }
              } else
#endif
              if (currArg < 0) {
                // Represent negative numbers in hex as 2's complement.
                currArg = -currArg;
                argText = (currAbsArg - 1).toString(16);
                var buffer = [];
                for (var i = 0; i < argText.length; i++) {
                  buffer.push((0xF - parseInt(argText[i], 16)).toString(16));
                }
                argText = buffer.join('');
                while (argText.length < argSize * 2) argText = 'f' + argText;
              } else {
                argText = currAbsArg.toString(16);
              }
              if (next == {{{ charCode('X') }}}) {
                prefix = prefix.toUpperCase();
                argText = argText.toUpperCase();
              }
            } else if (next == {{{ charCode('p') }}}) {
              if (currAbsArg === 0) {
                argText = '(nil)';
              } else {
                prefix = '0x';
                argText = currAbsArg.toString(16);
              }
            }
            if (precisionSet) {
              while (argText.length < precision) {
                argText = '0' + argText;
              }
            }

            // Add sign if needed
            if (currArg >= 0) {
              if (flagAlwaysSigned) {
                prefix = '+' + prefix;
              } else if (flagPadSign) {
                prefix = ' ' + prefix;
              }
            }

            // Move sign to prefix so we zero-pad after the sign
            if (argText.charAt(0) == '-') {
              prefix = '-' + prefix;
              argText = argText.substr(1);
            }

            // Add padding.
            while (prefix.length + argText.length < width) {
              if (flagLeftAlign) {
                argText += ' ';
              } else {
                if (flagZeroPad) {
                  argText = '0' + argText;
                } else {
                  prefix = ' ' + prefix;
                }
              }
            }

            // Insert the result into the buffer.
            argText = prefix + argText;
            argText.split('').forEach(function(chr) {
              ret.push(chr.charCodeAt(0));
            });
            break;
          }
          case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': {
            // Float.
            var currArg = getNextArg('double');
            var argText;
            if (isNaN(currArg)) {
              argText = 'nan';
              flagZeroPad = false;
            } else if (!isFinite(currArg)) {
              argText = (currArg < 0 ? '-' : '') + 'inf';
              flagZeroPad = false;
            } else {
              var isGeneral = false;
              var effectivePrecision = Math.min(precision, 20);

              // Convert g/G to f/F or e/E, as per:
              // http://pubs.opengroup.org/onlinepubs/9699919799/functions/printf.html
              if (next == {{{ charCode('g') }}} || next == {{{ charCode('G') }}}) {
                isGeneral = true;
                precision = precision || 1;
                var exponent = parseInt(currArg.toExponential(effectivePrecision).split('e')[1], 10);
                if (precision > exponent && exponent >= -4) {
                  next = ((next == {{{ charCode('g') }}}) ? 'f' : 'F').charCodeAt(0);
                  precision -= exponent + 1;
                } else {
                  next = ((next == {{{ charCode('g') }}}) ? 'e' : 'E').charCodeAt(0);
                  precision--;
                }
                effectivePrecision = Math.min(precision, 20);
              }

              if (next == {{{ charCode('e') }}} || next == {{{ charCode('E') }}}) {
                argText = currArg.toExponential(effectivePrecision);
                // Make sure the exponent has at least 2 digits.
                if (/[eE][-+]\d$/.test(argText)) {
                  argText = argText.slice(0, -1) + '0' + argText.slice(-1);
                }
              } else if (next == {{{ charCode('f') }}} || next == {{{ charCode('F') }}}) {
                argText = currArg.toFixed(effectivePrecision);
                if (currArg === 0 && __reallyNegative(currArg)) {
                  argText = '-' + argText;
                }
              }

              var parts = argText.split('e');
              if (isGeneral && !flagAlternative) {
                // Discard trailing zeros and periods.
                while (parts[0].length > 1 && parts[0].indexOf('.') != -1 &&
                       (parts[0].slice(-1) == '0' || parts[0].slice(-1) == '.')) {
                  parts[0] = parts[0].slice(0, -1);
                }
              } else {
                // Make sure we have a period in alternative mode.
                if (flagAlternative && argText.indexOf('.') == -1) parts[0] += '.';
                // Zero pad until required precision.
                while (precision > effectivePrecision++) parts[0] += '0';
              }
              argText = parts[0] + (parts.length > 1 ? 'e' + parts[1] : '');

              // Capitalize 'E' if needed.
              if (next == {{{ charCode('E') }}}) argText = argText.toUpperCase();

              // Add sign.
              if (currArg >= 0) {
                if (flagAlwaysSigned) {
                  argText = '+' + argText;
                } else if (flagPadSign) {
                  argText = ' ' + argText;
                }
              }
            }

            // Add padding.
            while (argText.length < width) {
              if (flagLeftAlign) {
                argText += ' ';
              } else {
                if (flagZeroPad && (argText[0] == '-' || argText[0] == '+')) {
                  argText = argText[0] + '0' + argText.slice(1);
                } else {
                  argText = (flagZeroPad ? '0' : ' ') + argText;
                }
              }
            }

            // Adjust case.
            if (next < {{{ charCode('a') }}}) argText = argText.toUpperCase();

            // Insert the result into the buffer.
            argText.split('').forEach(function(chr) {
              ret.push(chr.charCodeAt(0));
            });
            break;
          }
          case 's': {
            // String.
            var arg = getNextArg('i8*');
            var argLength = arg ? _strlen(arg) : '(null)'.length;
            if (precisionSet) argLength = Math.min(argLength, precision);
            if (!flagLeftAlign) {
              while (argLength < width--) {
                ret.push({{{ charCode(' ') }}});
              }
            }
            if (arg) {
              for (var i = 0; i < argLength; i++) {
                ret.push({{{ makeGetValue('arg++', 0, 'i8', null, true) }}});
              }
            } else {
              ret = ret.concat(intArrayFromString('(null)'.substr(0, argLength), true));
            }
            if (flagLeftAlign) {
              while (argLength < width--) {
                ret.push({{{ charCode(' ') }}});
              }
            }
            break;
          }
          case 'c': {
            // Character.
            if (flagLeftAlign) ret.push(getNextArg('i8'));
            while (--width > 0) {
              ret.push({{{ charCode(' ') }}});
            }
            if (!flagLeftAlign) ret.push(getNextArg('i8'));
            break;
          }
          case 'n': {
            // Write the length written so far to the next parameter.
            var ptr = getNextArg('i32*');
            {{{ makeSetValue('ptr', '0', 'ret.length', 'i32') }}};
            break;
          }
          case '%': {
            // Literal percent sign.
            ret.push(curr);
            break;
          }
          default: {
            // Unknown specifiers remain untouched.
            for (var i = startTextIndex; i < textIndex + 2; i++) {
              ret.push({{{ makeGetValue(0, 'i', 'i8') }}});
            }
          }
        }
        textIndex += 2;
        // TODO: Support a/A (hex float) and m (last error) specifiers.
        // TODO: Support %1${specifier} for arg selection.
      } else {
        ret.push(curr);
        textIndex += 1;
      }
    }
    return ret;
  }
});

