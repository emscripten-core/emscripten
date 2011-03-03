'''
Processes an LLVM assembly (.ll) file, adding debugging information.

You can then run the .ll file in the LLVM interpreter (lli) and 
compare that to the output when compiled using emscripten.
'''

import os, sys, re

POSTAMBLE = '''
@.emscripten.autodebug.str = private constant [9 x i8] c"%d : %d\\0A\\00", align 1 ; [#uses=1]

; [#uses=1]
define void @emscripten_autodebug(i32 %line, i32 %value) {
entry:
  %0 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([9 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %value) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
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

lines = data.split('\n')
for i in range(len(lines)):
  #if i == 5:
  #  lines[i] += '\n

  m = re.match('  store (?P<type>i64|i32|i16|i8) %(?P<var>[\w.]+), .*', lines[i])
  if m and m.group('type') == 'i32': # TODO: Other types
    lines[i] += '\n  call void @emscripten_autodebug(i32 %d, i32 %%%s)' % (i+1, m.group('var'))

f = open(ofilename, 'w')
f.write('\n'.join(lines) + '\n' + POSTAMBLE + '\n')
f.close()

print 'Success.'

