// XXX FIXME Hardcoded '4' in many places, here and in library_SDL, for RGBA

function ctx()
{
	return wglGetCurrentContext();
}

function prg(progi)
{
	return GL.programs[progi];
}

function getPrg(program)
{
	
}

function tex(texi)
{
	return GL.textures[texi];
}

function getTex(texture)
{
}

function buf(bufi)
{
	return GL.buffers[bufi];
}

function getBuf(buffer)
{

}

function shd(shadi)
{
	return GL.shaders[shadi];
}

function getShd(shader)
{

}

function typ(t)
{
	var type = typeof t;
	
	if( t === null )
		return GL.TYPE_NULL;
	
	if( t == "object" )
	{
		if( t.length === undefined )
			return GL.TYPE_OBJECT;
		else
			return GL.TYPE_ARRAY;
	}
	else if( t == "number" )
	{
		if( isNaN(t) )
			return GL.TYPE_NAN;
		else
			return GL.TYPE_NUMBER;
	}
	else
		return GL.TYPE_UNDEFINED;
}

// \0 is NOT counted!
function allocateString(str, maxlen)
{
	var len;
	
	if( str == null )
		len = 0;
	else
		len = str.length;
	
	if( maxlen == undefined )
		maxlen = len;
	
	maxlen = Math.max(maxlen, 0);
	
	// Crop string
	if( len > maxlen )
		len = maxlen;

	return
	{
		'data':   allocate(intArrayFromString(str), 'i8', ALLOC_NORMAL),
		'length': len
	};
}

function setToIntegerReturnVar(params, value)
{
	if( glFailed() )
		return;

	var intVal;

	switch( typ(value) )
	{
		case GL.TYPE_BOOLEAN:
			if( value )
				intVal = GL.TRUE;
			else
				intVal = GL.FALSE;
			break;
		case GL.TYPE_NUMBER:
			intVal = value;
			break;
		default:
			intVal = Number(value);
			break;
	}

	IHEAP[params] = intVal;
}

function isUndefined(value)
{
	if( value == undefined )
		return GL.FALSE;
	else
		return GL.TRUE;
}

var LibraryGLES2 =
{
	$GLES2:
	{
		textures: {},
		buffers:  {},
		programs: {},
		shaders:  {},
		textureCounter: 0,
		NO_ERROR:          0,
		INVALID_ENUM:      0x0500,
		INVALID_VALUE:     0x0501,
		INVALID_OPERATION: 0x0502,
		OUT_OF_MEMORY:     0x0505,
		FALSE: 0,
		TRUE:  1,
		TYPE_BOOLEAN:  1,
		TYPE_NUMBER:   2,
		TYPE_OBJECT:   3,
		TYPE_ARRAY:    4,
		TYPE_UNDEFINED 5,
		HEAPF32: IHEAP
	},

  glBindAttribLocation: function(program, index, name)
  {
  	ctx().bindAttribLocation(prg(program), index, name);
  }
  
  glGetString: function(name_) {
    switch(name_) {
      case Module.ctxGL.VENDOR:
      case Module.ctxGL.RENDERER:
      case Module.ctxGL.VERSION:
        return allocateString(Module.ctxGL.getParameter(name_));
      case 0x1F03: // Extensions
        return allocateString(Module.ctxGL.getSupportedExtensions().join(' '));
      default:
        throw 'Failure: Invalid glGetString value: ' + name_;
    }
  },

  glDeleteTextures: function(n, textures) {
    for (var i = 0; i < n; i++) {
      var id = IHEAP[textures+QUANTUM_SIZE*i];
      Module.ctxGL.deleteTexture(GL.textures[id]);
      delete GL.textures[id];
    }
  },

  glTexImage2D: function(target, level, internalformat, width, height, border, format, type, pixels) {
    if (pixels) {
      pixels = new Uint8Array(IHEAP.slice(pixels, pixels + width*height*4)); // TODO: optimize
    }
    Module.ctxGL.texImage2D(target, level, internalformat, width, height, border, format, type, pixels);
  },

  glTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, type, pixels) {
    if (pixels) {
      pixels = new Uint8Array(IHEAP.slice(pixels, pixels + width*height*4)); // TODO: optimize
    }
    Module.ctxGL.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
  },

  glBufferData: function(target, size, data, usage)
  {
  	ctx().bufferData(target, size, data, usage);
  },
  
  glBufferSubData: function(target, offset, size, data)
  {
  	ctx().bufferSubData(target, offset, size, data);
  },
  
/* No equivalent in WebGL
  glCompressedTexImage2D: function(target, level, internalformat, width, height, border, imageSize, data)
  {
  },

  glCompressedTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, imageSize, data)
  {
  },
*/
  
  glCreateProgram: function()
  {
  	return getPrg(ctx().createProgram());
  },
  
  glCreateShader: function(type)
  {
  	return getShd(ctx().createShader(type));
  },

  glDeleteBuffers: function(n, buffers)
  {
  	for( var i = 0; i < n; i++ )
  	{
  		ctx().deleteBuffer(buf(IHEAP[buffers + i]));
  	}
  },
  
  glDeleteFramebuffers: function(n, framebuffers)
  {
  	for( var i = 0; i < n; i++ )
  	{
  		ctx().deleteFramebuffer(buf(IHEAP[framebuffers + i]);
  	}
  },
  
  glDeleteProgram: function(program)
  {
  	ctx().deleteProgram(prg(program));
  },
  
  glDeleteRenderbuffers: function(n, renderbuffers)
  {
  	for( var i = 0; i < n; i++ )
  	{
  		ctx().deleteRenderbuffer(rbf(IHEAP[renderbuffers + i]));
  	}
  },

  glDeleteShader: function(shader)
  {
  	ctx().deleteShader(shd(shader));
  },
  
  glDeleteTextures: function(n, textures)
  {
  	ctx().deleteTexture(tex(textures));
  },

  glDetachShader: function(program, shader)
  {
  	ctx().detachShader(prg(program), shd(shader));
  },
  
  glDrawElements: function(mode, count, type, indices)
  {
  },

  glFramebufferRenderbuffer: function(target, attachment, renderbuffertarget, renderbuffer)
  {
  	ctx().framebufferRenderbuffer(target, attachment, renderbuffertarget, rbf(renderbuffer));
  },
  
  glGenBuffers: function(n, buffers)
  {
  	for( var i = 0; i < n; i++ )
  	{
  		IHEAP[buffers + i] = getBuf(ctx().createBuffer());
  		
  		if( glFailed() )
  			return;
  	}
  },
  
  glGenFramebuffers: function(n, framebuffers)
  {
  	for( var i = 0; i < n; i++ )
  	{
  		IHEAP[framebuffers + i] = getFbf(ctx().createFramebuffer());
  		
  		if( glFailed() )
  			return;
  	}
  },
  
  glGenRenderbuffers: function(n, renderbuffers)
  {
  	for( var i = 0; i < n; i++ )
  	{
  		IHEAP[renderbuffers + i] = getRbf(ctx().createRenderbuffer());
  		
  		if( glFailed() )
  			return;
  	}
  },
  
  //WTF is this?
  glGenTextures__deps: ['$GL'],
  glGenTextures: function(n, textures)
  {
  	for( var i = 0; i < n; i++ )
  	{
  		GL.textures[i] = getTex(ctx().createTexture());
  		
  		if( glFailed() )
  			return; //TODO: Handle this better
  		
  		//FIXME: Do we need IHEAP here?
  		//IHEAP[textures+QUANTUM_SIZE*i] = id;
  	}
  },

  glGetActiveAttrib: function(program, index, bufsize, length, size, type, name)
  {
  	//WebGLActiveInfo
  	var info = ctx().getActiveAttrib(prg(program), index);
  	
  	if( glFailed() )
  		return;
  	
  	IHEAP[size] = info.size;
  	IHEAP[type] = info.type;
  	
  	var newStr = allocateString(info.name, bufsize);
  	
  	IHEAP[name]   = newStr.data;
  	IHEAP[length] = newStr.length;
  },

  glGetActiveUniform: function(program, index, bufsize, length, size, type, name)
  {
  	//WebGLActiveInfo
  	var info = ctx().getActiveUniform(prg(program), index);
  	
  	if( glFailed() )
  		return;
  	
  	IHEAP[size] = info.size;
  	IHEAP[type] = info.type;
  	
  	var newStr = allocateString(info.name, bufsize);
  	
  	IHEAP[name]   = newStr.data; 
  	IHEAP[length] = newStr.length;
  },
  
  glGetAttachedShaders: function(program, maxcount, count, shaders)
  {
  	var objects = ctx().getAttachedShaders(prg(program));
  	
  	if( glFailed() )
  		return;
  	
  	var i;
  	for( i = 0; i < maxcount && i < objects.length(); i++ )
  	{
  		IHEAP[shaders + i] = objects[i];
  	}
  	
  	IHEAP[count] = i;
  },

  glGetAttribLocation: function(program, name)
  {
  	return ctx().getAttribLocation(prg(program), name);
  },

  glGetBooleanv: function(pname, GLboolean* params)
  {
  	var value = getParameter(pname);
  	
  	if( glFailed() )
  		return;
  	
  	var intVal;
  	
  	switch( typ(value) )
  	{
  		case GL.TYPE_Number:
  			if( value == 0.0 )
  				intVal = GL.FALSE;
  			else
  				intVal = GL.TRUE;
  			break;
  		case GL.TYPE_BOOLEAN:
  			intVal = value;
  			break;
  		default:
  			intVal = Number(Boolean(value));
  			break;
  	}
  	
  	IHEAP[params] = intVal;
  },
  
  glGetBufferParameteriv: function(target, pname, params)
  {
  	IHEAP[params] = ctx().getBufferParameter(target, pname);
  },
  
  glGetFloatv: function(pname, params)
  {
  	var value = ctx().getParameter(pname);
  	
  	if( glFailed() )
  		return;
  	
  	var floatVal;
  	
  	switch( typ(value) )
  	{
  		case GL.TYPE_BOOLEAN:
  			if( value )
  				floatVal = GL.TRUE;
  			else
  				floatVal = GL.FALSE;
  			break;
  		case GL.TYPE_NUMBER:
  			floatVal = value;
  			break;
  		default:
  			floatVal = Number(value);
  			break;
	}
	
	IHEAP[params] = floatVal;
  },

  glGetFramebufferAttachmentParameteriv: function(target, attachment, pname, params)
  {
  	var value = ctx().getFramebufferAttachmentParameter(target, attachment, pname);
  	
  	setToIntegerReturnVar(params, value);
  },
  
  glGetIntegerv: function(pname, params)
  {
  	var value = ctx().getParameter(pname);
  	
  	setToIntegerReturnVar(params, value);
  	//TODO: Floating-point colors and normals, however, are returned with a linear mapping that maps 1.0 to the most positive representable integer value, and -1.0 to the most negative representable integer value.
  },
  
  glGetProgramiv: function(program, pname, params)
  {
  	var value = ctx().getProgramParameter(prg(program), pname);
  	
  	setToIntegerReturnVar(params, value);
  },
  
  glGetProgramInfoLog: function(program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
  {
  	var str = ctx().getProgramInfoLog(prg(program);
  	
  	if( glFailed() )
  		return;
  	
  	var newStr = allocateString(str, bufsize - 1);
  	IHEAP[infolog] = newStr.data;
  	IHEAP[length]  = newStr.length;
  },
  
  glGetRenderbufferParameteriv: function(target, pname, params)
  {
  	var value = ctx().getRenderbufferParameter(target, pname);
  	
  	setToIntegerReturnVar(params, value);
  },
  
  glGetShaderiv: function(shader, pname, params)
  {
  	var value = ctx().getShaderParameters(shd(shader), pname);
  	
  	setToIntegerReturnVar(params, value);
  },
  
  glGetShaderInfoLog: function(shader, bufsize, length, infolog)
  {
  	var info = ctx().getShaderInfoLog(shd(shader));
  	
  	if( glFailed() )
  		return;
  	
  	var newStr = allocateString(info, bufsize);
  	
  	IHEAP[infolog] = newStr.data;
  	IHEAP[length]  = newStr.length;
  },
  
  glGetShaderPrecisionFormat: function(shadertype, precisiontype, range, precision)
  {
/*
glGetShaderPrecisionFormat returns range and precision limits for floating-point and integer shader variable formats with low, medium, and high precision qualifiers. When minRep and maxRep are the minimum and maximum representable values of the format, floor ⁡ log 2 ⁡ minRep and floor ⁡ log 2 ⁡ maxRep are returned in range as the first and second elements, respectively.

If the smallest representable value greater than 1 is 1 + ∊ then floor ⁡ - log 2 ⁡ ∊ is returned in precision. An integer format will have an ∊ of 1, and thus will return 0. Floating-point formats will return values greater than 0.

Notes

The minimum range and precision required for different formats is described in the OpenGL ES Shading Language Specification.

If a high precision floating-point format is not supported for fragment shaders, calling glGetShaderPrecisionFormat with arguments GL_FRAGMENT_SHADER and GL_HIGH_FLOAT will return 0 for both range and precision. Support for a high precision floating-point format is mandatory for vertex shaders.

Shader compiler support is optional, and thus must be queried before use by calling glGet with argument GL_SHADER_COMPILER. glShaderSource, glCompileShader, glGetShaderPrecisionFormat, and glReleaseShaderCompiler will each generate GL_INVALID_OPERATION on implementations that do not support a shader compiler. Such implementations instead offer the glShaderBinary alternative for supplying a pre-compiled shader binary.

If an error is generated, no change is made to the contents of range or precision.
*/
  },
  
  glGetShaderSource: function(shader, bufsize, length, source)
  {
  	var source = ctx().getShaderSource(shd(shader));
  	
  	if( glFailed() )
  		return;
  	
  	var newSource = allocateString(source, bufsize);
  	IHEAP[source] = newSource.data;
  	IHEAP[length] = newSource.length;
  },

  glGetString: function(name)
  {
  	var value = ctx().getParameter(name);
  	
  	switch( typ(value) )
  	{
  		case GL.TYPE_STRING:
  			return value;
  		default:
  			return String(value);
  	}
  },
  
  glGetTexParameterfv (GLenum target, GLenum pname, GLfloat* params);
  glGetTexParameteriv: function(target, pname, params)
  {
  	var value = ctx().getTexParameter(target, pname);
  	setToIntegerReturnVar(params, value);
  },
  
  glGetUniformfv (GLuint program, GLint location, GLfloat* params);
  glGetUniformiv: function(program, location, params)
  {
  	var value = ctx().getUniformParameter(prg(program), location);
  	setToIntegerReturnVar(params, value);
  },
  
  glGetUniformLocation: function(program, name)
  {
  	return ctx().getUniformLocation(prg(program), name);
  },
  
  glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat* params);
  glGetVertexAttribiv: function(index, pname, params)
  {
  	var value = ctx().getVertexAttrib(index, pname);
  	setToIntegerReturnVar(params, value);
  },
  glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid** pointer);
  
  glIsBuffer: function(buffer)
  {
	ctx().isBuffer(buf(buffer)) ?
  		return Gl.TRUE:
  		return GL.FALSE;
  },
  
  glIsEnabled: function(cap)
  {
  	ctx().isEnabled(cap) ?
  		return Gl.TRUE:
  		return GL.FALSE;
  },
  
  glIsFramebuffer:function(framebuffer)
  {
  	ctx().isFramebuffer(fbf(framebuffer))  ?
  		return Gl.TRUE:
  		return GL.FALSE;
  },
  
  glIsProgram: function(program)
  {
  	ctx().isProgram(prg(program))  ?
  		return Gl.TRUE:
  		return GL.FALSE;
  },
  
  glIsRenderbuffer: function(renderbuffer)
  {
  	ctx().isRenderbuffer(rbf(renderbuffer))  ?
  		return Gl.TRUE:
  		return GL.FALSE;
  },
  
  glIsShader: function(shader)
  {
  	ctx().isShader(shd(shader))  ?
  		return Gl.TRUE:
  		return GL.FALSE;
  },
  
  glIsTexture: function(texture)
  {
  	ctx().isTexture(tex(texture)) ?
  		return Gl.TRUE:
  		return GL.FALSE;
  },

  glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
  {
  	//TODO: make more dynamic
  	var array = Uint8Array(width * height * 4);
  	
  	ctx().readPixels(x, y, width, height, format, type, array);
  	
  	if( glFailed() )
  		return;
  	
  	//TODO: optimize
  	for( var i = 0; array.begin + i < array.end; i++ )
  	{
  		IHEAP[pixels + i] = array[array.begin + i];
  	}
  },
  
  glReleaseShaderCompiler: function()
  {
  	//No op
  },

  glSampleCoverage (GLclampf value, GLboolean invert)
  {
  	ctx().sampleCoverage(value, invert == GL.TRUE);
  },


/* Not supported
  glShaderBinary (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
*/
  glShaderSource: function(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
  {
  	var newLength = 0;
  	//TODO: find better way to allocate String!
  	var source = "";
  	
  	if( length == 0)
  	{ // Null terminated
  		for( var i = 0; i < count; i++ )
  		{
  			newLength += strchr(IHEAP[length + i], '\0')
  		}
  		
  		//TODO: finish
  		if( newLength > 0 )
  		{
  		}
  	}
  	else
  	{
  		
  	
  		for( var i = 0; i < count; i++ )
  		{
  			newLength += IHEAP[length + i]
  		}
  		
  		if( newLength > 0 )
  		{
  			for( var i = 0; i < count; i++ )
  			{
  				source += IHEAP[string + i];
  			}
  		}
	}
  
  	ctx().shaderSource(shd(shader), source);
  },

  glTexImage2D: function(target, level, internalformat, width, height, border, format, type, const GLvoid* pixels)
  {
  	for( int i = 0; i < width + height * 4; i++ )
  	{
  		var source = IHEAP[pixels + i];
  	}
  	
  	
  	context.createImageData(width, height);
  	
  	//TODO: type checking
  	
  	ctx().textImage2D(target, level, internalformat, width, height, border, format type, source);
  },
  

  glTexParameterfv (GLenum target, GLenum pname, const GLfloat* params);
  glTexParameteriv (GLenum target, GLenum pname, const GLint* params)
  glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);

  glUniform1fv (GLint location, GLsizei count, const GLfloat* v);

//

  glUniform1iv (GLint location, GLsizei count, const GLint* v);

//
  toArray(argname, count, heap)
  {
  	var array = '[' + range(count).map(function(i) { return 'GLES2.' + heap + '[' + argname + ' + ' + i + ']'}).join(', ') + ']';
  },
  
  glVertexAttrib1fv (GLuint indx, const GLfloat* values);
  glVertexAttrib2fv (GLuint indx, const GLfloat* values);
  glVertexAttrib3fv (GLuint indx, const GLfloat* values);
  glVertexAttrib4fv (GLuint indx, const GLfloat* values);
  glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);

};

// Simple pass-through functions
[
	[
		0,
		[
			'glFinish',
			'glFlush',
			{
				'api_name': 'glGetError',
				'return'  : true
			}
		]
	],
	[
		1,
		[
			{
				'api_name': 'glActiveTexture',
				'wrap':
				{
					'arg_index': 0,
					func       : 'tex'
				}
			},
			'glBlendEquation',
			'glCheckFramebufferStatus',
			'glClear',
			'glClearDepthf': {
				js_name: 'clearDepth'
			},
			'glClearStencil',
			'glCompileShader':
			{
				wrap:
				[
					{
						arg_index: 0,
						func     : 'shd'
					}
				]
			},
			'glCullFace',
			'glDepthFunc',
			'glDepthMask',
			'glDepthRangef': {
				js_name: 'depthRange',
			},
			'glDisable',
			'glDisableVertexAttribArray',
			'glEnable',
			'glEnableVertexAttribArray',
			'glFrontFace',
			'glGenerateMipmap',
			'glLineWidth',
			'glLinkProgram':
			{
				'wrap':
				{
					'arg_index': 0,
					func       : 'prg'
				}
			},
			'glStencilMask',
			'glUseProgram':
			{
				'wrap':
				{
					'arg_index': 0,
					func       : 'prg'
				}
			},
			'glValidateProgram':
			{
				'wrap':
				{
					'arg_index': 0,
					func       : 'prg'
				}
			}
		]
	],
	[
		2,
		[
			'glAttachShader':
			{
				'wrap':
				[
					{
						arg_index: 0,
						func     : 'prg'
					},
					{
						arg_index: 1,
						func     : 'shd'
					}
				]
			},
			'glBindBuffer':
			{
				wrap:
				[
					{
						arg_index: 1,
						func     : 'buf'
					}
				]
			},
			'glBindFramebuffer':
			{
				wrap:
				[
					{
						arg_index: 1,
						func     : 'fbf'
					}
				]
			},
			'glBindRenderbuffer':
			{
				wrap:
				[
					{
						arg_index: 1,
						func     : 'rbf'
					}
				]
			},
			'glBindTexture':
			{
				wrap:
				[
					{
						arg_index: 1,
						func     : 'tex'
					}
				]
			},
			'glBlendEquationSeparate',
			'glBlendFunc',
			'glHint',
			'glPixelStorei',
			'glPolygonOffset',
			'glStencilMaskSeparate'
			'glUniform1f',
			'glUniform1i',
			'glVertexAttrib1f'
		]
	],
	[
		3,
		[
			'glDrawArrays',
			'glRenderbufferStorage',
			'glStencilFunc',
			'glStencilOp',
			'glTexParameterf',
			'glTexParameteri',
			'glUniform2f',
			'glUniform2fv':
			{
				'array' :
				[
					{
						'count_arg_index': 1,
						'arg_index'      : 2,
						type             : GLES2.FLOAT32
					}
				]
			}
			'glUniform2i',
			'glUniform2iv': 
			{
				'array' :
				[
					{
						'count_arg_index': 1,
						'arg_index'      : 2,
						type             : GLES2.INT32
					}
				]
			},
			'glUniform3fv': 
			{
				'array' :
				[
					{
						'count_arg_index': 1,
						'arg_index'      : 2,
						type             : GLES2.FLOAT32
					}
				]
			},
			'glUniform3iv':
			{
				'array' :
				[
					{
						'count_arg_index': 1,
						'arg_index'      : 2,
						type             : GLES2.INT32
					}
				]
			},
			'glUniform4fv':
			{
				'array' :
				[
					{
						'count_arg_index': 1,
						'arg_index'      : 2,
						type             : GLES2.FLOAT32
					}
				]
			},
			'glUniform4iv':
			{
				'array' :
				[
					{
						'count_arg_index': 1,
						'arg_index'      : 2,
						type:            : GLES2.INT32
					}
				]
			}
			'glVertexAttrib2f'
		]
	],
	[
		4,
		[
			'glBlendColor',
			'glBlendFuncSeparate',
			'glClearColor',
			'glColorMask',
			'glScissor',
			'glStencilFuncSeparate',
			'glStencilOpSeparate',
			'glUniform3f',
			'glUniform3i',
			'glUniformMatrix2fv':
			{
				'array' :
				[
					{
						'count_arg_index': 1,
						'arg_index'      : 3,
						type             : GLES2.FLOAT32
					}
				]
			},
			'glUniformMatrix3fv':
			{
				'array' :
				[
					{
						'count_arg_index': 1,
						'arg_index'      : 3,
						type             : GLES2.FLOAT32
					}
				]
			},
			'glUniformMatrix4fv'
			{
				'array' :
				[
					{
						'count_arg_index': 1,
						'arg_index'      : 3,
						type             : GLES2.FLOAT32
					}
				]
			}
			'glVertexAttrib3f'
		]
	],
	[
		5,
		[
			'glUniform4f',
			'glUniform4i',
			'glVertexAttrib4f'
			'glViewport',
			'glFramebufferTexture2D'
		]
	],
	[
		8,
		[
			'glCopyTexImage2D',
			'glCopyTexSubImage2D'
		]
	]
].forEach(function(data) {
	var num = data[0];
	var names = data[1];
	var args = range(num).map(function(i) { return 'x' + i }).join(', ');

	names.forEach(function(entry) {
		var isReturn = false;
		var cName = null;
		var jsName = null;

		if( is_String(entry) )
			cName = entry;
		else
		{
			cName = entry.api_name;
	
			if( entry.return )
				isReturn = true;
	
			if( entry.js_name !== undefined )
				jsName = entry.js_name;
	
			if( entry.array !== undefined )
			{
				entry.array.forEach(function(array_entry){
					var countArgIndex = array_entry.count_arg_index;
					var argIndex      = array_entry.arg_index;
			
					var heap;
					switch( array_entry.type )
					{
						case GLES2.INT32:
							heap = "HEAPI32";
							break;
						case GLES2.FLOAT32:
							heap = "HEAPF32";
							break;
					}
			
					args = args.replace('x' + argIndex, toArray(countArgIndex, 'x' + argIndex, heap));
				});
			}
		}

		if( jsName == null )
			jsName = cName[2].toUpperCase() + cName.substr(3);

		var stub = '(function(' + args + ') { '
			+ (isReturn ? 'return ' : '')
			+ (num > 0 ? 'ctx().' + jsName + '(' + args + ');' : '') + ' })';
		LibraryGLES2[cName] = eval(stub);
		};
	});
});

mergeInto(LibraryManager.library, LibraryGLES2);

