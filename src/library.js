// An implementation of a libc for the web. Basically, implementations of
// the various standard C libraries, that can be called from compiled code,
// and work using the actual JavaScript environment.
//
// We search the Library object when there is an external function. If the
// entry in the Library is a function, we insert it. If it is a string, we
// do another lookup in the library (a simple way to write a function once,
// if it can be called by different names). We also allow dependencies,
// using __deps. Initialization code to be run after allocating all
// global constants can be defined by __postset.
//
// Note that the full function name will be '_' + the name in the Library
// object. For convenience, the short name appears here. Note that if you add a
// new function with an '_', it will not be found.

var Library = {
  // ==========================================================================
  // stdio.h
  // ==========================================================================

  _scanString: function() {
    // Supports %x, %4x, %d.%d, %s
    var str = Pointer_stringify(arguments[0]);
    var stri = 0;
    var fmt = Pointer_stringify(arguments[1]);
    var fmti = 0;
    var args = Array.prototype.slice.call(arguments, 2);
    var argsi = 0;
    var fields = 0;
    while (fmti < fmt.length) {
      if (fmt[fmti] === '%') {
        fmti++;
        var max_ = parseInt(fmt[fmti]);
        if (!isNaN(max_)) fmti++;
        var type = fmt[fmti];
        fmti++;
        var curr = 0;
        while ((curr < max_ || isNaN(max_)) && stri+curr < str.length) {
          if ((type === 'd' && parseInt(str[stri+curr]) >= 0) ||
              (type === 'x' && parseInt(str[stri+curr].replace(/[a-fA-F]/, 5)) >= 0) ||
              (type === 's')) {
            curr++;
          } else {
            break;
          }
        }
        if (curr === 0) return 0; // failure
        var text = str.substr(stri, curr);
        stri += curr;
        switch (type) {
          case 'd': {
            {{{ makeSetValue('args[argsi]', '0', 'parseInt(text)', 'i32') }}}
            break;
          }
          case 'x': {
            {{{ makeSetValue('args[argsi]', '0', 'eval("0x" + text)', 'i32') }}}
            break;
          }
          case 's': {
            var array = intArrayFromString(text);
            for (var j = 0; j < array.length; j++) {
              {{{ makeSetValue('args[argsi]', 'j', 'array[j]', 'i8') }}}
            }
            break;
          }
        }
        argsi++;
        fields++;
      } else { // not '%'
        if (fmt[fmti] === str[stri]) {
          fmti++;
          stri++;
        } else {
          break;
        }
      }
    }
    return { fields: fields, bytes: stri };
  },
  sscanf__deps: ['_scanString'],
  sscanf: function() {
    return __scanString.apply(null, arguments).fields;
  },

  _formatString__deps: ['$STDIO', 'isdigit'],
  _formatString: function() {
    var cStyle = false;
    var textIndex = arguments[0];
    var argIndex = 1;
    if (textIndex < 0) {
      cStyle = true;
      textIndex = -textIndex;
      argIndex = arguments[1];
    } else {
      var _arguments = arguments;
    }
    function getNextArg(isFloat, size) {
      var ret;
      if (!cStyle) {
        ret = _arguments[argIndex];
        argIndex++;
      } else {
        if (isFloat) {
          ret = {{{ makeGetValue(0, 'argIndex', 'double') }}};
        } else {
          ret = {{{ makeGetValue(0, 'argIndex', 'i32') }}};
        }
        argIndex += {{{ QUANTUM_SIZE === 1 ? 1 : 'Math.max(4, size || 0)' }}};
      }
      return +ret; // +: boolean=>int
    }

    var ret = [];
    var curr, next, currArg;
    while(1) {
      var startTextIndex = textIndex;
      curr = {{{ makeGetValue(0, 'textIndex', 'i8') }}};
      if (curr === 0) break;
      next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
      if (curr == '%'.charCodeAt(0)) {
        // Handle flags.
        var flagAlwaysSigned = false;
        var flagLeftAlign = false;
        var flagAlternative = false;
        var flagZeroPad = false;
        flagsLoop: while (1) {
          switch (next) {
            case '+'.charCodeAt(0):
              flagAlwaysSigned = true;
              break;
            case '-'.charCodeAt(0):
              flagLeftAlign = true;
              break;
            case '#'.charCodeAt(0):
              flagAlternative = true;
              break;
            case '0'.charCodeAt(0):
              if (flagZeroPad) {
                break flagsLoop;
              } else {
                flagZeroPad = true;
                break;
              }
            default:
              break flagsLoop;
          }
          textIndex++;
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
        }

        // Handle width.
        var width = 0;
        if (next == '*'.charCodeAt(0)) {
          width = getNextArg();
          textIndex++;
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
        } else {
          while (_isdigit(next)) {
            width = width * 10 + (next - '0'.charCodeAt(0));
            textIndex++;
            next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
          }
        }

        // Handle precision.
        var precisionSet = false;
        if (next == '.'.charCodeAt(0)) {
          var precision = 0;
          precisionSet = true;
          textIndex++;
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
          if (next == '*'.charCodeAt(0)) {
            precision = getNextArg();
            textIndex++;
          } else {
            while(1) {
              var precisionChr = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
              if (!_isdigit(precisionChr)) break;
              precision = precision * 10 + (precisionChr - '0'.charCodeAt(0));
              textIndex++;
            }
          }
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
        } else {
          var precision = 6; // Standard default.
        }

        // Handle integer sizes. WARNING: These assume a 32-bit architecture!
        var argSize;
        switch (String.fromCharCode(next)) {
          case 'h':
            var nextNext = {{{ makeGetValue(0, 'textIndex+2', 'i8') }}};
            if (nextNext == 'h'.charCodeAt(0)) {
              textIndex++;
              argSize = 1; // char
            } else {
              argSize = 2; // short
            }
            break;
          case 'l':
            var nextNext = {{{ makeGetValue(0, 'textIndex+2', 'i8') }}};
            if (nextNext == 'l'.charCodeAt(0)) {
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
            argSize = undefined;
        }
        if (argSize !== undefined) textIndex++;
        next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};

        // Handle type specifier.
        if (['d', 'i', 'u', 'o', 'x', 'X', 'p'].indexOf(String.fromCharCode(next)) != -1) {
          // Integer.
          var signed = next == 'd'.charCodeAt(0) || next == 'i'.charCodeAt(0);
          var currArg = getNextArg(false, argSize);
          // Truncate to requested size.
          argSize = argSize || 4;
          if (argSize <= 4) {
            var limit = Math.pow(256, argSize) - 1;
            currArg = (signed ? reSign : unSign)(currArg & limit, argSize * 8);
          }
          // Format the number.
          var currAbsArg = Math.abs(currArg);
          var argText;
          var prefix = '';
          if (next == 'd'.charCodeAt(0) || next == 'i'.charCodeAt(0)) {
            argText = currAbsArg.toString(10);
          } else if (next == 'u'.charCodeAt(0)) {
            argText = unSign(currArg, 8 * argSize).toString(10);
            currArg = Math.abs(currArg);
          } else if (next == 'o'.charCodeAt(0)) {
            argText = (flagAlternative ? '0' : '') + currAbsArg.toString(8);
          } else if (next == 'x'.charCodeAt(0) || next == 'X'.charCodeAt(0)) {
            prefix = flagAlternative ? '0x' : '';
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
            if (next == 'X'.charCodeAt(0)) {
              prefix = prefix.toUpperCase();
              argText = argText.toUpperCase();
            }
          } else if (next == 'p'.charCodeAt(0)) {
            prefix = '0x';
            argText = currAbsArg.toString(16);
          }
          if (precisionSet) {
            while (argText.length < precision) {
              argText = '0' + argText;
            }
          }

          // Add sign.
          if (currArg < 0) {
            prefix = '-' + prefix;
          } else if (flagAlwaysSigned) {
            prefix = '+' + prefix;
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
        } else if (['f', 'F', 'e', 'E', 'g', 'G'].indexOf(String.fromCharCode(next)) != -1) {
          // Float.
          var currArg = getNextArg(true, argSize);
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
            if (next == 'g'.charCodeAt(0) || next == 'G'.charCodeAt(0)) {
              isGeneral = true;
              precision = precision || 1;
              var exponent = parseInt(currArg.toExponential(effectivePrecision).split('e')[1], 10);
              if (precision > exponent && exponent >= -4) {
                next = ((next == 'g'.charCodeAt(0)) ? 'f' : 'F').charCodeAt(0);
                precision -= exponent + 1;
              } else {
                next = ((next == 'g'.charCodeAt(0)) ? 'e' : 'E').charCodeAt(0);
                precision--;
              }
              effectivePrecision = Math.min(precision, 20);
            }

            if (next == 'e'.charCodeAt(0) || next == 'E'.charCodeAt(0)) {
              argText = currArg.toExponential(effectivePrecision);
              // Make sure the exponent has at least 2 digits.
              if (/[eE][-+]\d$/.test(argText)) {
                argText = argText.slice(0, -1) + '0' + argText.slice(-1);
              }
            } else if (next == 'f'.charCodeAt(0) || next == 'F'.charCodeAt(0)) {
              argText = currArg.toFixed(effectivePrecision);
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
            if (next == 'E'.charCodeAt(0)) argText = argText.toUpperCase();

            // Add sign.
            if (flagAlwaysSigned && currArg >= 0) {
              argText = '+' + argText;
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
          if (next < 'a'.charCodeAt(0)) argText = argText.toUpperCase();

          // Insert the result into the buffer.
          argText.split('').forEach(function(chr) {
            ret.push(chr.charCodeAt(0));
          });
        } else if (next == 's'.charCodeAt(0)) {
          // String.
          var copiedString = String_copy(getNextArg());
          if (precisionSet && copiedString.length > precision) {
            copiedString = copiedString.slice(0, precision);
          }
          if (!flagLeftAlign) {
            while (copiedString.length < width--) {
              ret.push(' '.charCodeAt(0));
            }
          }
          ret = ret.concat(copiedString);
          if (flagLeftAlign) {
            while (copiedString.length < width--) {
              ret.push(' '.charCodeAt(0));
            }
          }
        } else if (next == 'c'.charCodeAt(0)) {
          // Character.
          if (flagLeftAlign) ret.push(getNextArg());
          while (--width > 0) {
            ret.push(' '.charCodeAt(0));
          }
          if (!flagLeftAlign) ret.push(getNextArg());
        } else if (next == 'n'.charCodeAt(0)) {
          // Write the length written so far to the next parameter.
          {{{ makeSetValue('getNextArg()', '0', 'ret.length', 'i32') }}}
        } else if (next == '%'.charCodeAt(0)) {
          // Literal percent sign.
          ret.push(curr);
        } else {
          // Unknown specifiers remain untouched.
          for (var i = startTextIndex; i < textIndex + 2; i++) {
            ret.push({{{ makeGetValue(0, 'i', 'i8') }}});
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
    return Pointer_make(ret.concat(0), 0, ALLOC_STACK, 'i8'); // NB: Stored on the stack
    //var len = ret.length+1;
    //var ret = Pointer_make(ret.concat(0), 0, ALLOC_STACK); // NB: Stored on the stack
    //STACKTOP -= len; // XXX horrible hack. we rewind the stack, to 'undo' the alloc we just did.
    //                 // the point is that this works if nothing else allocs on the stack before
    //                 // the string is read, which should be true - it is very transient, see the *printf* functions below.
    //return ret;
  },

  printf__deps: ['_formatString'],
  printf: function() {
    __print__(Pointer_stringify(__formatString.apply(null, arguments)));
  },

  sprintf__deps: ['strcpy', '_formatString'],
  sprintf: function() {
    var str = arguments[0];
    var args = Array.prototype.slice.call(arguments, 1);
    _strcpy(str, __formatString.apply(null, args)); // not terribly efficient
  },

  snprintf__deps: ['strncpy', '_formatString'],
  snprintf: function() {
    var str = arguments[0];
    var num = arguments[1];
    var args = Array.prototype.slice.call(arguments, 2);
    _strncpy(str, __formatString.apply(null, args), num); // not terribly efficient
  },

  puts: function(p) {
    __print__(Pointer_stringify(p) + '\n');
  },

  putc: 'fputc',
  _IO_putc: 'fputc',

  putchar: function(p) {
    __print__(String.fromCharCode(p));
  },
  _ZNSo3putEc: 'putchar',

  _ZNSo5flushEv: function() {
    __print__('\n');
  },

  vsprintf__deps: ['strcpy', '_formatString'],
  vsprintf: function(dst, src, ptr) {
    _strcpy(dst, __formatString(-src, ptr));
  },

  vsnprintf__deps: ['_formatString'],
  vsnprintf: function(dst, num, src, ptr) {
    var text = __formatString(-src, ptr); // |-|src tells formatstring to use C-style params (typically they are from varargs)
    var i;
    for (i = 0; i < num; i++) {
      {{{ makeCopyValues('dst+i', 'text+i', 1, 'i8') }}}
      if ({{{ makeGetValue('dst', 'i', 'i8') }}} == 0) break;
    }
    return i; // Actually, should return how many *would* have been written, if the |num| had not stopped us.
  },

  fileno: function(file) {
    return file;
  },

  isatty: function(file) {
    return 0; // TODO
  },

  clearerr: function(stream) {
  },

  flockfile: function(file) {
  },

  funlockfile: function(file) {
  },

  // ==========================================================================
  // stdio.h - file functions
  // ==========================================================================

  stdin: 0,
  stdout: 0,
  stderr: 0,

  $STDIO__postset: 'STDIO.init()',
  $STDIO__deps: ['stdin', 'stdout', 'stderr'],
  $STDIO: {
    streams: {},
    filenames: {},
    counter: 1,
    SEEK_SET: 0, /* Beginning of file.  */
    SEEK_CUR: 1, /* Current position.   */
    SEEK_END: 2, /* End of file.        */
    init: function() {
      _stdin = Pointer_make([0], null, ALLOC_STATIC, 'void*');
      {{{ makeSetValue('_stdin', '0', "STDIO.prepare('<<stdin>>', null, null, true)", 'i32') }}};
      if (Module.stdin) {
        // Make sure stdin returns a newline
        var orig = Module.stdin;
        Module.stdin = function stdinFixed(prompt) {
          var ret = orig(prompt);
          if (ret[ret.length-1] !== '\n') ret = ret + '\n';
          return ret;
        }
      } else {
        Module.stdin = function stdin(prompt) {
          return window.prompt(prompt) || '';
        };
      }

      _stdout = Pointer_make([0], null, ALLOC_STATIC, 'void*');
      {{{ makeSetValue('_stdout', '0', "STDIO.prepare('<<stdout>>', null, true)", 'i32') }}};

      _stderr = Pointer_make([0], null, ALLOC_STATIC, 'void*');
      {{{ makeSetValue('_stderr', '0', "STDIO.prepare('<<stderr>>', null, true)", 'i32') }}};
    },
    cleanFilename: function(filename) {
      return filename.replace('./', '');
    },
    prepare: function(filename, data, print_, interactiveInput) {
      filename = STDIO.cleanFilename(filename);
      var stream = STDIO.counter++;
      STDIO.streams[stream] = {
        filename: filename,
        data: data ? data : [],
        position: 0,
        eof: 0,
        error: 0,
        interactiveInput: interactiveInput, // true for stdin - on the web, we allow interactive input
        print: print_ // true for stdout and stderr - we print when receiving data for them
      };
      STDIO.filenames[filename] = stream;
      return stream;
    },
    open: function(filename) {
      filename = STDIO.cleanFilename(filename);
      var stream = STDIO.filenames[filename];
      if (!stream) {
        // Not already cached; try to load it right now
        try {
          return STDIO.prepare(filename, readBinary(filename));
        } catch(e) {
          return 0;
        }
      }
      var info = STDIO.streams[stream];
      info.position = info.error = info.eof = 0;
      return stream;
    },
    read: function(stream, ptr, size) {
      var info = STDIO.streams[stream];
      if (!info) return -1;
      if (info.interactiveInput) {
        for (var i = 0; i < size; i++) {
          if (info.data.length === 0) {
            info.data = intArrayFromString(Module.stdin(PRINTBUFFER.length > 0 ? PRINTBUFFER : '?')).map(function(x) { return x === 0 ? 10 : x }); // change 0 to newline
            PRINTBUFFER = '';
            if (info.data.length === 0) return i;
          }
          {{{ makeSetValue('ptr', '0', 'info.data.shift()', 'i8') }}}
          ptr++;
        }
        return size;
      }
      for (var i = 0; i < size; i++) {
        if (info.position >= info.data.length) {
          info.eof = 1;
          return 0; // EOF
        }
        {{{ makeSetValue('ptr', '0', 'info.data[info.position]', 'i8') }}}
        info.position++;
        ptr++;
      }
      return size;
    },
    write: function(stream, ptr, size) {
      var info = STDIO.streams[stream];
      if (!info) return -1;
      if (info.print) {
        __print__(intArrayToString(Array_copy(ptr, size)));
      } else {
        for (var i = 0; i < size; i++) {
          info.data[info.position] = {{{ makeGetValue('ptr', '0', 'i8') }}};
          info.position++;
          ptr++;
        }
      }
      return size;
    }
  },

  fopen__deps: ['$STDIO'],
  fopen: function(filename, mode) {
    filename = Pointer_stringify(filename);
    mode = Pointer_stringify(mode);
    if (mode.indexOf('r') >= 0) {
      return STDIO.open(filename);
    } else if (mode.indexOf('w') >= 0) {
      return STDIO.prepare(filename);
    } else {
      return assert(false, 'fopen with odd params: ' + mode);
    }
  },
  __01fopen64_: 'fopen',

  fdopen: function(descriptor, mode) {
    // TODO: Check whether mode is acceptable for the current stream.
    return descriptor;
  },

  rewind__deps: ['$STDIO'],
  rewind: function(stream) {
    var info = STDIO.streams[stream];
    info.position = 0;
    info.error = 0;
  },

  fseek__deps: ['$STDIO'],
  fseek: function(stream, offset, whence) {
    var info = STDIO.streams[stream];
    if (whence === STDIO.SEEK_CUR) {
      offset += info.position;
    } else if (whence === STDIO.SEEK_END) {
      offset += info.data.length;
    }
    info.position = offset;
    info.eof = 0;
    return 0;
  },
  __01fseeko64_: 'fseek',
  __01lseek64_: 'fseek',

  ftell__deps: ['$STDIO'],
  ftell: function(stream) {
    return STDIO.streams[stream].position;
  },
  __01ftello64_: 'ftell',

  fread__deps: ['$STDIO'],
  fread: function(ptr, size, count, stream) {
    var info = STDIO.streams[stream];
    if (info.interactiveInput) return STDIO.read(stream, ptr, size*count);
    for (var i = 0; i < count; i++) {
      if (info.position + size > info.data.length) {
        info.eof = 1;
        return i;
      }
      STDIO.read(stream, ptr, size);
      ptr += size;
    }
    return count;
  },

  fwrite__deps: ['$STDIO'],
  fwrite: function(ptr, size, count, stream) {
    STDIO.write(stream, ptr, size*count);
    return count;
  },

  fclose__deps: ['$STDIO'],
  fclose: function(stream) {
    return 0;
  },

  feof__deps: ['$STDIO'],
  feof: function(stream) {
    return STDIO.streams[stream].eof;
  },

  ferror__deps: ['$STDIO'],
  ferror: function(stream) {
    return STDIO.streams[stream].error;
  },

  fprintf__deps: ['_formatString', '$STDIO'],
  fprintf: function() {
    var stream = arguments[0];
    var args = Array.prototype.slice.call(arguments, 1);
    var ptr = __formatString.apply(null, args);
    STDIO.write(stream, ptr, String_len(ptr));
  },

  vfprintf__deps: ['$STDIO', '_formatString'],
  vfprintf: function(stream, format, args) {
    var ptr = __formatString(-format, args);
    STDIO.write(stream, ptr, String_len(ptr));
  },

  fflush__deps: ['$STDIO'],
  fflush: function(stream) {
    var info = STDIO.streams[stream];
    if (info && info.print) {
      __print__(null);
    }
  },

  fputs__deps: ['$STDIO', 'fputc'],
  fputs: function(p, stream) {
    STDIO.write(stream, p, String_len(p));
  },

  fputc__deps: ['$STDIO'],
  fputc: function(chr, stream) {
    if (!Module._fputc_ptr) Module._fputc_ptr = _malloc(1);
    {{{ makeSetValue('Module._fputc_ptr', '0', 'chr', 'i8') }}}
    var ret = STDIO.write(stream, Module._fputc_ptr, 1);
    return (ret == -1) ? -1 /* EOF */ : chr;
  },

  getc__deps: ['$STDIO'],
  getc: function(file) {
    if (!Module._getc_ptr) Module._getc_ptr = _malloc(1);
    var ret = STDIO.read(file, Module._getc_ptr, 1);
    if (ret === 0) return -1; // EOF
    return {{{ makeGetValue('Module._getc_ptr', '0', 'i8') }}}
  },
  getc_unlocked: 'getc',
  _IO_getc: 'getc',

  getchar__deps: ['getc'],
  getchar: function() {
    return _getc(_stdin);
  },

  ungetc: function(chr, stream) {
    var f = STDIO.streams[stream];
    if (!f)
      return -1; // EOF
    if (!f.interactiveInput)
      f.position--;
    return chr;
  },

  gets: function(ptr) {
    var num = 0;
    while (STDIO.read({{{ makeGetValue('_stdin', '0', 'void*') }}}, ptr+num, 1) &&
           {{{ makeGetValue('ptr', 'num', 'i8') }}} !== 10) { num++; }
    if (num === 0) return 0;
    {{{ makeSetValue('ptr', 'num', 0, 'i8') }}}
    return ptr;
  },

  fscanf__deps: ['_scanString'],
  fscanf: function(stream, format) {
    var f = STDIO.streams[stream];
    if (!f)
      return -1; // EOF
    arguments[0] = Pointer_make(f.data.slice(f.position).concat(0), 0, ALLOC_STACK, 'i8');
    var ret = __scanString.apply(null, arguments);
    f.position += ret.bytes;
    return ret.fields;
  },

  // unix file IO, see http://rabbit.eng.miami.edu/info/functions/unixio.html

  open: function(filename, flags, mode) {
    filename = Pointer_stringify(filename);
    if (flags === 0) { // RDONLY
      return STDIO.open(filename);
    } else if (flags === 1) { // WRONLY
      return STDIO.prepare(filename);
    } else {
      return assert(false, 'open with odd params: ' + [flags, mode]);
    }
  },

  __01open64___deps: ['open'],
  __01open64_: function(filename, mode, flags) {
    // open(), but with flags and mode switched.
    // TODO: Verify why this happens at all.
    return _open(filename, flags, mode);
  },

  close: function(stream) {
    return 0;
  },

  read: function(stream, ptr, numbytes) {
    return STDIO.read(stream, ptr, numbytes);
  },

  fcntl: function() { }, // TODO...

  fstat: function(stream, ptr) {
    var info = STDIO.streams[stream];
    if (!info) return -1;
    // XXX: hardcoded indexes into the structure.
    try {
      {{{ makeSetValue('ptr', '$struct_stat___FLATTENER[0]', '1', 'i32') }}} // st_dev
      {{{ makeSetValue('ptr', '$struct_stat___FLATTENER[15]', 'stream', 'i32') }}} // st_ino
      {{{ makeSetValue('ptr', '$struct_stat___FLATTENER[9]', 'info.data.length', 'i32') }}} // st_size
    } catch(e) {
      // no FLATTENER
      {{{ makeSetValue('ptr', '0', '1', 'i32') }}}
      {{{ makeSetValue('ptr', '15', 'stream', 'i32') }}}
      {{{ makeSetValue('ptr', '9', 'info.data.length', 'i32') }}}
    }
    // TODO: other fields
    return 0;
  },

  stat__deps: ['open', 'fstat'],
  stat: function(filename, ptr) {
    if (typeof window === 'undefined') {
      // XXX d8 hangs if you try to read a folder.
      // http://code.google.com/p/v8/issues/detail?id=1533
      return 0;
    }
    // TODO: Handle symbolic links.
    var stream = _open(filename, 0, 256); // RDONLY, 0400.
    return _fstat(stream, ptr);
  },

  mmap: function(start, num, prot, flags, stream, offset) {
    // Leaky and non-shared... FIXME
    var info = STDIO.streams[stream];
    if (!info) return -1;
    return Pointer_make(info.data.slice(offset, offset+num), null, ALLOC_NORMAL, 'i8');
  },

  munmap: function(start, num) {
    _free(start); // FIXME: not really correct at all
  },

  setbuf: function(stream, buffer) {
    // just a stub
    assert(!buffer);
  },

  setvbuf: 'setbuf',

  access: function(filename) {
    filename = Pointer_stringify(filename);
    return STDIO.open(filename) ? 0 : -1;
  },

  // ==========================================================================
  // stdlib.h
  // ==========================================================================

  malloc: Runtime.staticAlloc,
  _Znwj: 'malloc',
  _Znaj: 'malloc',
  _Znam: 'malloc',
  _Znwm: 'malloc',

  free: function(){},
  _ZdlPv: 'free',
  _ZdaPv: 'free',

  calloc__deps: ['malloc'],
  calloc: function(n, s) {
    var ret = _malloc(n*s);
    _memset(ret, 0, n*s);
    return ret;
  },

  abs: 'Math.abs', // XXX should be integer?

  atoi: function(s) {
    return Math.floor(Number(Pointer_stringify(s)));
  },

  exit: function(status) {
    __shutdownRuntime__();
    ABORT = true;
    throw 'exit(' + status + ') called, at ' + new Error().stack;
  },

  atexit: function(func, arg) {
    __ATEXIT__.push({ func: func, arg: arg });
  },
  __cxa_atexit: 'atexit',

  abort: function(code) {
    ABORT = true;
    throw 'ABORT: ' + code + ', at ' + (new Error().stack);
  },

  realloc__deps: ['memcpy'],
  realloc: function(ptr, size) {
    // Very simple, inefficient implementation - if you use a real malloc, best to use
    // a real realloc with it
    if (!size) {
      if (ptr) _free(ptr);
      return 0;
    }
    var ret = _malloc(size);
    if (ptr) {
      _memcpy(ret, ptr, size); // might be some invalid reads
      _free(ptr);
    }
    return ret;
  },

  getenv: function(name_) {
    return 0; // TODO
  },

  strtod__deps: ['isspace', 'isdigit'],
  strtod: function(str, endptr) {
    // Skip space.
    while (_isspace(str)) str++;

    var chr;
    var ret = 0;

    // Get whole part.
    while(1) {
      chr = {{{ makeGetValue('str', 0, 'i8') }}};
      if (!_isdigit(chr)) break;
      ret = ret*10 + chr - '0'.charCodeAt(0);
      str++;
    }

    // Get fractional part.
    if ({{{ makeGetValue('str', 0, 'i8') }}} == '.'.charCodeAt(0)) {
      str++;
      var mul = 1/10;
      while(1) {
        chr = {{{ makeGetValue('str', 0, 'i8') }}};
        if (!_isdigit(chr)) break;
        ret += mul*(chr - '0'.charCodeAt(0));
        mul /= 10;
        str++;
      }
    }

    // Get exponent part.
    chr = {{{ makeGetValue('str', 0, 'i8') }}};
    if (chr == 'e'.charCodeAt(0) || chr == 'E'.charCodeAt(0)) {
      str++;
      var exponent = 0;
      var expNegative = false;
      chr = {{{ makeGetValue('str', 0, 'i8') }}};
      if (chr == '-'.charCodeAt(0)) {
        expNegative = true;
        str++;
      } else if (chr == '+'.charCodeAt(0)) {
        str++;
      }
      chr = {{{ makeGetValue('str', 0, 'i8') }}};
      while(1) {
        if (!_isdigit(chr)) break;
        exponent = exponent*10 + chr - '0'.charCodeAt(0);
        str++;
        chr = {{{ makeGetValue('str', 0, 'i8') }}};
      }
      if (expNegative) exponent = -exponent;
      ret *= Math.pow(10, exponent);
    }

    // Set end pointer.
    if (endptr) {
      {{{ makeSetValue('endptr', 0, 'str', '*') }}}
    }

    return ret;
  },

  qsort__deps: ['memcpy'],
  qsort: function(base, num, size, comparator) {
    // forward calls to the JavaScript sort method
    // first, sort the items logically
    comparator = FUNCTION_TABLE[comparator];
    var keys = [];
    for (var i = 0; i < num; i++) keys.push(i);
    keys.sort(function(a, b) {
      return comparator(base+a*size, base+b*size);
    });
    // apply the sort
    var temp = _malloc(num*size);
    _memcpy(temp, base, num*size);
    for (var i = 0; i < num; i++) {
      if (keys[i] == i) continue; // already in place
      _memcpy(base+i*size, temp+keys[i]*size, size);
    }
    _free(temp);
  },

  // ==========================================================================
  // string.h
  // ==========================================================================

  memcpy: function (dest, src, num, idunno) {
#if ASSERTIONS
    assert(num % 1 === 0, 'memcpy given ' + num + ' bytes to copy. Problem with QUANTUM_SIZE=1 corrections perhaps?');
#endif
    // || 0, since memcpy sometimes copies uninitialized areas XXX: Investigate why initializing alloc'ed memory does not fix that too
    {{{ makeCopyValues('dest', 'src', 'num', 'null', ' || 0') }}};
  },
  llvm_memcpy_i32: 'memcpy',
  llvm_memcpy_i64: 'memcpy',
  llvm_memcpy_p0i8_p0i8_i32: 'memcpy',
  llvm_memcpy_p0i8_p0i8_i64: 'memcpy',

  memmove__deps: ['memcpy'],
  memmove: function(dest, src, num, idunno) {
    // not optimized!
    if (num === 0) return; // will confuse malloc if 0
    var tmp = _malloc(num);
    _memcpy(tmp, src, num);
    _memcpy(dest, tmp, num);
    _free(tmp);
  },
  llvm_memmove_i32: 'memmove',
  llvm_memmove_i64: 'memmove',
  llvm_memmove_p0i8_p0i8_i32: 'memmove',
  llvm_memmove_p0i8_p0i8_i64: 'memmove',

  memset: function(ptr, value, num) {
    {{{ makeSetValues('ptr', '0', 'value', 'null', 'num') }}}
  },
  llvm_memset_i32: 'memset',
  llvm_memset_p0i8_i32: 'memset',
  llvm_memset_p0i8_i64: 'memset',

  strlen: function(ptr) {
    return String_len(ptr);
  },

  strspn: function(pstr, pset) {
    var str = String_copy(pstr, true);
    var set = String_copy(pset);
    var i = 0;
    while (set.indexOf(str[i]) != -1) i++; // Must halt, as 0 is in str but not set
    return i;
  },

  strcspn: function(pstr, pset) {
    var str = String_copy(pstr, true);
    var set = String_copy(pset, true);
    var i = 0;
    while (set.indexOf(str[i]) == -1) i++; // Must halt, as 0 is in both
    return i;
  },

  strcpy: function(pdest, psrc) {
    var i = 0;
    do {
      {{{ makeCopyValues('pdest+i', 'psrc+i', 1, 'i8') }}}
      i ++;
    } while ({{{ makeGetValue('psrc', 'i-1', 'i8') }}} != 0);
  },

  strncpy: function(pdest, psrc, num) {
    var padding = false, curr;
    for (var i = 0; i < num; i++) {
      curr = padding ? 0 : {{{ makeGetValue('psrc', 'i', 'i8') }}};
      {{{ makeSetValue('pdest', 'i', 'curr', 'i8') }}}
      padding = padding || {{{ makeGetValue('psrc', 'i', 'i8') }}} == 0;
    }
  },

  strcat: function(pdest, psrc) {
    var len = Pointer_stringify(pdest).length; // TODO: use strlen, but need dependencies system
    var i = 0;
    do {
      {{{ makeCopyValues('pdest+len+i', 'psrc+i', 1, 'i8') }}}
      i ++;
    } while ({{{ makeGetValue('psrc', 'i-1', 'i8') }}} != 0);
    return pdest;
  },

  strncat: function(pdest, psrc, num) {
    var len = Pointer_stringify(pdest).length; // TODO: use strlen, but need dependencies system
    var i = 0;
    while(1) {
      {{{ makeCopyValues('pdest+len+i', 'psrc+i', 1, 'i8') }}}
      if ({{{ makeGetValue('pdest', 'len+i', 'i8') }}} == 0) break;
      i ++;
      if (i == num) {
        {{{ makeSetValue('pdest', 'len+i', 0, 'i8') }}}
        break;
      }
    }
    return pdest;
  },

  strtol: function(ptr) {
    assert(!arguments[1] && !arguments[2], "We don't support all strtol params yet");
    return parseInt(Pointer_stringify(ptr));
  },

  strcmp__deps: ['strncmp'],
  strcmp: function(px, py) {
    return _strncmp(px, py, TOTAL_MEMORY);
  },

  strcasecmp__deps: ['strncasecmp'],
  strcasecmp: function(px, py) {
    return _strncasecmp(px, py, TOTAL_MEMORY);
  },

  strncmp: function(px, py, n) {
    var i = 0;
    while (i < n) {
      var x = {{{ makeGetValue('px', 'i', 'i8') }}};
      var y = {{{ makeGetValue('py', 'i', 'i8') }}};
      if (x == y && x == 0) return 0;
      if (x == 0) return -1;
      if (y == 0) return 1;
      if (x == y) {
        i ++;
        continue;
      } else {
        return x > y ? 1 : -1;
      }
    }
    return 0;
  },

  strncasecmp__deps: ['tolower'],
  strncasecmp: function(px, py, n) {
    var i = 0;
    while (i < n) {
      var x = _tolower({{{ makeGetValue('px', 'i', 'i8') }}});
      var y = _tolower({{{ makeGetValue('py', 'i', 'i8') }}});
      if (x == y && x == 0) return 0;
      if (x == 0) return -1;
      if (y == 0) return 1;
      if (x == y) {
        i ++;
        continue;
      } else {
        return x > y ? 1 : -1;
      }
    }
    return 0;
  },

  memcmp: function(p1, p2, num) {
    for (var i = 0; i < num; i++) {
      var v1 = {{{ makeGetValue('p1', 'i', 'i8') }}};
      var v2 = {{{ makeGetValue('p2', 'i', 'i8') }}};
      if (v1 != v2) return v1 > v2 ? 1 : -1;
    }
    return 0;
  },

  memchr: function(ptr, chr, num) {
    chr = unSign(chr);
    for (var i = 0; i < num; i++) {
      if ({{{ makeGetValue('ptr', 0, 'i8') }}} == chr) return ptr;
      ptr++;
    }
    return 0;
  },

  strstr: function(ptr1, ptr2) {
    var str1 = Pointer_stringify(ptr1);
    var str2 = Pointer_stringify(ptr2);
    var ret = str1.search(str2);
    return ret >= 0 ? ptr1 + ret : 0;
  },

  strchr: function(ptr, chr) {
    ptr--;
    do {
      ptr++;
      var val = {{{ makeGetValue('ptr', 0, 'i8') }}};
      if (val == chr) return ptr;
    } while (val);
    return 0;
  },

  strrchr: function(ptr, chr) {
    var ptr2 = ptr + Pointer_stringify(ptr).length; // TODO: use strlen, but need dependencies system
    do {
      if ({{{ makeGetValue('ptr2', 0, 'i8') }}} == chr) return ptr2;
      ptr2--;
    } while (ptr2 >= ptr);
    return 0;
  },

  strdup: function(ptr) {
    return Pointer_make(String_copy(ptr, true), 0, ALLOC_NORMAL, 'i8');
  },

  strpbrk: function(ptr1, ptr2) {
    var searchSet = Runtime.set.apply(null, String_copy(ptr2));
    while ({{{ makeGetValue('ptr1', 0, 'i8') }}}) {
      if ({{{ makeGetValue('ptr1', 0, 'i8') }}} in searchSet) return ptr1;
      ptr1++;
    }
    return 0;
  },

  // Compiled from newlib; for the original source and licensing, see library_strtok_r.c XXX will not work with typed arrays
  strtok_r: function(b,j,f){var a;a=null;var c,e;b=b;var i=b!=0;a:do if(i)a=0;else{b=HEAP[f];if(b!=0){a=0;break a}c=0;a=3;break a}while(0);if(a==0){a:for(;;){e=HEAP[b];b+=1;a=j;var g=e;i=a;a=2;b:for(;;){d=a==5?d:0;a=HEAP[i+d];if(a!=0==0){a=9;break a}var d=d+1;if(g==a)break b;else a=5}a=2}if(a==9)if(g==0)c=HEAP[f]=0;else{c=b+-1;a:for(;;){e=HEAP[b];b+=1;a=j;g=e;d=a;a=10;b:for(;;){h=a==13?h:0;a=HEAP[d+h];if(a==g!=0)break a;var h=h+1;if(a!=0)a=13;else break b}}if(e==0)b=0;else HEAP[b+-1]=0; HEAP[f]=b;c=c}else if(a==7){HEAP[f]=b;HEAP[b+-1]=0;c=b+-1}}return c},

  strerror_r__deps: ['$ERRNO_CODES', '$ERRNO_MESSAGES', '__setErrNo'],
  strerror_r: function(errnum, strerrbuf, buflen) {
    if (errnum in ERRNO_MESSAGES) {
      if (ERRNO_MESSAGES[errnum].length > buflen - 1) {
        return ___setErrNo(ERRNO_CODES.ERANGE);
      } else {
        var msg = ERRNO_MESSAGES[errnum];
        for (var i = 0; i < msg.length; i++) {
          {{{ makeSetValue('strerrbuf', 'i', 'msg.charCodeAt(i)', 'i8') }}}
        }
        {{{ makeSetValue('strerrbuf', 'i', 0, 'i8') }}}
        return 0;
      }
    } else {
      return ___setErrNo(ERRNO_CODES.EINVAL);
    }
  },
  strerror__deps: ['strerror_r'],
  strerror: function(errnum) {
    if (!_strerror.buffer) _strerror.buffer = _malloc(256);
    _strerror_r(errnum, _strerror.buffer, 256);
    return _strerror.buffer;
  },

  // ==========================================================================
  // ctype.h
  // ==========================================================================

  isdigit: function(chr) {
    return chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0);
  },

  isxdigit: function(chr) {
    return (chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0)) ||
           (chr >= 'a'.charCodeAt(0) && chr <= 'f'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'F'.charCodeAt(0));
  },

  isalpha: function(chr) {
    return (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0));
  },

  isalnum: function(chr) {
    return (chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0)) ||
           (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0));
  },

  isspace: function(chr) {
    return chr in { 32: 0, 9: 0, 10: 0, 11: 0, 12: 0, 13: 0 };
  },

  iscntrl: function(chr) {
    return (chr >= 0 && chr <= 0x1f) || chr === 0x7f;
  },

  isprint__deps: ['iscntrl'],
  isprint: function(chr) {
    return !_iscntrl(chr);
  },

  toupper: function(chr) {
    if (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) {
      return chr - 'a'.charCodeAt(0) + 'A'.charCodeAt(0);
    }
    return chr;
  },

  tolower: function(chr) {
    if (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0)) {
      return chr - 'A'.charCodeAt(0) + 'a'.charCodeAt(0);
    }
    return chr;
  },

  // ==========================================================================
  // ctype.h Linux specifics
  // ==========================================================================

  __ctype_b_loc: function() { // http://refspecs.freestandards.org/LSB_3.0.0/LSB-Core-generic/LSB-Core-generic/baselib---ctype-b-loc.html
    var me = ___ctype_b_loc;
    if (!me.ret) {
      var values = [
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,8195,0,8194,0,8194,0,8194,0,8194,0,2,0,2,
        0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,24577,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,
        0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,55304,0,55304,0,55304,0,55304,0,55304,0,55304,
        0,55304,0,55304,0,55304,0,55304,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,54536,0,54536,0,54536,0,54536,
        0,54536,0,54536,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,
        0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,54792,0,54792,
        0,54792,0,54792,0,54792,0,54792,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,
        0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,49156,0,49156,0,49156,0,49156,0,2,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0
      ];
      me.ret = Pointer_make([Pointer_make(values, 0, ALLOC_STATIC, 'i16')+256], 0, ALLOC_STATIC, 'void*');
#if USE_TYPED_ARRAYS == 0
      assert(HEAP[HEAP[me.ret]] == 2);
      assert(HEAP[HEAP[me.ret]-2] == 0);
      assert(HEAP[HEAP[me.ret]+18] == 8195);
#endif
    }
    return me.ret;
  },

  // LLVM specifics

  llvm_va_copy: function(ppdest, ppsrc) {
    {{{ makeCopyValues('ppdest', 'ppsrc', QUANTUM_SIZE, 'null') }}}
    /* Alternate implementation that copies the actual DATA; it assumes the va_list is prefixed by its size
    var psrc = IHEAP[ppsrc]-1;
    var num = IHEAP[psrc]; // right before the data, is the number of (flattened) values
    var pdest = _malloc(num+1);
    _memcpy(pdest, psrc, num+1);
    IHEAP[ppdest] = pdest+1;
    */
  },

  llvm_bswap_i32: function(x) {
    x = unSign(x, 32);
    var bytes = [];
    for (var i = 0; i < 4; i++) {
      bytes[i] = x & 255;
      x >>= 8;
    }
    var ret = 0;
    for (i = 0; i < 4; i++) {
      ret <<= 8;
      ret += bytes[i];
    }
    return ret;
  },

  __assert_fail: function(condition, file, line) {
    ABORT = true;
    throw 'Assertion failed: ' + Pointer_stringify(condition);//JSON.stringify(arguments)//condition;
  },

  __cxa_guard_acquire: function() {
    return 1;
  },
  __cxa_guard_release: function() {
    return 1;
  },

  // Exceptions - minimal support, only (...) for now (no actual exception objects can be caught)
  __cxa_allocate_exception: function(size) {
    return _malloc(size); // warning: leaked
  },
  __cxa_throw: function(ptr, data, dunno) {
#if EXCEPTION_DEBUG
    print('Compiled code throwing an exception, ' + [ptr,data,dunno] + ', at ' + new Error().stack);
#endif
    throw ptr;
  },
  llvm_eh_exception: function() {
    return 'code-generated exception: ' + (new Error().stack);
  },
  llvm_eh_selector: function(exception, personality, num) {
    return 0;
  },
  __cxa_begin_catch: function(ptr) {
  },
  __cxa_end_catch: function(ptr) {
  },

  __cxa_call_unexpected: function(exception) {
    ABORT = true;
    throw exception;
  },

  __gxx_personality_v0: function() {
  },

  llvm_umul_with_overflow_i32: function(x, y) {
    return {
      f0: x*y,
      f1: 0 // We never overflow... for now
    };
  },

  llvm_stacksave: function() {
    var self = _llvm_stacksave;
    if (!self.LLVM_SAVEDSTACKS) {
      self.LLVM_SAVEDSTACKS = [];
    }
    self.LLVM_SAVEDSTACKS.push(STACKTOP);
    return self.LLVM_SAVEDSTACKS.length-1;
  },
  llvm_stackrestore: function(p) {
    var self = _llvm_stacksave;
    var ret = self.LLVM_SAVEDSTACKS[p];
    self.LLVM_SAVEDSTACKS.splice(p, 1);
    return ret;
  },

  __cxa_pure_virtual: function() {
    ABORT = true;
    throw 'Pure virtual function called!';
  },

  llvm_flt_rounds: function() {
    return -1; // 'indeterminable' for FLT_ROUNDS
  },

  // iostream

  _ZNSt8ios_base4InitC1Ev: function() {
    // need valid 'file descriptors'
    //__ZSt4cout = 1;
    //__ZSt4cerr = 2;
  },
  _ZNSt8ios_base4InitD1Ev: '_ZNSt8ios_base4InitC1Ev',
  _ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_: 0, // endl
  _ZNSolsEi: function(stream, data) {
    __print__(data);
  },
  _ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc: function(stream, data) {
    __print__(Pointer_stringify(data));
  },
  _ZNSolsEd: function(stream, data) {
    __print__('\n');
  },
  _ZNSolsEPFRSoS_E: function(stream, data) {
    __print__('\n');
  },
  _ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_i: function(stream, data, call_) {
    __print__(Pointer_stringify(data));
  },

  // ==========================================================================
  // math.h
  // ==========================================================================

  cos: 'Math.cos',
  cosf: 'Math.cos',
  sin: 'Math.sin',
  sinf: 'Math.sin',
  tan: 'Math.tan',
  tanf: 'Math.tan',
  acos: 'Math.acos',
  acosf: 'Math.acos',
  asin: 'Math.asin',
  asinf: 'Math.asin',
  atan: 'Math.atan',
  atanf: 'Math.atan',
  atan2: 'Math.atan2',
  atan2f: 'Math.atan2',
  sqrt: 'Math.sqrt',
  sqrtf: 'Math.sqrt',
  fabs: 'Math.abs',
  fabsf: 'Math.abs',
  ceil: 'Math.ceil',
  ceilf: 'Math.ceil',
  floor: 'Math.floor',
  floorf: 'Math.floor',
  llvm_sqrt_f32: 'Math.sqrt',
  llvm_sqrt_f64: 'Math.sqrt',
  llvm_pow_f32: 'Math.pow',
  llvm_pow_f64: 'Math.pow',
  llvm_log_f32: 'Math.log',
  llvm_log_f64: 'Math.log',
  ldexp: function(x, exp_) {
    return x*Math.pow(2, exp_);
  },

  modf: function(x, intpart) {
    {{{ makeSetValue('intpart', 0, 'Math.floor(x)', 'double') }}}
    return x - {{{ makeGetValue('intpart', 0, 'double') }}};
  },

  frexp: function(x, exp_addr) {
    var sig = 0, exp_ = 0;
    if (x !== 0) {
      var raw_exp = Math.log(x)/Math.log(2);
      exp_ = Math.ceil(raw_exp);
      if (exp_ === raw_exp) exp_ += 1;
      sig = x/Math.pow(2, exp_);
    }
    {{{ makeSetValue('exp_addr', 0, 'exp_', 'i32') }}}
    return sig;
  },

  finite: function(x) {
    return isFinite(x);
  },
  __finite: 'finite',

  isinf: function(x) {
    return !isNaN(x) && !isFinite(x);
  },
  __isinf: 'isinf',

  isnan: function(x) {
    return isNaN(x);
  },
  __isnan: 'isnan',

  copysign: function(a, b) {
      if (a<0 === b<0) return a;
      return -a;
  },

  hypot: function(a, b) {
     return Math.sqrt(a*a + b*b);
  },

  sinh: function(x) {
    var p = Math.pow(Math.E, x);
    return (p - (1 / p)) / 2;
  },

  cosh: function(x) {
    var p = Math.pow(Math.E, x);
    return (p + (1 / p)) / 2;
  },

  tanh__deps: ['sinh', 'cosh'],
  tanh: function(x) {
    return _sinh(x) / _cosh(x);
  },

  asinh: function(x) {
    return Math.log(x + Math.sqrt(x * x + 1));
  },

  acosh: function(x) {
    return Math.log(x * 1 + Math.sqrt(x * x - 1));
  },

  atanh: function(x) {
    return Math.log((1 + x) / (1 - x)) / 2;
  },

  // LLVM internal math

  exp2: function(x) {
    return Math.pow(2, x);
  },

  
  // ==========================================================================
  // dlfcn.h - Dynamic library loading
  //
  // Some limitations:
  //
  //  * Minification on each file separately may not work, as they will
  //    have different shortened names. You can in theory combine them, then
  //    minify, then split... perhaps.
  //
  //  * LLVM optimizations may fail. If the child wants to access a function
  //    in the parent, LLVM opts may remove it from the parent when it is
  //    being compiled. Not sure how to tell LLVM to not do so.
  // ==========================================================================

  // Data for dlfcn.h.
  $DLFCN_DATA: {
    error: null,
    isError: false,
    loadedLibs: {}, // handle -> [refcount, name, lib_object]
    loadedLibNames: {}, // name -> handle
  },
  // void* dlopen(const char* filename, int flag);
  dlopen__deps: ['$DLFCN_DATA'],
  dlopen: function(filename, flag) {
    // TODO: Add support for LD_LIBRARY_PATH.
    filename = Pointer_stringify(filename);

    if (DLFCN_DATA.loadedLibNames[filename]) {
      // Already loaded; increment ref count and return.
      var handle = DLFCN_DATA.loadedLibNames[filename];
      DLFCN_DATA.loadedLibs[handle][0]++;
      return handle;
    }

    try {
      var lib_data = read(filename);
    } catch (e) {
      DLFCN_DATA.isError = true;
      return 0;
    }

    try {
      var lib_module = eval(lib_data)(FUNCTION_TABLE.length);
    } catch (e) {
#if ASSERTIONS
      print('Error in loading dynamic library: ' + e);
#endif
      DLFCN_DATA.isError = true;
      return 0;
    }

    // Not all browsers support Object.keys().
    var handle = 1;
    for (var key in DLFCN_DATA.loadedLibs) {
      if (DLFCN_DATA.loadedLibs.hasOwnProperty(key)) handle++;
    }

    DLFCN_DATA.loadedLibs[handle] = [1, filename, lib_module];
    DLFCN_DATA.loadedLibNames[filename] = handle;

    // We don't care about RTLD_NOW and RTLD_LAZY.
    if (flag & 256) { // RTLD_GLOBAL
      for (var ident in lib_module) {
        if (lib_module.hasOwnProperty(ident)) {
          // TODO: Check if we need to unmangle here.
          Module[ident] = lib_module[ident];
        }
      }
    }

    return handle;
  },
  // int dlclose(void* handle);
  dlclose__deps: ['$DLFCN_DATA'],
  dlclose: function(handle) {
    if (!DLFCN_DATA.loadedLibs[handle]) {
      DLFCN_DATA.isError = true;
      return 1;
    } else {
      var lib_record = DLFCN_DATA.loadedLibs[handle];
      if (lib_record[0]-- == 0) {
        delete DLFCN_DATA.loadedLibNames[lib_record[1]];
        delete DLFCN_DATA.loadedLibs[handle];
      }
      return 0;
    }
  },
  // void* dlsym(void* handle, const char* symbol);
  dlsym__deps: ['$DLFCN_DATA'],
  dlsym: function(handle, symbol) {
    symbol = Pointer_stringify(symbol);
    // TODO: Properly mangle.
    symbol = '_' + symbol;

    if (!DLFCN_DATA.loadedLibs[handle]) {
      DLFCN_DATA.isError = true;
      return 0;
    } else {
      var lib_module = DLFCN_DATA.loadedLibs[handle][2];
      if (!lib_module[symbol]) {
        DLFCN_DATA.isError = true;
        return 0;
      } else {
        var result = lib_module[symbol];
        if (typeof result == 'function') {
          // TODO: Cache functions rather than appending on every lookup.
          FUNCTION_TABLE.push(result);
          FUNCTION_TABLE.push(0);
          result = FUNCTION_TABLE.length - 2;
        }
        return result;
      }
    }
  },
  // char* dlerror(void);
  dlerror__deps: ['$DLFCN_DATA'],
  dlerror: function() {
    if (DLFCN_DATA.isError) {
      return 0;
    } else {
      // TODO: Return non-generic error messages.
      if (DLFCN_DATA.error === null) {
        var msg = 'An error occurred while loading dynamic library.';
        var arr = Module.intArrayFromString(msg)
        DLFCN_DATA.error = Pointer_make(arr, 0, 2, 'i8');
      }
      DLFCN_DATA.isError = false;
      return DLFCN_DATA.error;
    }
  },

  // ==========================================================================
  // unistd.h
  // ==========================================================================

  getcwd__deps: ['malloc'],
  getcwd: function(buf, size) {
    // TODO: Implement for real once we have a file system.
    var path = window ? window.location.pathname.replace(/\/[^/]*$/, '') : '/';
    if (buf === 0) {
      // Null. Allocate manually.
      buf = _malloc(path.length);
    } else if (size < path.length) {
      return 0;
      // TODO: Set errno.
    }
    for (var i = 0; i < path.length; i++) {
      {{{ makeSetValue('buf', 'i', 'path[i].charCodeAt(0)', 'i8') }}}
    }
    return buf;
  },

  sysconf: function(name_) {
    // XXX we only handle _SC_PAGE_SIZE/PAGESIZE for now, 30 on linux, 29 on OS X... be careful here!
    switch(name_) {
      case 29: case 30: return PAGE_SIZE;
      case 2: return 1000000; // _SC_CLK_TCK
      default: throw 'unknown sysconf param: ' + name_;
    }
  },

  sbrk: function(bytes) {
    // Implement a Linux-like 'memory area' for our 'process'.
    // Changes the size of the memory area by |bytes|; returns the
    // address of the previous top ('break') of the memory area

    // We need to make sure no one else allocates unfreeable memory!
    // We must control this entirely. So we don't even need to do
    // unfreeable allocations - the HEAP is ours, from STATICTOP up.
    // TODO: We could in theory slice off the top of the HEAP when
    // sbrk gets a negative increment in |bytes|...
    var self = _sbrk;
    if (!self.STATICTOP) {
      STATICTOP = alignMemoryPage(STATICTOP);
      self.STATICTOP = STATICTOP;
      self.DATASIZE = 0;
    } else {
      assert(self.STATICTOP == STATICTOP, "Noone should touch the heap!");
    }
    var ret = STATICTOP + self.DATASIZE;
    self.DATASIZE += alignMemoryPage(bytes);
    return ret; // previous break location
  },

  readlink: function(path, buf, bufsiz) {
    return -1;
  },

  unlink: function(pathname) {
    var pathStr = Pointer_stringify(pathname);
    var fd = STDIO.filenames[pathStr];
    if (fd === undefined) {
      return -1;
    }
    delete STDIO.filenames[pathStr];
    return 0;
  },

  getuid: function() {
    return 100;
  },
  geteuid: 'getuid',

  getgid: function() {
    return 100;
  },
  getegid: 'getgid',

  getpwuid: function(uid) {
    return 0; // NULL
  },

  // ==========================================================================
  // time.h
  // ==========================================================================

  clock: function() {
    if (_clock.start === undefined) _clock.start = new Date();
    return (Date.now() - _clock.start.getTime()) * 1000;
  },

  time: function(ptr) {
    var ret = Math.floor(Date.now()/1000);
    if (ptr) {
      {{{ makeSetValue('ptr', 0, 'ret', 'i32') }}}
    }
    return ret;
  },

  difftime: function(time1, time0) {
    return time1 - time0;
  },

  __tm_struct_layout: Runtime.generateStructInfo([
    ['i32', 'tm_sec'],
    ['i32', 'tm_min'],
    ['i32', 'tm_hour'],
    ['i32', 'tm_mday'],
    ['i32', 'tm_mon'],
    ['i32', 'tm_year'],
    ['i32', 'tm_wday'],
    ['i32', 'tm_yday'],
    ['i32', 'tm_isdst'],
    ['i32', 'tm_gmtoff'],
    ['i8*', 'tm_zone'],
  ]),
  // Statically allocated time struct.
  __tm_current: 0,
  // Statically allocated timezone strings.
  __tm_timezones: {},
  // Statically allocated time strings.
  __tm_formatted: 0,

  mktime__deps: ['__tm_struct_layout', 'tzset'],
  mktime: function(tmPtr) {
    _tzset();
    var year = {{{ makeGetValue('tmPtr', '___tm_struct_layout.tm_year', 'i32') }}};
    var timestamp = new Date(year >= 1900 ? year : year + 1900,
                             {{{ makeGetValue('tmPtr', '___tm_struct_layout.tm_mon', 'i32') }}},
                             {{{ makeGetValue('tmPtr', '___tm_struct_layout.tm_mday', 'i32') }}},
                             {{{ makeGetValue('tmPtr', '___tm_struct_layout.tm_hour', 'i32') }}},
                             {{{ makeGetValue('tmPtr', '___tm_struct_layout.tm_min', 'i32') }}},
                             {{{ makeGetValue('tmPtr', '___tm_struct_layout.tm_sec', 'i32') }}},
                             0).getTime() / 1000;
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_wday', 'new Date(timestamp).getDay()', 'i32') }}}
    var yday = Math.round((timestamp - (new Date(year, 0, 1)).getTime()) / (1000 * 60 * 60 * 24));
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_yday', 'yday', 'i32') }}}
    return timestamp;
  },
  timelocal: 'mktime',

  gmtime__deps: ['malloc', '__tm_struct_layout', '__tm_current', 'gmtime_r'],
  gmtime: function(time) {
    if (!___tm_current) ___tm_current = _malloc(___tm_struct_layout.__size__);
    return _gmtime_r(time, ___tm_current);
  },

  gmtime_r__deps: ['__tm_struct_layout', '__tm_timezones'],
  gmtime_r: function(time, tmPtr) {
    var date = new Date({{{ makeGetValue('time', 0, 'i32') }}}*1000);
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_sec', 'date.getUTCSeconds()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_min', 'date.getUTCMinutes()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_hour', 'date.getUTCHours()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_mday', 'date.getUTCDate()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_mon', 'date.getUTCMonth()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_year', 'date.getUTCFullYear()-1900', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_wday', 'date.getUTCDay()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_gmtoff', '0', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_isdst', '0', 'i32') }}}

    var start = new Date(date.getFullYear(), 0, 1);
    var yday = Math.round((date.getTime() - start.getTime()) / (1000 * 60 * 60 * 24));
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_yday', 'yday', 'i32') }}}

    var timezone = "GMT";
    if (!(timezone in ___tm_timezones)) {
      ___tm_timezones[timezone] = Pointer_make(intArrayFromString(timezone), null, ALLOC_NORMAL, 'i8');
    }
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_zone', '___tm_timezones[timezone]', 'i32') }}}

    return tmPtr;
  },

  timegm__deps: ['__tm_struct_layout', 'mktime'],
  timegm: function(tmPtr) {
    _tzset();
    var offset = {{{ makeGetValue('_timezone', 0, 'i32') }}};
    var daylight = {{{ makeGetValue('_daylight', 0, 'i32') }}};
    daylight = (daylight == 1) ? 60 * 60 : 0;
    var ret = _mktime(tmPtr) + offset - daylight;
    return ret;
  },

  localtime__deps: ['malloc', '__tm_struct_layout', '__tm_current', 'localtime_r'],
  localtime: function(time) {
    if (!___tm_current) ___tm_current = _malloc(___tm_struct_layout.__size__);
    return _localtime_r(time, ___tm_current);
  },

  localtime_r__deps: ['__tm_struct_layout', '__tm_timezones', 'tzset'],
  localtime_r: function(time, tmPtr) {
    _tzset();
    var date = new Date({{{ makeGetValue('time', 0, 'i32') }}}*1000);
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_sec', 'date.getSeconds()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_min', 'date.getMinutes()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_hour', 'date.getHours()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_mday', 'date.getDate()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_mon', 'date.getMonth()', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_year', 'date.getFullYear()-1900', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_wday', 'date.getDay()', 'i32') }}}

    var start = new Date(date.getFullYear(), 0, 1);
    var yday = Math.floor((date.getTime() - start.getTime()) / (1000 * 60 * 60 * 24));
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_yday', 'yday', 'i32') }}}
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_gmtoff', 'start.getTimezoneOffset() * 60', 'i32') }}}

    var dst = Number(start.getTimezoneOffset() != date.getTimezoneOffset());
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_isdst', 'dst', 'i32') }}}

    var timezone = date.toString().match(/\(([A-Z]+)\)/)[1];
    if (!(timezone in ___tm_timezones)) {
      ___tm_timezones[timezone] = Pointer_make(intArrayFromString(timezone), null, ALLOC_NORMAL, 'i8');
    }
    {{{ makeSetValue('tmPtr', '___tm_struct_layout.tm_zone', '___tm_timezones[timezone]', 'i32') }}}

    return tmPtr;
  },

  asctime__deps: ['malloc', '__tm_formatted', 'asctime_r'],
  asctime: function(tmPtr) {
    if (!___tm_formatted) ___tm_formatted = _malloc(26);
    return _asctime_r(tmPtr, ___tm_formatted);
  },

  asctime_r__deps: ['__tm_formatted', 'mktime'],
  asctime_r: function(tmPtr, buf) {
    var date = new Date(_mktime(tmPtr)*1000);
    var formatted = date.toString();
    var datePart = formatted.replace(/\d{4}.*/, '').replace(/ 0/, '  ');
    var timePart = formatted.match(/\d{2}:\d{2}:\d{2}/)[0];
    formatted = datePart + timePart + ' ' + date.getFullYear() + '\n';
    formatted.split('').forEach(function(chr, index) {
      {{{ makeSetValue('buf', 'index', 'chr.charCodeAt(0)', 'i8') }}}
    });
    {{{ makeSetValue('buf', '25', '0', 'i8') }}}
    return buf;
  },

  ctime__deps: ['localtime', 'asctime'],
  ctime: function(timer) {
    return _asctime(_localtime(timer));
  },

  ctime_r__deps: ['localtime', 'asctime'],
  ctime_r: function(timer, buf) {
    return _asctime_r(_localtime_r(timer, ___tm_current), buf);
  },

  dysize: function(year) {
    var leap = ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
    return leap ? 366 : 365;
  },

  // TODO: Initialize these to defaults on startup from system settings.
  tzname: null,
  daylight: null,
  timezone: null,
  tzset__deps: ['malloc', 'tzname', 'daylight', 'timezone'],
  tzset: function() {
    // TODO: Use (malleable) environment variables instead of system settings.
    if (_tzname !== null) return;

    _timezone = _malloc(QUANTUM_SIZE);
    {{{ makeSetValue('_timezone', '0', '-(new Date()).getTimezoneOffset() * 60', 'i32') }}}

    _daylight = _malloc(QUANTUM_SIZE);
    var winter = new Date(2000, 0, 1);
    var summer = new Date(2000, 6, 1);
    {{{ makeSetValue('_daylight', '0', 'Number(winter.getTimezoneOffset() != summer.getTimezoneOffset())', 'i32') }}}

    var winterName = winter.toString().match(/\(([A-Z]+)\)/)[1];
    var summerName = summer.toString().match(/\(([A-Z]+)\)/)[1];
    var winterNamePtr = Pointer_make(intArrayFromString(winterName), null, ALLOC_NORMAL, 'i8');
    var summerNamePtr = Pointer_make(intArrayFromString(summerName), null, ALLOC_NORMAL, 'i8');
    _tzname = _malloc(2 * QUANTUM_SIZE);
    {{{ makeSetValue('_tzname', '0', 'winterNamePtr', 'i32') }}}
    {{{ makeSetValue('_tzname', QUANTUM_SIZE, 'summerNamePtr', 'i32') }}}
  },

  stime: function(when) {
    // TODO: Set errno.
    return -1;
  },

  // TODO: Implement strftime(), strptime() and getdate().

  // ==========================================================================
  // sys/time.h
  // ==========================================================================

  gettimeofday: function(ptr) {
    // %struct.timeval = type { i32, i32 }
    var indexes = Runtime.calculateStructAlignment({ fields: ['i32', 'i32'] });
    var now = Date.now();
    {{{ makeSetValue('ptr', 'indexes[0]', 'Math.floor(now/1000)', 'i32') }}} // seconds
    {{{ makeSetValue('ptr', 'indexes[1]', 'Math.floor((now-1000*Math.floor(now/1000))*1000)', 'i32') }}} // microseconds
    return 0;
  },

  // ==========================================================================
  // setjmp.h
  // ==========================================================================

  _setjmp: function(env) {
    // XXX print('WARNING: setjmp() not really implemented, will fail if longjmp() is actually called');
    return 0;
  },

  _longjmp: function(env, val) {
    // not really working...
    assert(0);
  },

  // ==========================================================================
  // signal.h
  // ==========================================================================

  signal: function(sig, func) {
    // TODO
    return 0;
  },

  __libc_current_sigrtmin: function() {
    return 0;
  },
  __libc_current_sigrtmax: function() {
    return 0;
  },

  // ==========================================================================
  // stat.h
  // ==========================================================================

  __01fstat64_: 'fstat',
  __01stat64_: 'stat',
  __01lstat64_: 'stat',

  // ==========================================================================
  // locale.h
  // ==========================================================================

  setlocale: function(category, locale) {
    return 0;
  },

  localeconv: function() {
    // %struct.timeval = type { char* decimal point, other stuff... }
    // var indexes = Runtime.calculateStructAlignment({ fields: ['i32', 'i32'] });
    var me = _localeconv;
    if (!me.ret) {
      me.ret = Pointer_make([Pointer_make(intArrayFromString('.'), null, ALLOC_NORMAL, 'i8')], null, ALLOC_NORMAL, 'i8'); // just decimal point, for now
    }
    return me.ret;
  },

  // ==========================================================================
  // langinfo.h
  // ==========================================================================

  nl_langinfo: function(item) {
    var me = _nl_langinfo;
    if (!me.ret) {
      me.ret = Pointer_make(intArrayFromString("eh?"), null, ALLOC_NORMAL, 'i8'); 
    }
    return me.ret;
  },

  // ==========================================================================
  // errno.h
  // ==========================================================================

  $ERRNO_CODES: {
    E2BIG: 7,
    EACCES: 13,
    EADDRINUSE: 98,
    EADDRNOTAVAIL: 99,
    EAFNOSUPPORT: 97,
    EAGAIN: 11,
    EALREADY: 114,
    EBADF: 9,
    EBADMSG: 74,
    EBUSY: 16,
    ECANCELED: 125,
    ECHILD: 10,
    ECONNABORTED: 103,
    ECONNREFUSED: 111,
    ECONNRESET: 104,
    EDEADLK: 35,
    EDESTADDRREQ: 89,
    EDOM: 33,
    EDQUOT: 122,
    EEXIST: 17,
    EFAULT: 14,
    EFBIG: 27,
    EHOSTUNREACH: 113,
    EIDRM: 43,
    EILSEQ: 84,
    EINPROGRESS: 115,
    EINTR: 4,
    EINVAL: 22,
    EIO: 5,
    EISCONN: 106,
    EISDIR: 21,
    ELOOP: 40,
    EMFILE: 24,
    EMLINK: 31,
    EMSGSIZE: 90,
    EMULTIHOP: 72,
    ENAMETOOLONG: 36,
    ENETDOWN: 100,
    ENETRESET: 102,
    ENETUNREACH: 101,
    ENFILE: 23,
    ENOBUFS: 105,
    ENODATA: 61,
    ENODEV: 19,
    ENOENT: 2,
    ENOEXEC: 8,
    ENOLCK: 37,
    ENOLINK: 67,
    ENOMEM: 12,
    ENOMSG: 42,
    ENOPROTOOPT: 92,
    ENOSPC: 28,
    ENOSR: 63,
    ENOSTR: 60,
    ENOSYS: 38,
    ENOTCONN: 107,
    ENOTDIR: 20,
    ENOTEMPTY: 39,
    ENOTRECOVERABLE: 131,
    ENOTSOCK: 88,
    ENOTSUP: 95,
    ENOTTY: 25,
    ENXIO: 6,
    EOVERFLOW: 75,
    EOWNERDEAD: 130,
    EPERM: 1,
    EPIPE: 32,
    EPROTO: 71,
    EPROTONOSUPPORT: 93,
    EPROTOTYPE: 91,
    ERANGE: 34,
    EROFS: 30,
    ESPIPE: 29,
    ESRCH: 3,
    ESTALE: 116,
    ETIME: 62,
    ETIMEDOUT: 110,
    ETXTBSY: 26,
    EWOULDBLOCK: 11,
    EXDEV: 18,
  },
  $ERRNO_MESSAGES: {
    2: 'No such file or directory',
    13: 'Permission denied',
    98: 'Address already in use',
    99: 'Cannot assign requested address',
    97: 'Address family not supported by protocol',
    11: 'Resource temporarily unavailable',
    114: 'Operation already in progress',
    9: 'Bad file descriptor',
    74: 'Bad message',
    16: 'Device or resource busy',
    125: 'Operation canceled',
    10: 'No child processes',
    103: 'Software caused connection abort',
    111: 'Connection refused',
    104: 'Connection reset by peer',
    35: 'Resource deadlock avoided',
    89: 'Destination address required',
    33: 'Numerical argument out of domain',
    122: 'Disk quota exceeded',
    17: 'File exists',
    14: 'Bad address',
    27: 'File too large',
    113: 'No route to host',
    43: 'Identifier removed',
    84: 'Invalid or incomplete multibyte or wide character',
    115: 'Operation now in progress',
    4: 'Interrupted system call',
    22: 'Invalid argument',
    5: 'Input/output error',
    106: 'Transport endpoint is already connected',
    21: 'Is a directory',
    40: 'Too many levels of symbolic links',
    24: 'Too many open files',
    31: 'Too many links',
    90: 'Message too long',
    72: 'Multihop attempted',
    36: 'File name too long',
    100: 'Network is down',
    102: 'Network dropped connection on reset',
    101: 'Network is unreachable',
    23: 'Too many open files in system',
    105: 'No buffer space available',
    61: 'No data available',
    19: 'No such device',
    2: 'No such file or directory',
    8: 'Exec format error',
    37: 'No locks available',
    67: 'Link has been severed',
    12: 'Cannot allocate memory',
    42: 'No message of desired type',
    92: 'Protocol not available',
    28: 'No space left on device',
    63: 'Out of streams resources',
    60: 'Device not a stream',
    38: 'Function not implemented',
    107: 'Transport endpoint is not connected',
    20: 'Not a directory',
    39: 'Directory not empty',
    131: 'State not recoverable',
    88: 'Socket operation on non-socket',
    95: 'Operation not supported',
    25: 'Inappropriate ioctl for device',
    6: 'No such device or address',
    75: 'Value too large for defined data type',
    130: 'Owner died',
    1: 'Operation not permitted',
    32: 'Broken pipe',
    71: 'Protocol error',
    93: 'Protocol not supported',
    91: 'Protocol wrong type for socket',
    34: 'Numerical result out of range',
    30: 'Read-only file system',
    29: 'Illegal seek',
    3: 'No such process',
    116: 'Stale NFS file handle',
    62: 'Timer expired',
    110: 'Connection timed out',
    26: 'Text file busy',
    11: 'Resource temporarily unavailable',
    18: 'Invalid cross-device link'
  },
  __setErrNo: function(value) {
    // For convenient setting and returning of errno.
    var me = ___setErrNo;
    if (!me.ptr) me.ptr = Pointer_make([0], 0, ALLOC_STATIC, 'i32');
    {{{ makeSetValue('me.ptr', '0', 'value', 'i32') }}}
    return value;
  },
  __errno_location__deps: ['__setErrNo'],
  __errno_location: function() {
    if (!___setErrNo.ptr) ___setErrNo(0);
    return ___setErrNo.ptr;
  },

  // ==========================================================================
  // pthread.h (stubs for mutexes only - no thread support yet!)
  // ==========================================================================

  pthread_mutex_init: function() {},
  pthread_mutex_destroy: function() {},
  pthread_mutex_lock: function() {},
  pthread_mutex_unlock: function() {},

  // ==========================================================================
  // malloc.h
  // ==========================================================================

  memalign: function(boundary, size) {
    // leaks, and even returns an invalid pointer. Horrible hack... but then, this is a deprecated function...
    var ret = Runtime.staticAlloc(size + boundary);
    return ret + boundary - (ret % boundary);
  },

  posix_memalign__deps: ['memalign'],
  posix_memalign: function(memptr, alignment, size) {
    var ptr = _memalign(alignment, size);
    {{{ makeSetValue('memptr', '0', 'ptr', 'i8*') }}}
    return 0;
  },

  // ==========================================================================
  // dirent.h
  // ==========================================================================

  opendir: function(pname) {
    return 0;
  },

  // ==========================================================================
  // ** emscripten.h **
  // ==========================================================================
  emscripten_run_script: function(ptr) {
    eval(Pointer_stringify(ptr));
  },

  _Z21emscripten_run_scriptPKc: function(ptr) {
    eval(Pointer_stringify(ptr));
  }
};


// 'Runtime' functions that need preprocessor parsing like library functions

// Converts a value we have as signed, into an unsigned value. For
// example, -1 in int32 would be a very large number as unsigned.
function unSign(value, bits, ignore, sig) {
  if (value >= 0) {
#if CHECK_SIGNS
    if (!ignore) CorrectionsMonitor.note('UnSign', 1, sig);
#endif
    return value;
  }
#if CHECK_SIGNS
  if (!ignore) CorrectionsMonitor.note('UnSign', 0, sig);
#endif
  return bits <= 32 ? 2*Math.abs(1 << (bits-1)) + value // Need some trickery, since if bits == 32, we are right at the limit of the bits JS uses in bitshifts
                    : Math.pow(2, bits)         + value;
  // TODO: clean up previous line
}

// Converts a value we have as unsigned, into a signed value. For
// example, 200 in a uint8 would be a negative number.
function reSign(value, bits, ignore, sig) {
  if (value <= 0) {
#if CHECK_SIGNS
    if (!ignore) CorrectionsMonitor.note('ReSign', 1, sig);
#endif
    return value;
  }
  var half = bits <= 32 ? Math.abs(1 << (bits-1)) // abs is needed if bits == 32
                        : Math.pow(2, bits-1);
#if CHECK_SIGNS
  var noted = false;
#endif
  if (value >= half) {
#if CHECK_SIGNS
    if (!ignore) {
      CorrectionsMonitor.note('ReSign', 0, sig);
      noted = true;
    }
#endif
    value = -2*half + value; // Cannot bitshift half, as it may be at the limit of the bits JS uses in bitshifts
  }
#if CHECK_SIGNS
  // If this is a 32-bit value, then it should be corrected at this point. And,
  // without CHECK_SIGNS, we would just do the |0 shortcut, so check that that
  // would indeed give the exact same result.
  if (bits === 32 && (value|0) !== value && typeof value !== 'boolean') {
    if (!ignore) {
      CorrectionsMonitor.note('ReSign', 0, sig);
      noted = true;
    }
  }
  if (!noted) CorrectionsMonitor.note('ReSign', 1, sig);
#endif
  return value;
}

// Just a stub. We don't care about noting compile-time corrections. But they are called.
var CorrectionsMonitor = {
  note: function(){}
};

