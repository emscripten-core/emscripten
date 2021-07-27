if (!LibraryManager.library.glTexImage2D) throw 'This file should be getting processed after library_webgl.js!';

mergeInto(LibraryManager.library, {
	orig_glTexImage2D__deps: LibraryManager.library.glTexImage2D__deps,
	orig_glTexImage2D: LibraryManager.library.glTexImage2D,

	glTexImage2D__deps: ['orig_glTexImage2D'],
	glTexImage2D: function(target, level, internalFormat, width, height, border, format, type, pixels) {
		_glTexImage2D.createdType = type;
		// Check that the orignal fuction exists
		assert(_orig_glTexImage2D);
		// Also try invoking glTexImage2D to verify that it is actually the
		// underlying functions from library_webgl.js
		var texImage2D_called = false;
		GLctx = {
			texImage2D: function() {
				texImage2D_called = true;
			},
		};
		_orig_glTexImage2D();
		assert(texImage2D_called);
	},

	what_got_created: function() {
		return _glTexImage2D.createdType;
	}
});
