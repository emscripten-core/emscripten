#!/usr/bin/env python2

import os

def generate(env, emscripten_path=None, **kw):
	""" SCons tool entry point """

	if emscripten_path is None:
		# Try to find emscripten
		# Use same method as Emscripten's shared.py
		EM_CONFIG = os.environ.get('EM_CONFIG')
		if not EM_CONFIG:
			EM_CONFIG = '~/.emscripten'
		
		CONFIG_FILE = os.path.expanduser(EM_CONFIG)
		try:
			exec(open(CONFIG_FILE, 'r').read())
		except Exception, e:
  			print >> sys.stderr, 'Error in evaluating %s (at %s): %s' % (EM_CONFIG, CONFIG_FILE, str(e))
  			sys.exit(1)
  		
  		emscripten_path = EMSCRIPTEN_ROOT

	env['EMSCRIPTEN_ROOT'] = emscripten_path
	
	try:
		emscPath = emscripten_path.abspath
	except:
		emscPath = emscripten_path
	
	env.Replace(CC     = os.path.join(emscPath, "emcc"    ))
	env.Replace(CXX    = os.path.join(emscPath, "em++"    ))
	env.Replace(LINK   = os.path.join(emscPath, "emcc"    ))
	# SHLINK and LDMODULE should use LINK so no
	# need to change them here
    
	env.Replace(AR     = os.path.join(emscPath, "emar"    ))
	env.Replace(RANLIB = os.path.join(emscPath, "emranlib"))

 	env.Replace(OBJSUFFIX  = [".js", ".bc", ".o"][2])
 	env.Replace(LIBSUFFIX  = [".js", ".bc", ".o"][2])
 	env.Replace(PROGSUFFIX = [".html", ".js"    ][1])

def exists(env):
	""" NOOP method required by SCons """
	return 1
