'''
Processes an LLVM assembly (.ll) file, adding debugging information.

You can then run the .ll file in the LLVM interpreter (lli) and 
compare that to the output when compiled using emscripten.
'''

import os, sys, re

ALLOW_POINTERS = False

POSTAMBLE = '''
@.emscripten.autodebug.str = private constant [10 x i8] c"AD:%d,%d\\0A\\00", align 1 ; [#uses=1]

; [#uses=1]
define void @emscripten_autodebug_i64(i32 %line, i64 %value) {
entry:
  %0 = sitofp i64 %value to double ; [#uses=1]
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, double %0) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_i32(i32 %line, i32 %value) {
entry:
  %0 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %value) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_i16(i32 %line, i16 %value) {
entry:
  %0 = zext i16 %value to i32 ; [#uses=1]
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %0) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_i8(i32 %line, i8 %value) {
entry:
  %0 = zext i8 %value to i32 ; [#uses=1]
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %0) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_float(i32 %line, float %value) {
entry:
  %0 = fpext float %value to double ; [#uses=1]
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, double %0) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_double(i32 %line, double %value) {
entry:
  %0 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, double %value) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  ret void
}
'''

POSTAMBLE_NEW = '''
@.emscripten.autodebug.str = private constant [10 x i8] c"AD:%d,%d\\0A\\00", align 1 ; [#uses=1]

; [#uses=1]
define void @emscripten_autodebug_i64(i32 %line, i64 %value) {
  %1 = sitofp i64 %value to double
  %2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, double %1) ; [#uses=0]
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_i32(i32 %line, i32 %value) {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %value) ; [#uses=0]
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_i16(i32 %line, i16 %value) {
  %1 = zext i16 %value to i32 ; [#uses=1]
  %2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %1) ; [#uses=0]
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_i8(i32 %line, i8 %value) {
  %1 = zext i8 %value to i32 ; [#uses=1]
  %2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %1) ; [#uses=0]
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_float(i32 %line, float %value) {
  %1 = fpext float %value to double ; [#uses=1]
  %2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, double %1) ; [#uses=0]
  ret void
}

; [#uses=1]
define void @emscripten_autodebug_double(i32 %line, double %value) {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, double %value) ; [#uses=0]
  ret void
}
'''

filename, ofilename = sys.argv[1], sys.argv[2]
f = open(filename, 'r')
data = f.read()
f.close()

if 'declare i32 @printf(' not in data:
  POSTAMBLE += '''
; [#uses=1]
declare i32 @printf(i8*, ...)
'''

LLVM_STYLE_OLD = '<label>' not in data and 'entry:' in data

if not LLVM_STYLE_OLD:
  POSTAMBLE = POSTAMBLE_NEW

lines_added = 0
lines = data.split('\n')
for i in range(len(lines)):
  m = re.match('  store (?P<type>i64|i32|i16|i8|float|double|%?[\w\.\*]+) %(?P<var>[\w.]+), .*', lines[i])
  if m:
    index = i+1+lines_added
    if m.group('type') in ['i8', 'i16', 'i32', 'i64', 'float', 'double']:
      lines[i] += '\n  call void @emscripten_autodebug_%s(i32 %d, %s %%%s)' % (m.group('type'), index, m.group('type'), m.group('var'))
      lines_added += 1
    elif ALLOW_POINTERS and m.group('type').endswith('*') and m.group('type').count('*') == 1:
      lines[i] += '\n  %%ead.%d = ptrtoint %s %%%s to i32' % (index, m.group('type'), m.group('var'))
      lines[i] += '\n  call void @emscripten_autodebug_i32(i32 %d, i32 %%ead.%d)' % (index, index)
      lines_added += 2

f = open(ofilename, 'w')
f.write('\n'.join(lines) + '\n' + POSTAMBLE + '\n')
f.close()

print 'Success.'

