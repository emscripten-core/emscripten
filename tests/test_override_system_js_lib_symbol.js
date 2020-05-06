if (!LibraryManager.library.glTexImage2D) throw 'This file should be getting processed after library_webgl.js!';

mergeInto(LibraryManager.library, {
	orig_glTexImage2D__deps: LibraryManager.library.glTexImage2D__deps,
	orig_glTexImage2D: LibraryManager.library.glTexImage2D,

	glTexImage2D__deps: ['orig_glTexImage2D'],
	glTexImage2D: function(target, level, internalFormat, width, height, border, format, type, pixels) {
		_glTexImage2D.createdType = type;
		_orig_glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
	},

	what_got_created: function() {
		return _glTexImage2D.createdType;
	}
});
