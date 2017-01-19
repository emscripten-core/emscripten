
function _objc_getClass(classNamePtr)
{
	var className = Pointer_stringify(classNamePtr);
	console.log("objc_getClass('" + className + "');");
	return className; 
	
	// this should return a pointer to _OBJC_CLASS_<classname>.
	// it should NOT return the class name, because C code may
	// refer to individual members of the C structure defining
	// the class.

	// unfortunately, it looks like class definitions are put in 
	// a different section of the executable, "__OBJC, __class".
	// similarly, 'static' methods (marked with +) are stored in
	// "__OBJC, __meta_class".
	
	// while present in the LLVM bitcode, they do not seem to be
	// present in the Javascript output by Emscripten.
}
function _sel_registerName(selectorNamePtr)
{
	var selectorName = Pointer_stringify(selectorNamePtr);
	console.log("sel_registerName('" + selectorName + "');");
	return selectorName;

	// with Apple ABI, this should return a unique C string containing
	// the name of the selector. that is, we should have a dictionary with
	// string values as keys, and C string pointers as values.
}
function _objc_msgSend(objectPtr, selector)
{
	console.log("objc_msgSend(/*object: */ '" + objectPtr + "', /*selector: */ '" + selector + "');");
}
