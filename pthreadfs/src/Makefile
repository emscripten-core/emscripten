# Define some folders
JSLIB_SRC = js

LIB_WRAPPER = $(JSLIB_SRC)/pthreadfs.js
LIB_SYS = $(JSLIB_SRC)/library_syscall_async.js
LIB_FS = $(JSLIB_SRC)/library_asyncfs.js
LIB_MEMFS = $(JSLIB_SRC)/library_memfs_async.js
LIB_SFAFS = $(JSLIB_SRC)/library_sfafs_async.js
LIB_FSAFS = $(JSLIB_SRC)/library_fsafs.js
LIB_TTY = $(JSLIB_SRC)/library_tty_async.js 

library_pthreadfs.js:  $(LIB_WRAPPER) $(LIB_SYS) $(LIB_FS) $(LIB_MEMFS) $(LIB_FSAFS) $(LIB_TTY) 
	rm -f $@
	cat $? > ../$@
