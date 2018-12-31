'''
Processes an LLVM assembly (.ll) file, adding debugging information.

You can then run the .ll file in the LLVM interpreter (lli) and 
compare that to the output when compiled using emscripten.

Warning: You probably want to compile with SKIP_STACK_IN_SMALL=0! Otherwise
         there may be weird errors.
'''

import os, sys, re

ALLOW_POINTERS = True
ALLOW_MISC = True
MEMCPY = False
MEMCPY2 = False
NO_DLMALLOC = False
JS_LIB_PRINTING = True

POSTAMBLE = '''
@.emscripten.autodebug.str = private constant [10 x i8] c"AD:%d,%d\\0A\\00", align 1 ; [#uses=1]
@.emscripten.autodebug.str.f = private constant [11 x i8] c"AD:%d,%lf\\0A\\00", align 1 ; [#uses=1]
@.emscripten.autodebug.str.64 = private constant [13 x i8] c"AD:%d,%d,%d\\0A\\00", align 1 ; [#uses=1]
'''

if JS_LIB_PRINTING:
  POSTAMBLE += '''
; [#uses=1]
declare void @emscripten_autodebug_i64(i32 %line, i64 %value)

; [#uses=1]
declare void @emscripten_autodebug_i32(i32 %line, i32 %value)

; [#uses=1]
declare void @emscripten_autodebug_i16(i32 %line, i16 %value)

; [#uses=1]
declare void @emscripten_autodebug_i8(i32 %line, i8 %value)

; [#uses=1]
declare void @emscripten_autodebug_float(i32 %line, float %value)

; [#uses=1]
declare void @emscripten_autodebug_double(i32 %line, double %value)
'''
else:
  POSTAMBLE += '''
; [#uses=1]
define void @emscripten_autodebug_i64(i32 %line, i64 %value) {
entry:
  %0 = trunc i64 %value to i32
  %1 = lshr i64 %value, 32
  %2 = trunc i64 %1 to i32
  %3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.emscripten.autodebug.str.64, i32 0, i32 0), i32 %line, i32 %0, i32 %2) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_i32(i32 %line, i32 %value) {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %value) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_i16(i32 %line, i16 %value) {
entry:
  %0 = zext i16 %value to i32 ; [#uses=1]
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %0) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_i8(i32 %line, i8 %value) {
entry:
  %0 = zext i8 %value to i32 ; [#uses=1]
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %0) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_float(i32 %line, float %value) {
entry:
  %0 = fpext float %value to double ; [#uses=1]
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.emscripten.autodebug.str.f, i32 0, i32 0), i32 %line, double %0) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_double(i32 %line, double %value) {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.emscripten.autodebug.str.f, i32 0, i32 0), i32 %line, double %value) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}
'''

filename, ofilename = sys.argv[1], sys.argv[2]
f = open(filename, 'r')
data = f.read()
f.close()

if 'declare i32 @printf(' not in data and 'define internal i32 @printf(' not in data:
  POSTAMBLE += '''
; [#uses=1]
declare i32 @printf(i8*, ...)
'''

if MEMCPY:
  POSTAMBLE = '''
@.emscripten.memcpy.str = private constant [7 x i8] c"MC:%d\\0A\\00", align 1 ; [#uses=1]

''' + POSTAMBLE + '''
; [#uses=1]
define void @emscripten_memcpy(i8* %destination, i8* %source, i32 %num, i32 %whati, i1 %sthis) nounwind {
entry:
  %destination.addr = alloca i8*, align 4         ; [#uses=3]
  %source.addr = alloca i8*, align 4              ; [#uses=2]
  %num.addr = alloca i32, align 4                 ; [#uses=3]
  %i = alloca i32, align 4                        ; [#uses=5]
  %src = alloca i8*, align 4                      ; [#uses=5]
  %dst = alloca i8*, align 4                      ; [#uses=4]
  store i8* %destination, i8** %destination.addr, align 4
  store i8* %source, i8** %source.addr, align 4
  store i32 %num, i32* %num.addr, align 4
  %tmp = load i8** %source.addr, align 4 ; [#uses=1]
  store i8* %tmp, i8** %src, align 4
  %tmp2 = load i8** %destination.addr, align 4 ; [#uses=1]
  store i8* %tmp2, i8** %dst, align 4
  store i32 0, i32* %i, align 4
  %tmp31 = load i32* %i, align 4       ; [#uses=1]
  %tmp42 = load i32* %num.addr, align 4 ; [#uses=1]
  %cmp3 = icmp ult i32 %tmp31, %tmp42   ; [#uses=1]
  br i1 %cmp3, label %for.body, label %for.end

for.body:                                         ; preds = %for.body, %entry
  %tmp5 = load i8** %src, align 4       ; [#uses=1]
  %tmp6 = load i8* %tmp5                ; [#uses=1]
  %conv = zext i8 %tmp6 to i32         ; [#uses=1]
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8]* @.emscripten.memcpy.str, i32 0, i32 0), i32 %conv); [#uses=0]
  %tmp7 = load i8** %src, align 4     ; [#uses=1]
  %tmp8 = load i8* %tmp7                ; [#uses=1]
  %tmp9 = load i8** %dst, align 4      ; [#uses=1]
  store i8 %tmp8, i8* %tmp9
  %tmp10 = load i32* %i, align 4       ; [#uses=1]
  %inc = add i32 %tmp10, 1              ; [#uses=1]
  store i32 %inc, i32* %i, align 4
  %tmp11 = load i8** %src, align 4     ; [#uses=1]
  %incdec.ptr = getelementptr inbounds i8* %tmp11, i32 1 ; [#uses=1]
  store i8* %incdec.ptr, i8** %src, align 4
  %tmp12 = load i8** %dst, align 4      ; [#uses=1]
  %incdec.ptr13 = getelementptr inbounds i8* %tmp12, i32 1 ; [#uses=1]
  store i8* %incdec.ptr13, i8** %dst, align 4
  %tmp3 = load i32* %i, align 4         ; [#uses=1]
  %tmp4 = load i32* %num.addr, align 4  ; [#uses=1]
  %cmp = icmp ult i32 %tmp3, %tmp4      ; [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %entry
  %tmp14 = load i8** %destination.addr, align 4 ; [#uses=1]
  ret void
}
'''

lines_added = 0
lines = data.split('\n')
in_func = False
added_entry = False
for i in range(len(lines)):
  if MEMCPY:
    if not lines[i].startswith('declare void'):
      lines[i] = lines[i].replace('@llvm.memcpy.p0i8.p0i8.i32', '@emscripten_memcpy')

  try:
    pre = ''
    if lines[i].startswith('define '):
      in_func = True
      if NO_DLMALLOC and ('@malloc(' in lines[i] or '@free(' in lines[i] or '@sys_alloc(' in lines[i] or '@segment_holding(' in lines[i] or '@init_top(' in lines[i] or '@add_segment(' in lines[i] or '@tmalloc_small(' in lines[i]):
        in_func = False
      if in_func:
        added_entry = False
      if 'printf' in lines[i] or '__fwritex' in lines[i] or '__towrite' in lines[i] or 'pop_arg391' in lines[i] or 'fmt_u' in lines[i] or 'pad(' in lines[i] or 'stdout_write' in lines[i] or 'stdio_write' in lines[i] or 'syscall' in lines[i]:
        if not JS_LIB_PRINTING:
          in_func = False # do not add logging in musl printing code, which would infinitely recurse
    elif lines[i].startswith('}'):
      in_func = False
    elif in_func and not added_entry and ' = alloca' not in lines[i] and lines[i].startswith('  '):
      # This is a good place to mark entry to this function
      added_entry = True
      index = i+1+lines_added
      pre = '  call void @emscripten_autodebug_i32(i32 -1, i32 %d)' % index
    elif in_func and lines[i].startswith('  ret '):
      # This is a good place to mark entry to this function
      index = i+1+lines_added
      pre = '  call void @emscripten_autodebug_i32(i32 -2, i32 %d)' % index

    if in_func:
      m = re.match('  store (?P<type>i64|i32|i16|i8|float|double|%?[\w\.\*]+) (?P<var>%?[\w.+_]+), .*', lines[i])
      if m:
        index = i+1+lines_added
        if m.group('type') in ['i8', 'i16', 'i32', 'i64', 'float', 'double']:
          lines[i] += '\n  call void @emscripten_autodebug_%s(i32 %d, %s %s)' % (m.group('type'), index, m.group('type'), m.group('var'))
          lines_added += 1
        elif ALLOW_POINTERS and m.group('type').endswith('*') and m.group('type').count('*') == 1:
          lines[i] += '\n  %%ead.%d = ptrtoint %s %s to i32' % (index, m.group('type'), m.group('var'))
          lines[i] += '\n  call void @emscripten_autodebug_i32(i32 %d, i32 %%ead.%d)' % (index, index)
          lines_added += 2
        continue
      m = re.match('  %(?P<var>[\w_.]+) = load (?P<type>i64|i32|i16|i8|float|double+)\* [^(].*.*', lines[i])
      if m:
        index = i+1+lines_added
        lines[i] += '\n  call void @emscripten_autodebug_%s(i32 %d, %s %%%s)' % (m.group('type'), index, m.group('type'), m.group('var'))
        lines_added += 1
        continue
      if ALLOW_MISC:
        # call is risky - return values can be i32 (i8*) (i16)
        m = re.match('  %(?P<var>[\w_.]+) = (mul|add) (nsw )?(?P<type>i64|i32|i16|i8|float|double+) .*', lines[i])
        if m:
          index = i+1+lines_added
          lines[i] += '\n  call void @emscripten_autodebug_%s(i32 %d, %s %%%s)' % (m.group('type'), index, m.group('type'), m.group('var'))
          lines_added += 1
          continue
        if MEMCPY2:
          m = re.match('  call void @llvm\.memcpy\.p0i8\.p0i8\.i32\(i8\* %(?P<dst>[\w_.]+), i8\* %(?P<src>[\w_.]+), i32 8, i32 (?P<align>\d+),.*', lines[i])
          if m:
            index = i+1+lines_added
            lines[i] += '\n  %%adtemp%d = load i8* %%%s, align 1' % (index, m.group('src')) + \
                        '\n  call void @emscripten_autodebug_i8(i32 %d, i8 %%adtemp%d)' % (index, index)
            lines_added += 3
            continue
      m = re.match('[^ ].*; preds = ', lines[i])
      if m:
        # basic block
        if len(lines) > i+1 and 'phi' not in lines[i+1] and 'landingpad' not in lines[i+1]:
          lines[i] += '\n  call void @emscripten_autodebug_i32(i32 -10, i32 %d)' % (i+1+lines_added,)
          lines_added += 1
          continue

  finally:
    if len(pre) > 0:
      lines[i] = pre + '\n' + lines[i]
      lines_added += 1

f = open(ofilename, 'w')
ll = '\n'.join(lines)
meta_start = ll.find('\n!')
f.write(ll[:meta_start] + '\n' + POSTAMBLE + '\n' + ll[meta_start:])
f.close()

print 'Success.'

