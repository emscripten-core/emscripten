mergeInto(LibraryManager.library, {
	foo: function() {
		thisVarDoesNotExist++;
		thisVarDoesNotExistEither++;
	}
});
