addToLibrary({
	js_function: function() {
#if CUSTOM_JS_OPTION
		return 1;
#else
		return 0;
#endif
	}
});
