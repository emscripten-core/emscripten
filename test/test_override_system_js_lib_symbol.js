if (!LibraryManager.library.glTexImage3D) throw 'This file should be getting processed after library_webgl2.js!';

addToLibrary({
	orig_glTexImage3D__deps: LibraryManager.library.glTexImage3D__deps,
	orig_glTexImage3D: LibraryManager.library.glTexImage3D,

	glTexImage3D__deps: ['orig_glTexImage3D'],
	glTexImage3D: function(target, level, internalFormat, width, height, depth, border, format, type, pixels) {
		_glTexImage3D.createdType = type;
		// Check that the original fuction exists
		assert(_orig_glTexImage3D);
		// Also try invoking glTexImage3D to verify that it is actually the
		// underlying function from library_webgl2.js
		var texImage3D_called = false;
		// Mock GL context to be able to call from shell.
		GLctx = {
			texImage3D: function() {
				texImage3D_called = true;
			},
		};
		_orig_glTexImage3D();
		assert(texImage3D_called);
	},

	what_got_created: function() {
		return _glTexImage3D.createdType;
	}
});
