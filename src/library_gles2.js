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

var LibraryGLES2 = {
  $GLES2: {
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
    TYPE_UNDEFINED 5
  },
  
  glActiveTexture: function(GLenum texture)
  {
  	ctx().activeTexture(tex(texture));
  }
  
  glAttachShader: function(program, shader)
  {
  	ctx().attachShader(prg(program), shader);
  }
  
  glBindAttribLocation: function(program, index, name)
  {
  	ctx().bindAttribLocation(prg(program), index, name);
  }
  
  glBindBuffer: function(target, buffer)
  {
  	ctx().bindBuffer(target, buf(buffer));
  },
  
  glBindFramebuffer: function(target, framebuffer)
  {
  	ctx().bindFramebuffer(target, fbf(framebuffer));
  },
  
  glBindRenderbuffer: function(target, renderbuffer)
  {
  	ctx().bindRenderbuffer(target, rbf(renderbuffer));
  },

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

  glBindTexture: function(target, texture) {
    ctx().bindTexture(target, tex(texture));
  },
  
  glBufferData: function(target, size, data, usage)
  {
  	ctx().bufferData(target, size, data, usage);
  },
  
  glBufferSubData: function(target, offset, size, data)
  {
  	ctx().bufferSubData(target, offset, size, data);
  },
  
  glCheckFramebufferStatus: function(target)
  {
  	return ctx().checkFramebufferStatus(target);
  },
  
  glClear: function(mask)
  {
  	ctx().clear(mask);
  },
  
  glClearColor: function(red, green, blue, alpha)
  {
  	ctx().clearColor(red, green, blue, alpha);
  },
  
  glClearDepthf: function(depth)
  {
  	ctx().clearDepthf(depth);
  },
  
  glClearStencil: function(s)
  {
  	ctx().clearStencil(s);
  },
  
  glColorMask: function(red, green, blue, alpha)
  {
  	ctx().colorMask(red, green, blue, alpha);
  },
  
  glCompileShader: function(shader)
  {
  	ctx().compileShader(shd(shader));
  },
/* No equivalent in WebGL
  glCompressedTexImage2D: function(target, level, internalformat, width, height, border, imageSize, data)
  {
  },

  glCompressedTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, imageSize, data)
  {
  },
*/
  glCopyTexImage2D: function(target, level, internalformat, x, y, width, height, border)
  {
  	ctx().copyTexImage2D(target, level, internalformat, x, y, width, height, border);
  },
  
  glCopyTexSubImage2D: function(target, level, xoffset, yoffset, x, y, width, height)
  {
  	ctx().copyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
  },
  
  glCreateProgram: function()
  {
  	return getPrg(ctx().createProgram());
  },
  
  glCreateShader: function(type)
  {
  	return getShd(ctx().createShader(type));
  },
  
  glCullFace: function(mode)
  {
  	return ctx().cullFace(mode);
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

  glLinkProgram: function(program)
  {
  	ctx().linkProgram(prg(program));
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

  glUniform2fv: function(GLint location, GLsizei count, const GLfloat* v)
  {
  	var n = count / 2;
  	n = Math.floor(n);
  	
  	for( int i = 0; i < n; i += 2 )
  	{
  		ctx().uniform2fv(IHEAP[v + i], IHEAP[v + i + 1]);
  	}
  },
  glUniform2iv (GLint location, GLsizei count, const GLint* v);
  glUniform3fv (GLint location, GLsizei count, const GLfloat* v);
  glUniform3iv (GLint location, GLsizei count, const GLint* v);
  glUniform4fv (GLint location, GLsizei count, const GLfloat* v);
  glUniform4iv (GLint location, GLsizei count, const GLint* v);
  glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
  glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
  glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
  
  glUseProgram: function(program)
  {
  	ctx().useProgram(prg(program));
  },
  
  glValidateProgram: function(program)
  {
  	ctx().validateProgram(prg(program));
  },
  
  
  glVertexAttrib1fv (GLuint indx, const GLfloat* values);
  glVertexAttrib2fv (GLuint indx, const GLfloat* values);
  glVertexAttrib3fv (GLuint indx, const GLfloat* values);
  glVertexAttrib4fv (GLuint indx, const GLfloat* values);
  glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);

};

// Ignored stubs for fixed-function pipeline. We will need to emulate this
'begin end matrixMode loadIdentity ortho color3f texCoord2f vertex2f blendFunc pushMatrix popMatrix translatef scalef color4ub enableClientState disableClientState vertexPointer colorPointer normalPointer texCoordPointer drawArrays clientActiveTexture_'.split(' ').forEach(function(name_) {
  var cName = 'gl' + name_[0].toUpperCase() + name_.substr(1);
  LibraryGL[cName] = function(){};
});

// Simple pass-through functions
[
	[
		0,
		[
			'glFinish',
			'glFlush',
			{
				'api_name': 'glGetError',
				'return': true
			}
		]
	],
	[
		1,
		[
			'glBlendEquation',
			'glDepthFunc',
			'glDepthMask',
			{
				'api_name': 'glDepthRangef',
				'js_name': 'depthRange',
			},
			'glDisable',
			'glDisableVertexAttribArray',
			'glEnable',
			'glEnableVertexAttribArray',
			'glFrontFace',
			'glGenerateMipmap',
			'glLineWidth',
			'glStencilMask'
		]
	],
	[
		2,
		[
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
			'glUniform2i',
			'glVertexAttrib2f'
		]
	],
	[
		4,
		[
			'glBlendColor',
			'glBlendFuncSeparate'
			'glScissor',
			'glStencilFuncSeparate',
			'glStencilOpSeparate',
			'glUniform3f',
			'glUniform3i',
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
      
      if( entry.return == true )
        isReturn = true;
     
      if( entry.js_name !== undefined )
        jsName = entry.js_name;
    }
    
    if( jsName == null )
      jsName = cName[2].toUpperCase() + cName.substr(3);
      
    var stub = '(function(' + args + ') { '
      + (isReturn ? 'return ' : '')
      + (num > 0 ? 'ctx().' + jsName + '(' + args + ');' : '') + ' })';
    LibraryGLES2[cName] = eval(stub);
  };
  
});




[[0, 'shadeModel fogi fogfv'],
 [1, 'clearDepth depthFunc enable disable frontFace cullFace'],
 [2, 'pixelStorei'],
 [3, 'texParameteri texParameterf'],
 [4, 'viewport clearColor']].forEach(function(data) {
  var num = data[0];
  var names = data[1];
  var args = range(num).map(function(i) { return 'x' + i }).join(', ');
  var stub = '(function(' + args + ') { ' + (num > 0 ? 'Module.ctxGL.NAME(' + args + ')' : '') + ' })';
  names.split(' ').forEach(function(name_) {
    var cName = 'gl' + name_[0].toUpperCase() + name_.substr(1);
    LibraryGL[cName] = eval(stub.replace('NAME', name_));
    //print(cName + ': ' + LibraryGL[cName]);
  });
});

mergeInto(LibraryManager.library, LibraryGLES2);

