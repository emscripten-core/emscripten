/* Errors / GetError return values */
var SUCCESS             = 0x3000;
var NOT_INITIALIZED     = 0x3001;
var BAD_ACCESS          = 0x3002;
var BAD_ALLOC           = 0x3003;
var BAD_ATTRIBUTE       = 0x3004;
var BAD_CONFIG          = 0x3005;
var BAD_CONTEXT         = 0x3006;
var BAD_CURRENT_SURFACE = 0x3007;
var BAD_DISPLAY         = 0x3008;
var BAD_MATCH           = 0x3009;
var BAD_NATIVE_PIXMAP   = 0x300A;
var BAD_NATIVE_WINDOW   = 0x300B;
var BAD_PARAMETER       = 0x300C;
var BAD_SURFACE         = 0x300D;
var CONTEXT_LOST        = 0x300E;

var KHRONOS_FALSE = 0;
var KHRONOS_TRUE  = 1;

var WGL_TRUE = 0;
var WGL_FALSE = 1;

var _gl_error = SUCCESS;
var _gl_current_ctx = null;

var _gl_names = ["webgl", "experimental-webgl"];

var LibraryWGL =
{
	wglInitialize: function(major, minor)
	{
		return WGL_TRUE; // No op
	},
	
	wglTerminate: function()
	{
		return WGL_TRUE; // No op
	},
	
	wglGetError: function ()
	{
		return _gl_error;
	},
	
	wglGetContext: function(canvas)
	{
		if( !canvas )
		{
			_gl_error = BAD_ATTRIBUTE;
			return null;
		}
	
		for( var ctxName in _gl_names )
		{
			var ctx = canvas.getContext(ctxName);
		
			if( ctx )
			{
				_gl_error = SUCCESS;
				return ctx;
			}
			//else not gotten
		}
	
		_gl_error = BAD_ATTRIBUTE;
		return null;
	},
	
	wglMakeCurrent: function(ctx)
	{
		_gl_error = SUCCESS;
		_gl_current_ctx = ctx;
		return WGL_TRUE;
	},

	wglGetCurrentContext: function()
	{
		_gl_error = SUCCESS;
		return _gl_current_ctx;
	}
}

mergeInto(LibraryManager.library, LibraryWGL);
