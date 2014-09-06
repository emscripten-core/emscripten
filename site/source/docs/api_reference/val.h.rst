.. _val-h:

================================
val.h (under-construction)
================================

The C++ APIs in `val.h <https://github.com/kripken/emscripten/blob/master/system/include/emscripten/val.h>`_ define (**HamishW**-Replace with description.)

.. contents:: Table of Contents
    :local:
    :depth: 1
	
.. COMMENT (Not rendered) : This created from val.h header file on 10 Aug 2014-03


.. cpp:type: EMSCRIPTEN_SYMBOL(name)

	**HamishW**-Replace with description.



.. cpp:namespace:: emscripten
	
.. cpp:class:: val

	**HamishW** Notes from source FYI: ::

        // missing operators:
        // * delete
        // * in
        // * instanceof
        // * ! ~ - + ++ --
        // * * / %
        // * + -
        // * << >> >>>
        // * < <= > >=
        // * == != === !==
        // * & ^ | && || ?:
        //
        // exposing void, comma, and conditional is unnecessary
        // same with: = += -= *= /= %= <<= >>= >>>= &= ^= |=


	.. cpp:function:: static val array()

		**HamishW**-Replace with description.
		
		:returns: **HamishW**-Replace with description.

		
	.. cpp:function:: static val object()

		**HamishW**-Replace with description.
		
		:returns: **HamishW**-Replace with description.

		
	.. cpp:function:: static val undefined()

		**HamishW**-Replace with description.
		
		:returns: **HamishW**-Replace with description.		
	
	
	.. cpp:function:: static val null()

		**HamishW**-Replace with description.
		
		:returns: **HamishW**-Replace with description.		

		
	.. cpp:function:: static val take_ownership(internal::EM_VAL e)

		**HamishW**-Replace with description.
		
		:returns: **HamishW**-Replace with description.		


	.. cpp:function:: static val global(const char* name)

		**HamishW**-Replace with description.
		
		:param const char* name: **HamishW**-Replace with description.
		:returns: **HamishW**-Replace with description.				
		


	.. cpp:function:: static val module_property(const char* name)

		**HamishW**-Replace with description.
		
		:param const char* name: **HamishW**-Replace with description.
		:returns: **HamishW**-Replace with description.				

		
	.. cpp:function:: static val module_property(const char* name)

		**HamishW**-Replace with description.
		
		:param const char* name: **HamishW**-Replace with description.
		:returns: **HamishW**-Replace with description.	

	.. cpp:function:: explicit val(T&& value)

		**HamishW**-Replace with description.
		
		:param T&& value: **HamishW**-Replace with description.
		
		
		**HamishW** Don't know how following "floating statement works". Leaving here for discussion
        val() = delete;

	.. cpp:function:: explicit val(const char* v)

		**HamishW**-Replace with description.
		
		:param const char* v: **HamishW**-Replace with description.		
		

	.. cpp:function:: val(val&& v)

		**HamishW**-Replace with description.
		
		:param val&& v: **HamishW**-Replace with description.	
		

	.. cpp:function:: val(const val& v)

		**HamishW**-Replace with description.
		
		:param const val& v: **HamishW**-Replace with description.	
        

	.. cpp:function:: ~val()

		Destructor. **HamishW**-Replace with further description or delete comment.

		
	.. cpp:function:: val(const val& v)

		**HamishW**-Replace with description.
		
		:param const val& v: **HamishW**-Replace with description.			
		

	.. cpp:function:: val& operator=(val&& v)

		**HamishW**-Replace with description.
		
		:param val&& v: **HamishW**-Replace with description.
		:returns: **HamishW**-Replace with description.


	.. cpp:function:: val& operator=(const val& v)

		**HamishW**-Replace with description.
		
		:param val&& v: **HamishW**-Replace with description.
		:returns: **HamishW**-Replace with description.		

		
	.. cpp:function:: bool hasOwnProperty(const char* key) const

		**HamishW**-Replace with description.
		
		:param const char* key: **HamishW**-Replace with description.
		:returns: **HamishW**-Replace with description.				

		
	.. cpp:function:: val new_(Args&&... args) const

		**HamishW**-Replace with description.
		
		:param Args&&... args: **HamishW**-Replace with description. Note that this is a templated value.
		:returns: **HamishW**-Replace with description.				



	.. cpp:function:: val operator[](const T& key) const

		**HamishW**-Replace with description.
		
		:param const T& key: **HamishW**-Replace with description. Note that this is a templated value.
		:returns: **HamishW**-Replace with description.		


	.. cpp:function:: void set(const K& key, const val& v)

		**HamishW**-Replace with description.
		
		:param const K& key: **HamishW**-Replace with description. Note that this is a templated value.
		:param const val& v: **HamishW**-Replace with description.	 Note that this is a templated value.
		

	.. cpp:function:: val operator()(Args&&... args)

		**HamishW**-Replace with description.
		
		:param Args&&... args: **HamishW**-Replace with description. Note that this is a templated value.	
		

	.. cpp:function:: ReturnValue call(const char* name, Args&&... args) const

		**HamishW**-Replace with description.
		
		:param const char* name: **HamishW**-Replace with description. 
		:param Args&&... args: **HamishW**-Replace with description. Note that this is a templated value.			

		
	.. cpp:function:: T as() const

		**HamishW**-Replace with description.
		
		:returns: **HamishW**-Replace with description. Note that this is a templated value.		


	.. cpp:function:: val typeof() const

		**HamishW**-Replace with description.
		
		:returns: **HamishW**-Replace with description. 	
		
		
	.. cpp:function:: std::vector<T> vecFromJSArray(val v)

		**HamishW**-Replace with description.
		
		**HamishW**. I believe NOT internal. Please confirm.
		
		:param val v: **HamishW**-Replace with description. 
		:returns: **HamishW**-Replace with description. 	
		
		
		
