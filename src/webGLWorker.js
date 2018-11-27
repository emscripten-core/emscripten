// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// WebGLWorker worker code

function WebGLBuffer(id) {
  this.what = 'buffer';
  this.id = id;
}
function WebGLProgram(id) {
  this.what = 'program';
  this.id = id;
  this.shaders = [];
  this.attributes = {};
  this.attributeVec = [];
  this.nextAttributes = {};
  this.nextAttributeVec = [];
}
function WebGLFramebuffer(id) {
  this.what = 'frameBuffer';
  this.id = id;
}
function WebGLRenderbuffer(id) {
  this.what = 'renderBuffer';
  this.id = id;
}
function WebGLTexture(id) {
  this.what = 'texture';
  this.id = id;
  this.binding = 0;
}

function WebGLWorker() {
  //===========
  // Constants
  //===========

  /* ClearBufferMask */
  this.DEPTH_BUFFER_BIT               = 0x00000100;
  this.STENCIL_BUFFER_BIT             = 0x00000400;
  this.COLOR_BUFFER_BIT               = 0x00004000;
  
  /* BeginMode */
  this.POINTS                         = 0x0000;
  this.LINES                          = 0x0001;
  this.LINE_LOOP                      = 0x0002;
  this.LINE_STRIP                     = 0x0003;
  this.TRIANGLES                      = 0x0004;
  this.TRIANGLE_STRIP                 = 0x0005;
  this.TRIANGLE_FAN                   = 0x0006;
  
  /* AlphaFunction (not supported in ES20) */
  /*      NEVER */
  /*      LESS */
  /*      EQUAL */
  /*      LEQUAL */
  /*      GREATER */
  /*      NOTEQUAL */
  /*      GEQUAL */
  /*      ALWAYS */
  
  /* BlendingFactorDest */
  this.ZERO                           = 0;
  this.ONE                            = 1;
  this.SRC_COLOR                      = 0x0300;
  this.ONE_MINUS_SRC_COLOR            = 0x0301;
  this.SRC_ALPHA                      = 0x0302;
  this.ONE_MINUS_SRC_ALPHA            = 0x0303;
  this.DST_ALPHA                      = 0x0304;
  this.ONE_MINUS_DST_ALPHA            = 0x0305;
  
  /* BlendingFactorSrc */
  /*      ZERO */
  /*      ONE */
  this.DST_COLOR                      = 0x0306;
  this.ONE_MINUS_DST_COLOR            = 0x0307;
  this.SRC_ALPHA_SATURATE             = 0x0308;
  /*      SRC_ALPHA */
  /*      ONE_MINUS_SRC_ALPHA */
  /*      DST_ALPHA */
  /*      ONE_MINUS_DST_ALPHA */
  
  /* BlendEquationSeparate */
  this.FUNC_ADD                       = 0x8006;
  this.BLEND_EQUATION                 = 0x8009;
  this.BLEND_EQUATION_RGB             = 0x8009;   /* same as BLEND_EQUATION */
  this.BLEND_EQUATION_ALPHA           = 0x883D;
  
  /* BlendSubtract */
  this.FUNC_SUBTRACT                  = 0x800A;
  this.FUNC_REVERSE_SUBTRACT          = 0x800B;
  
  /* Separate Blend Functions */
  this.BLEND_DST_RGB                  = 0x80C8;
  this.BLEND_SRC_RGB                  = 0x80C9;
  this.BLEND_DST_ALPHA                = 0x80CA;
  this.BLEND_SRC_ALPHA                = 0x80CB;
  this.CONSTANT_COLOR                 = 0x8001;
  this.ONE_MINUS_CONSTANT_COLOR       = 0x8002;
  this.CONSTANT_ALPHA                 = 0x8003;
  this.ONE_MINUS_CONSTANT_ALPHA       = 0x8004;
  this.BLEND_COLOR                    = 0x8005;
  
  /* Buffer Objects */
  this.ARRAY_BUFFER                   = 0x8892;
  this.ELEMENT_ARRAY_BUFFER           = 0x8893;
  this.ARRAY_BUFFER_BINDING           = 0x8894;
  this.ELEMENT_ARRAY_BUFFER_BINDING   = 0x8895;
  
  this.STREAM_DRAW                    = 0x88E0;
  this.STATIC_DRAW                    = 0x88E4;
  this.DYNAMIC_DRAW                   = 0x88E8;
  
  this.BUFFER_SIZE                    = 0x8764;
  this.BUFFER_USAGE                   = 0x8765;
  
  this.CURRENT_VERTEX_ATTRIB          = 0x8626;
  
  /* CullFaceMode */
  this.FRONT                          = 0x0404;
  this.BACK                           = 0x0405;
  this.FRONT_AND_BACK                 = 0x0408;
  
  /* DepthFunction */
  /*      NEVER */
  /*      LESS */
  /*      EQUAL */
  /*      LEQUAL */
  /*      GREATER */
  /*      NOTEQUAL */
  /*      GEQUAL */
  /*      ALWAYS */
  
  /* EnableCap */
  /* TEXTURE_2D */
  this.CULL_FACE                      = 0x0B44;
  this.BLEND                          = 0x0BE2;
  this.DITHER                         = 0x0BD0;
  this.STENCIL_TEST                   = 0x0B90;
  this.DEPTH_TEST                     = 0x0B71;
  this.SCISSOR_TEST                   = 0x0C11;
  this.POLYGON_OFFSET_FILL            = 0x8037;
  this.SAMPLE_ALPHA_TO_COVERAGE       = 0x809E;
  this.SAMPLE_COVERAGE                = 0x80A0;
  
  /* ErrorCode */
  this.NO_ERROR                       = 0;
  this.INVALID_ENUM                   = 0x0500;
  this.INVALID_VALUE                  = 0x0501;
  this.INVALID_OPERATION              = 0x0502;
  this.OUT_OF_MEMORY                  = 0x0505;
  
  /* FrontFaceDirection */
  this.CW                             = 0x0900;
  this.CCW                            = 0x0901;
  
  /* GetPName */
  this.LINE_WIDTH                     = 0x0B21;
  this.ALIASED_POINT_SIZE_RANGE       = 0x846D;
  this.ALIASED_LINE_WIDTH_RANGE       = 0x846E;
  this.CULL_FACE_MODE                 = 0x0B45;
  this.FRONT_FACE                     = 0x0B46;
  this.DEPTH_RANGE                    = 0x0B70;
  this.DEPTH_WRITEMASK                = 0x0B72;
  this.DEPTH_CLEAR_VALUE              = 0x0B73;
  this.DEPTH_FUNC                     = 0x0B74;
  this.STENCIL_CLEAR_VALUE            = 0x0B91;
  this.STENCIL_FUNC                   = 0x0B92;
  this.STENCIL_FAIL                   = 0x0B94;
  this.STENCIL_PASS_DEPTH_FAIL        = 0x0B95;
  this.STENCIL_PASS_DEPTH_PASS        = 0x0B96;
  this.STENCIL_REF                    = 0x0B97;
  this.STENCIL_VALUE_MASK             = 0x0B93;
  this.STENCIL_WRITEMASK              = 0x0B98;
  this.STENCIL_BACK_FUNC              = 0x8800;
  this.STENCIL_BACK_FAIL              = 0x8801;
  this.STENCIL_BACK_PASS_DEPTH_FAIL   = 0x8802;
  this.STENCIL_BACK_PASS_DEPTH_PASS   = 0x8803;
  this.STENCIL_BACK_REF               = 0x8CA3;
  this.STENCIL_BACK_VALUE_MASK        = 0x8CA4;
  this.STENCIL_BACK_WRITEMASK         = 0x8CA5;
  this.VIEWPORT                       = 0x0BA2;
  this.SCISSOR_BOX                    = 0x0C10;
  /*      SCISSOR_TEST */
  this.COLOR_CLEAR_VALUE              = 0x0C22;
  this.COLOR_WRITEMASK                = 0x0C23;
  this.UNPACK_ALIGNMENT               = 0x0CF5;
  this.PACK_ALIGNMENT                 = 0x0D05;
  this.MAX_TEXTURE_SIZE               = 0x0D33;
  this.MAX_VIEWPORT_DIMS              = 0x0D3A;
  this.SUBPIXEL_BITS                  = 0x0D50;
  this.RED_BITS                       = 0x0D52;
  this.GREEN_BITS                     = 0x0D53;
  this.BLUE_BITS                      = 0x0D54;
  this.ALPHA_BITS                     = 0x0D55;
  this.DEPTH_BITS                     = 0x0D56;
  this.STENCIL_BITS                   = 0x0D57;
  this.POLYGON_OFFSET_UNITS           = 0x2A00;
  /*      POLYGON_OFFSET_FILL */
  this.POLYGON_OFFSET_FACTOR          = 0x8038;
  this.TEXTURE_BINDING_2D             = 0x8069;
  this.SAMPLE_BUFFERS                 = 0x80A8;
  this.SAMPLES                        = 0x80A9;
  this.SAMPLE_COVERAGE_VALUE          = 0x80AA;
  this.SAMPLE_COVERAGE_INVERT         = 0x80AB;
  
  /* GetTextureParameter */
  /*      TEXTURE_MAG_FILTER */
  /*      TEXTURE_MIN_FILTER */
  /*      TEXTURE_WRAP_S */
  /*      TEXTURE_WRAP_T */
  
  this.COMPRESSED_TEXTURE_FORMATS     = 0x86A3;
  
  /* HintMode */
  this.DONT_CARE                      = 0x1100;
  this.FASTEST                        = 0x1101;
  this.NICEST                         = 0x1102;
  
  /* HintTarget */
  this.GENERATE_MIPMAP_HINT            = 0x8192;
  
  /* DataType */
  this.BYTE                           = 0x1400;
  this.UNSIGNED_BYTE                  = 0x1401;
  this.SHORT                          = 0x1402;
  this.UNSIGNED_SHORT                 = 0x1403;
  this.INT                            = 0x1404;
  this.UNSIGNED_INT                   = 0x1405;
  this.FLOAT                          = 0x1406;
  
  /* PixelFormat */
  this.DEPTH_COMPONENT                = 0x1902;
  this.ALPHA                          = 0x1906;
  this.RGB                            = 0x1907;
  this.RGBA                           = 0x1908;
  this.LUMINANCE                      = 0x1909;
  this.LUMINANCE_ALPHA                = 0x190A;
  
  /* PixelType */
  /*      UNSIGNED_BYTE */
  this.UNSIGNED_SHORT_4_4_4_4         = 0x8033;
  this.UNSIGNED_SHORT_5_5_5_1         = 0x8034;
  this.UNSIGNED_SHORT_5_6_5           = 0x8363;
  
  /* Shaders */
  this.FRAGMENT_SHADER                  = 0x8B30;
  this.VERTEX_SHADER                    = 0x8B31;
  this.MAX_VERTEX_ATTRIBS               = 0x8869;
  this.MAX_VERTEX_UNIFORM_VECTORS       = 0x8DFB;
  this.MAX_VARYING_VECTORS              = 0x8DFC;
  this.MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D;
  this.MAX_VERTEX_TEXTURE_IMAGE_UNITS   = 0x8B4C;
  this.MAX_TEXTURE_IMAGE_UNITS          = 0x8872;
  this.MAX_FRAGMENT_UNIFORM_VECTORS     = 0x8DFD;
  this.SHADER_TYPE                      = 0x8B4F;
  this.DELETE_STATUS                    = 0x8B80;
  this.LINK_STATUS                      = 0x8B82;
  this.VALIDATE_STATUS                  = 0x8B83;
  this.ATTACHED_SHADERS                 = 0x8B85;
  this.ACTIVE_UNIFORMS                  = 0x8B86;
  this.ACTIVE_ATTRIBUTES                = 0x8B89;
  this.SHADING_LANGUAGE_VERSION         = 0x8B8C;
  this.CURRENT_PROGRAM                  = 0x8B8D;
  
  /* StencilFunction */
  this.NEVER                          = 0x0200;
  this.LESS                           = 0x0201;
  this.EQUAL                          = 0x0202;
  this.LEQUAL                         = 0x0203;
  this.GREATER                        = 0x0204;
  this.NOTEQUAL                       = 0x0205;
  this.GEQUAL                         = 0x0206;
  this.ALWAYS                         = 0x0207;
  
  /* StencilOp */
  /*      ZERO */
  this.KEEP                           = 0x1E00;
  this.REPLACE                        = 0x1E01;
  this.INCR                           = 0x1E02;
  this.DECR                           = 0x1E03;
  this.INVERT                         = 0x150A;
  this.INCR_WRAP                      = 0x8507;
  this.DECR_WRAP                      = 0x8508;
  
  /* StringName */
  this.VENDOR                         = 0x1F00;
  this.RENDERER                       = 0x1F01;
  this.VERSION                        = 0x1F02;
  
  /* TextureMagFilter */
  this.NEAREST                        = 0x2600;
  this.LINEAR                         = 0x2601;
  
  /* TextureMinFilter */
  /*      NEAREST */
  /*      LINEAR */
  this.NEAREST_MIPMAP_NEAREST         = 0x2700;
  this.LINEAR_MIPMAP_NEAREST          = 0x2701;
  this.NEAREST_MIPMAP_LINEAR          = 0x2702;
  this.LINEAR_MIPMAP_LINEAR           = 0x2703;
  
  /* TextureParameterName */
  this.TEXTURE_MAG_FILTER             = 0x2800;
  this.TEXTURE_MIN_FILTER             = 0x2801;
  this.TEXTURE_WRAP_S                 = 0x2802;
  this.TEXTURE_WRAP_T                 = 0x2803;
  
  /* TextureTarget */
  this.TEXTURE_2D                     = 0x0DE1;
  this.TEXTURE                        = 0x1702;
  
  this.TEXTURE_CUBE_MAP               = 0x8513;
  this.TEXTURE_BINDING_CUBE_MAP       = 0x8514;
  this.TEXTURE_CUBE_MAP_POSITIVE_X    = 0x8515;
  this.TEXTURE_CUBE_MAP_NEGATIVE_X    = 0x8516;
  this.TEXTURE_CUBE_MAP_POSITIVE_Y    = 0x8517;
  this.TEXTURE_CUBE_MAP_NEGATIVE_Y    = 0x8518;
  this.TEXTURE_CUBE_MAP_POSITIVE_Z    = 0x8519;
  this.TEXTURE_CUBE_MAP_NEGATIVE_Z    = 0x851A;
  this.MAX_CUBE_MAP_TEXTURE_SIZE      = 0x851C;
  
  /* TextureUnit */
  this.TEXTURE0                       = 0x84C0;
  this.TEXTURE1                       = 0x84C1;
  this.TEXTURE2                       = 0x84C2;
  this.TEXTURE3                       = 0x84C3;
  this.TEXTURE4                       = 0x84C4;
  this.TEXTURE5                       = 0x84C5;
  this.TEXTURE6                       = 0x84C6;
  this.TEXTURE7                       = 0x84C7;
  this.TEXTURE8                       = 0x84C8;
  this.TEXTURE9                       = 0x84C9;
  this.TEXTURE10                      = 0x84CA;
  this.TEXTURE11                      = 0x84CB;
  this.TEXTURE12                      = 0x84CC;
  this.TEXTURE13                      = 0x84CD;
  this.TEXTURE14                      = 0x84CE;
  this.TEXTURE15                      = 0x84CF;
  this.TEXTURE16                      = 0x84D0;
  this.TEXTURE17                      = 0x84D1;
  this.TEXTURE18                      = 0x84D2;
  this.TEXTURE19                      = 0x84D3;
  this.TEXTURE20                      = 0x84D4;
  this.TEXTURE21                      = 0x84D5;
  this.TEXTURE22                      = 0x84D6;
  this.TEXTURE23                      = 0x84D7;
  this.TEXTURE24                      = 0x84D8;
  this.TEXTURE25                      = 0x84D9;
  this.TEXTURE26                      = 0x84DA;
  this.TEXTURE27                      = 0x84DB;
  this.TEXTURE28                      = 0x84DC;
  this.TEXTURE29                      = 0x84DD;
  this.TEXTURE30                      = 0x84DE;
  this.TEXTURE31                      = 0x84DF;
  this.ACTIVE_TEXTURE                 = 0x84E0;
  
  /* TextureWrapMode */
  this.REPEAT                         = 0x2901;
  this.CLAMP_TO_EDGE                  = 0x812F;
  this.MIRRORED_REPEAT                = 0x8370;
  
  /* Uniform Types */
  this.FLOAT_VEC2                     = 0x8B50;
  this.FLOAT_VEC3                     = 0x8B51;
  this.FLOAT_VEC4                     = 0x8B52;
  this.INT_VEC2                       = 0x8B53;
  this.INT_VEC3                       = 0x8B54;
  this.INT_VEC4                       = 0x8B55;
  this.BOOL                           = 0x8B56;
  this.BOOL_VEC2                      = 0x8B57;
  this.BOOL_VEC3                      = 0x8B58;
  this.BOOL_VEC4                      = 0x8B59;
  this.FLOAT_MAT2                     = 0x8B5A;
  this.FLOAT_MAT3                     = 0x8B5B;
  this.FLOAT_MAT4                     = 0x8B5C;
  this.SAMPLER_2D                     = 0x8B5E;
  this.SAMPLER_3D                     = 0x8B5F;
  this.SAMPLER_CUBE                   = 0x8B60;
  
  /* Vertex Arrays */
  this.VERTEX_ATTRIB_ARRAY_ENABLED        = 0x8622;
  this.VERTEX_ATTRIB_ARRAY_SIZE           = 0x8623;
  this.VERTEX_ATTRIB_ARRAY_STRIDE         = 0x8624;
  this.VERTEX_ATTRIB_ARRAY_TYPE           = 0x8625;
  this.VERTEX_ATTRIB_ARRAY_NORMALIZED     = 0x886A;
  this.VERTEX_ATTRIB_ARRAY_POINTER        = 0x8645;
  this.VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F;
  
  /* Read Format */
  this.IMPLEMENTATION_COLOR_READ_TYPE   = 0x8B9A;
  this.IMPLEMENTATION_COLOR_READ_FORMAT = 0x8B9B;

  /* Shader Source */
  this.COMPILE_STATUS                 = 0x8B81;
  
  /* Shader Precision-Specified Types */
  this.LOW_FLOAT                      = 0x8DF0;
  this.MEDIUM_FLOAT                   = 0x8DF1;
  this.HIGH_FLOAT                     = 0x8DF2;
  this.LOW_INT                        = 0x8DF3;
  this.MEDIUM_INT                     = 0x8DF4;
  this.HIGH_INT                       = 0x8DF5;
  
  /* Framebuffer Object. */
  this.FRAMEBUFFER                    = 0x8D40;
  this.RENDERBUFFER                   = 0x8D41;
  
  this.RGBA4                          = 0x8056;
  this.RGB5_A1                        = 0x8057;
  this.RGB565                         = 0x8D62;
  this.DEPTH_COMPONENT16              = 0x81A5;
  this.STENCIL_INDEX                  = 0x1901;
  this.STENCIL_INDEX8                 = 0x8D48;
  this.DEPTH_STENCIL                  = 0x84F9;
  
  this.RENDERBUFFER_WIDTH             = 0x8D42;
  this.RENDERBUFFER_HEIGHT            = 0x8D43;
  this.RENDERBUFFER_INTERNAL_FORMAT   = 0x8D44;
  this.RENDERBUFFER_RED_SIZE          = 0x8D50;
  this.RENDERBUFFER_GREEN_SIZE        = 0x8D51;
  this.RENDERBUFFER_BLUE_SIZE         = 0x8D52;
  this.RENDERBUFFER_ALPHA_SIZE        = 0x8D53;
  this.RENDERBUFFER_DEPTH_SIZE        = 0x8D54;
  this.RENDERBUFFER_STENCIL_SIZE      = 0x8D55;
  
  this.FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE           = 0x8CD0;
  this.FRAMEBUFFER_ATTACHMENT_OBJECT_NAME           = 0x8CD1;
  this.FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL         = 0x8CD2;
  this.FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0x8CD3;
  
  this.COLOR_ATTACHMENT0              = 0x8CE0;
  this.DEPTH_ATTACHMENT               = 0x8D00;
  this.STENCIL_ATTACHMENT             = 0x8D20;
  this.DEPTH_STENCIL_ATTACHMENT       = 0x821A;
  
  this.NONE                           = 0;
  
  this.FRAMEBUFFER_COMPLETE                      = 0x8CD5;
  this.FRAMEBUFFER_INCOMPLETE_ATTACHMENT         = 0x8CD6;
  this.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7;
  this.FRAMEBUFFER_INCOMPLETE_DIMENSIONS         = 0x8CD9;
  this.FRAMEBUFFER_UNSUPPORTED                   = 0x8CDD;
  
  this.ACTIVE_TEXTURE                 = 0x84E0;
  this.FRAMEBUFFER_BINDING            = 0x8CA6;
  this.RENDERBUFFER_BINDING           = 0x8CA7;
  this.MAX_RENDERBUFFER_SIZE          = 0x84E8;
  
  this.INVALID_FRAMEBUFFER_OPERATION  = 0x0506;
  
  /* WebGL-specific enums */
  this.UNPACK_FLIP_Y_WEBGL            = 0x9240;
  this.UNPACK_PREMULTIPLY_ALPHA_WEBGL = 0x9241;
  this.CONTEXT_LOST_WEBGL             = 0x9242;
  this.UNPACK_COLORSPACE_CONVERSION_WEBGL = 0x9243;
  this.BROWSER_DEFAULT_WEBGL          = 0x9244;

  //=======
  // State
  //=======

  var commandBuffer = [];

  var nextId = 1; // valid ids are > 0

  var bindings = {
    texture2D: null,
    arrayBuffer: null,
    elementArrayBuffer: null,
    program: null,
    framebuffer: null,
    activeTexture: this.TEXTURE0,
    generateMipmapHint: this.DONT_CARE,
    blendSrcRGB: this.ONE,
    blendSrcAlpha: this.ONE,
    blendDstRGB: this.ZERO,
    blendDstAlpha: this.ZERO,
    blendEquationRGB: this.FUNC_ADD,
    blendEquationAlpha: this.FUNC_ADD,
    enabledState: {} // Stores whether various GL state via glEnable/glDisable/glIsEnabled/getParameter are enabled.
  };
  var stateDisabledByDefault = [this.BLEND, this.CULL_FACE, this.DEPTH_TEST, this.DITHER, this.POLYGON_OFFSET_FILL, this.SAMPLE_ALPHA_TO_COVERAGE, this.SAMPLE_COVERAGE, this.SCISSOR_TEST, this.STENCIL_TEST];
  for(var i in stateDisabledByDefault) {
    bindings.enabledState[stateDisabledByDefault[i]] = false; // It will be important to distinguish between false and undefined (undefined meaning the state cap enum is unknown/unsupported).
  }

  //==========
  // Functions
  //==========

  var that = this;

  // Helpers

  this.onmessage = function(msg) {
    //dump('worker GL got ' + JSON.stringify(msg) + '\n');
    switch(msg.op) {
      case 'setPrefetched': {
        WebGLWorker.prototype.prefetchedParameters = msg.parameters;
        WebGLWorker.prototype.prefetchedExtensions = msg.extensions;
        WebGLWorker.prototype.prefetchedPrecisions = msg.precisions;
        removeRunDependency('gl-prefetch');
        break;
      }
      default: throw 'weird gl onmessage ' + JSON.stringify(msg);
    }
  };

  function revname(name) {
    for (var x in that) if (that[x] === name) return x;
    return null;
  }

  // GL

  this.getParameter = function(name) {
    assert(name);
    if (name in this.prefetchedParameters) return this.prefetchedParameters[name];
    switch (name) {
      case this.TEXTURE_BINDING_2D: {
        return bindings.texture2D;
      }
      case this.ARRAY_BUFFER_BINDING: {
        return bindings.arrayBuffer;
      }
      case this.ELEMENT_ARRAY_BUFFER_BINDING: {
        return bindings.elementArrayBuffer;
      }
      case this.CURRENT_PROGRAM: {
        return bindings.program;
      }
      case this.FRAMEBUFFER_BINDING: {
        return bindings.framebuffer;
      }
      case this.ACTIVE_TEXTURE: {
        return bindings.activeTexture;
      }
      case this.GENERATE_MIPMAP_HINT: {
        return bindings.generateMipmapHint;
      }
      case this.BLEND_SRC_RGB: {
        return bindings.blendSrcRGB;
      }
      case this.BLEND_SRC_ALPHA: {
        return bindings.blendSrcAlpha;
      }
      case this.BLEND_DST_RGB: {
        return bindings.blendDstRGB;
      }
      case this.BLEND_DST_ALPHA: {
        return bindings.blendDstAlpha;
      }
      case this.BLEND_EQUATION_RGB: {
        return bindings.blendEquationRGB;
      }
      case this.BLEND_EQUATION_ALPHA: {
        return bindings.blendEquationAlpha;
      }
      default: {
        if (bindings.enabledState[name] !== undefined) return bindings.enabledState[name];
        throw 'TODO: get parameter ' + name + ' : ' + revname(name);
      }
    }
  };
  this.getExtension = function(name) {
    var i = this.prefetchedExtensions.indexOf(name);
    if (i < 0) return null;
    commandBuffer.push(1, name);
    switch (name) {
      case 'EXT_texture_filter_anisotropic': {
        return {
          TEXTURE_MAX_ANISOTROPY_EXT:     0x84FE,
          MAX_TEXTURE_MAX_ANISOTROPY_EXT: 0x84FF
        };
      }
      case 'WEBGL_draw_buffers': {
        return {
          COLOR_ATTACHMENT0_WEBGL     : 0x8CE0,
          COLOR_ATTACHMENT1_WEBGL     : 0x8CE1,
          COLOR_ATTACHMENT2_WEBGL     : 0x8CE2,
          COLOR_ATTACHMENT3_WEBGL     : 0x8CE3,
          COLOR_ATTACHMENT4_WEBGL     : 0x8CE4,
          COLOR_ATTACHMENT5_WEBGL     : 0x8CE5,
          COLOR_ATTACHMENT6_WEBGL     : 0x8CE6,
          COLOR_ATTACHMENT7_WEBGL     : 0x8CE7,
          COLOR_ATTACHMENT8_WEBGL     : 0x8CE8,
          COLOR_ATTACHMENT9_WEBGL     : 0x8CE9,
          COLOR_ATTACHMENT10_WEBGL    : 0x8CEA,
          COLOR_ATTACHMENT11_WEBGL    : 0x8CEB,
          COLOR_ATTACHMENT12_WEBGL    : 0x8CEC,
          COLOR_ATTACHMENT13_WEBGL    : 0x8CED,
          COLOR_ATTACHMENT14_WEBGL    : 0x8CEE,
          COLOR_ATTACHMENT15_WEBGL    : 0x8CEF,

          DRAW_BUFFER0_WEBGL          : 0x8825,
          DRAW_BUFFER1_WEBGL          : 0x8826,
          DRAW_BUFFER2_WEBGL          : 0x8827,
          DRAW_BUFFER3_WEBGL          : 0x8828,
          DRAW_BUFFER4_WEBGL          : 0x8829,
          DRAW_BUFFER5_WEBGL          : 0x882A,
          DRAW_BUFFER6_WEBGL          : 0x882B,
          DRAW_BUFFER7_WEBGL          : 0x882C,
          DRAW_BUFFER8_WEBGL          : 0x882D,
          DRAW_BUFFER9_WEBGL          : 0x882E,
          DRAW_BUFFER10_WEBGL         : 0x882F,
          DRAW_BUFFER11_WEBGL         : 0x8830,
          DRAW_BUFFER12_WEBGL         : 0x8831,
          DRAW_BUFFER13_WEBGL         : 0x8832,
          DRAW_BUFFER14_WEBGL         : 0x8833,
          DRAW_BUFFER15_WEBGL         : 0x8834,

          MAX_COLOR_ATTACHMENTS_WEBGL : 0x8CDF,
          MAX_DRAW_BUFFERS_WEBGL      : 0x8824,

          drawBuffersWEBGL: function(buffers) {
            that.drawBuffersWEBGL(buffers);
          }
        };
      }
      case 'OES_standard_derivatives': {
        return { FRAGMENT_SHADER_DERIVATIVE_HINT_OES: 0x8B8B };
      }
    };
    return true; // TODO: return an object here
  };
  this.getSupportedExtensions = function() {
    return this.prefetchedExtensions;
  };
  this.getShaderPrecisionFormat = function(shaderType, precisionType) {
    return this.prefetchedPrecisions[shaderType][precisionType];
  };
  this.enable = function(cap) {
    commandBuffer.push(2, cap);
    bindings.enabledState[cap] = true;
  };
  this.isEnabled = function(cap) {
    return bindings.enabledState[cap];
  };
  this.disable = function(cap) {
    commandBuffer.push(3, cap);
    bindings.enabledState[cap] = false;
  };
  this.clear = function(mask) {
    commandBuffer.push(4, mask);
  };
  this.clearColor = function(r, g, b, a) {
    commandBuffer.push(5, r, g, b, a);
  };
  this.createShader = function(type) {
    var id = nextId++;
    commandBuffer.push(6, type, id);
    return { id: id, what: 'shader', type: type };
  };
  this.deleteShader = function(shader) {
    if (!shader) return;
    commandBuffer.push(7, shader.id);
  };
  this.shaderSource = function(shader, source) {
    shader.source = source;
    commandBuffer.push(8, shader.id, source);
  };
  this.compileShader = function(shader) {
    commandBuffer.push(9, shader.id);
  };
  this.getShaderInfoLog = function(shader) {
    return ''; // optimistic assumption of success; no proxying
  };
  this.createProgram = function() {
    var id = nextId++;
    commandBuffer.push(10, id);
    return new WebGLProgram(id);
  };
  this.deleteProgram = function(program) {
    if (!program) return;
    commandBuffer.push(11, program.id);
  };
  this.attachShader = function(program, shader) {
    program.shaders.push(shader);
    commandBuffer.push(12, program.id, shader.id);
  };
  this.bindAttribLocation = function(program, index, name) {
    program.nextAttributes[name] = { what: 'attribute', name: name, size: -1, location: index, type: '?' }; // fill in size, type later
    program.nextAttributeVec[index] = name;
    commandBuffer.push(13, program.id, index, name);
  };
  this.getAttribLocation = function(program, name) {
    // all existing attribs are cached locally
    if (name in program.attributes) return program.attributes[name].location;
    return -1;
  };
  this.linkProgram = function(program) {
    // parse shader sources
    function getTypeId(text) {
      switch (text) {
        case 'bool': return that.BOOL;
        case 'int': return that.INT;
        case 'uint': return that.UNSIGNED_INT;
        case 'float': return that.FLOAT;
        case 'vec2': return that.FLOAT_VEC2;
        case 'vec3': return that.FLOAT_VEC3;
        case 'vec4': return that.FLOAT_VEC4;
        case 'ivec2': return that.INT_VEC2;
        case 'ivec3': return that.INT_VEC3;
        case 'ivec4': return that.INT_VEC4;
        case 'bvec2': return that.BOOL_VEC2;
        case 'bvec3': return that.BOOL_VEC3;
        case 'bvec4': return that.BOOL_VEC4;
        case 'mat2': return that.FLOAT_MAT2;
        case 'mat3': return that.FLOAT_MAT3;
        case 'mat4': return that.FLOAT_MAT4;
        case 'sampler2D': return that.SAMPLER_2D;
        case 'sampler3D': return that.SAMPLER_3D;
        case 'samplerCube': return that.SAMPLER_CUBE;
        default: throw 'not yet recognized type text: ' + text;
      }
    }
    function parseElementType(shader, type, obj, vec) {
      var source = shader.source;
      source = source.replace(/\n/g, '|\n'); // barrier between lines, to make regexing easier
      var newItems = source.match(new RegExp(type + '\\s+\\w+\\s+[\\w,\\s\[\\]]+;', 'g'));
      if (!newItems) return;
      newItems.forEach(function(item) {
        var m = new RegExp(type + '\\s+(\\w+)\\s+([\\w,\\s\[\\]]+);').exec(item);
        assert(m);
        m[2].split(',').map(function(name) { name = name.trim(); return name.search(/\s/) >= 0 ? '' : name }).filter(function(name) { return !!name }).forEach(function(name) {
          var size = 1;
          var open = name.indexOf('[');
          var fullname = name;
          if (open >= 0) {
            var close = name.indexOf(']');
            size = parseInt(name.substring(open+1, close));
            name = name.substr(0, open);
            fullname = name + '[0]';
          }
          if (!obj[name]) {
            obj[name] = { what: type, name: fullname, size: size, location: -1, type: getTypeId(m[1]) };
            if (vec) vec.push(name);
          }
        });
      });
    }

    program.uniforms = {};
    program.uniformVec = [];

    program.attributes = program.nextAttributes;
    program.attributeVec = program.nextAttributeVec;
    program.nextAttributes = {};
    program.nextAttributeVec = [];

    var existingAttributes = {};

    program.shaders.forEach(function(shader) {
      parseElementType(shader, 'uniform', program.uniforms, program.uniformVec);
      parseElementType(shader, 'attribute', existingAttributes, null);
    });

    // bind not-yet bound attributes
    for (var attr in existingAttributes) {
      if (!(attr in program.attributes)) {
        var index = program.attributeVec.length;
        program.attributes[attr] = { what: 'attribute', name: attr, size: -1, location: index, type: '?' }; // fill in size, type later
        program.attributeVec[index] = attr;
        commandBuffer.push(13, program.id, index, attr); // do a bindAttribLocation as well, so this takes effect in the link we are about to do
      }
      program.attributes[attr].size = existingAttributes[attr].size;
      program.attributes[attr].type = existingAttributes[attr].type;
    }

    commandBuffer.push(14, program.id);
  };
  this.getProgramParameter = function(program, name) {
    switch (name) {
      case this.ACTIVE_UNIFORMS: return program.uniformVec.length;
      case this.ACTIVE_ATTRIBUTES: return program.attributeVec.length;
      case this.LINK_STATUS: {
        // optimisticaly return success; client will abort on an actual error. we assume an error-free async workflow
        commandBuffer.push(15, program.id, name);
        return true;
      }
      default: throw 'bad getProgramParameter ' + revname(name);
    }
  };
  this.getActiveAttrib = function(program, index) {
    var name = program.attributeVec[index];
    if (!name) return null;
    return program.attributes[name];
  };
  this.getActiveUniform = function(program, index) {
    var name = program.uniformVec[index];
    if (!name) return null;
    return program.uniforms[name];
  };
  this.getUniformLocation = function(program, name) {
    var fullname = name;
    var index = -1;
    var open = name.indexOf('[');
    if (open >= 0) {
      var close = name.indexOf(']');
      index = parseInt(name.substring(open+1, close));
      name = name.substr(0, open);
    }
    if (!(name in program.uniforms)) return null;
    var id = nextId++;
    commandBuffer.push(16, program.id, fullname, id);
    return { what: 'location', uniform: program.uniforms[name], id: id, index: index };
  };
  this.getProgramInfoLog = function(shader) {
    return ''; // optimistic assumption of success; no proxying
  };
  this.useProgram = function(program) {
    commandBuffer.push(17, program ? program.id : 0);
    bindings.program = program;
  };
  this.uniform1i = function(location, data) {
    if (!location) return;
    commandBuffer.push(18, location.id, data);
  };
  this.uniform1f = function(location, data) {
    if (!location) return;
    commandBuffer.push(19, location.id, data);
  };
  this.uniform3fv = function(location, data) {
    if (!location) return;
    commandBuffer.push(20, location.id, new Float32Array(data));
  };
  this.uniform4f = function(location, x, y, z, w) {
    if (!location) return;
    commandBuffer.push(21, location.id, new Float32Array([x, y, z, w]));
  };
  this.uniform4fv = function(location, data) {
    if (!location) return;
    commandBuffer.push(21, location.id, new Float32Array(data));
  };
  this.uniformMatrix4fv = function(location, transpose, data) {
    if (!location) return;
    commandBuffer.push(22, location.id, transpose, new Float32Array(data));
  };
  this.vertexAttrib4fv = function(index, values) {
    commandBuffer.push(23, index, new Float32Array(values));
  };
  this.createBuffer = function() {
    var id = nextId++;
    commandBuffer.push(24, id);
    return new WebGLBuffer(id);
  };
  this.deleteBuffer = function(buffer) {
    if (!buffer) return;
    commandBuffer.push(25, buffer.id);
  };
  this.bindBuffer = function(target, buffer) {
    commandBuffer.push(26, target, buffer ? buffer.id : 0);
    switch (target) {
      case this.ARRAY_BUFFER_BINDING: {
        bindings.arrayBuffer = buffer;
        break;
      }
      case this.ELEMENT_ARRAY_BUFFER_BINDING: {
        bindings.elementArrayBuffer = buffer;
        break;
      }
    }
  };
  function duplicate(something) {
    // clone data properly: handles numbers, null, typed arrays, js arrays and array buffers
    if (!something || typeof something === 'number') return something;
    if (something.slice) return something.slice(0); // ArrayBuffer or js array
    return new something.constructor(something); // typed array
  }
  this.bufferData = function(target, something, usage) {
    commandBuffer.push(27, target, duplicate(something), usage);
  };
  this.bufferSubData = function(target, offset, something) {
    commandBuffer.push(28, target, offset, duplicate(something));
  };
  this.viewport = function(x, y, w, h) {
    commandBuffer.push(29, x, y, w, h);
  };
  this.vertexAttribPointer = function(index, size, type, normalized, stride, offset) {
    commandBuffer.push(30, index, size, type, normalized, stride, offset);
  };
  this.enableVertexAttribArray = function(index) {
    commandBuffer.push(31, index);
  };
  this.disableVertexAttribArray = function(index) {
    commandBuffer.push(32, index);
  };
  this.drawArrays = function(mode, first, count) {
    commandBuffer.push(33, mode, first, count);
  };
  this.drawElements = function(mode, count, type, offset) {
    commandBuffer.push(34, mode, count, type, offset);
  };
  this.getError = function() {
    // optimisticaly return success; client will abort on an actual error. we assume an error-free async workflow
    commandBuffer.push(35);
    return this.NO_ERROR;
  };
  this.createTexture = function() {
    var id = nextId++;
    commandBuffer.push(36, id);
    return new WebGLTexture(id);
  };
  this.deleteTexture = function(texture) {
    if (!texture) return;
    commandBuffer.push(37, texture.id);
    texture.id = 0;
  };
  this.isTexture = function(texture) {
    return texture && texture.what === 'texture' && texture.id > 0 && texture.binding;
  };
  this.bindTexture = function(target, texture) {
    switch (target) {
      case that.TEXTURE_2D: {
        bindings.texture2D = texture;
        break;
      }
    }
    if (texture) texture.binding = target;
    commandBuffer.push(38, target, texture ? texture.id : 0);
  };
  this.texParameteri = function(target, pname, param) {
    commandBuffer.push(39, target, pname, param);
  };
  this.texImage2D = function(target, level, internalformat, width, height, border, format, type, pixels) {
    if (pixels === undefined) {
      format = width; // width, height, border do not exist in the shorter overload
      type = height;
      pixels = border;
      assert(pixels instanceof Image);
      assert(internalformat === format && format === this.RGBA); // HTML Images are RGBA, 8-bit
      assert(type === this.UNSIGNED_BYTE);
      var data = pixels.data;
      width = data.width;
      height = data.height;
      border = 0;
      pixels = new Uint8Array(data.data); // XXX transform from clamped to normal, could have been done in duplicate
    }
    commandBuffer.push(40, target, level, internalformat, width, height, border, format, type, duplicate(pixels));
  };
  this.compressedTexImage2D = function(target, level, internalformat, width, height, border, pixels) {
    commandBuffer.push(41, target, level, internalformat, width, height, border, duplicate(pixels));
  };
  this.activeTexture = function(texture) {
    commandBuffer.push(42, texture);
    bindings.activeTexture = texture;
  };
  this.getShaderParameter = function(shader, pname) {
    switch (pname) {
      case this.SHADER_TYPE: return shader.type;
      case this.COMPILE_STATUS: {
        // optimisticaly return success; client will abort on an actual error. we assume an error-free async workflow
        commandBuffer.push(43, shader.id, pname);
        return true;
      }
      default: throw 'unsupported getShaderParameter ' + pname;
    }
  };
  this.clearDepth = function(depth) {
    commandBuffer.push(44, depth);
  };
  this.depthFunc = function(depth) {
    commandBuffer.push(45, depth);
  };
  this.frontFace = function(depth) {
    commandBuffer.push(46, depth);
  };
  this.cullFace = function(depth) {
    commandBuffer.push(47, depth);
  };
  this.readPixels = function(depth) {
    abort('readPixels is impossible, we are async GL');
  };
  this.pixelStorei = function(pname, param) {
    commandBuffer.push(48, pname, param);
  };
  this.depthMask = function(flag) {
    commandBuffer.push(49, flag);
  };
  this.depthRange = function(near, far) {
    commandBuffer.push(50, near, far);
  };
  this.blendFunc = function(sfactor, dfactor) {
    commandBuffer.push(51, sfactor, dfactor);
    bindings.blendSrcRGB = bindings.blendSrcAlpha = sfactor;
    bindings.blendDstRGB = bindings.blendDstAlpha = dfactor;
  };
  this.scissor = function(x, y, width, height) {
    commandBuffer.push(52, x, y, width, height);
  };
  this.colorMask = function(red, green, blue, alpha) {
    commandBuffer.push(53, red, green, blue, alpha);
  };
  this.lineWidth = function(width) {
    commandBuffer.push(54, width);
  };
  this.createFramebuffer = function() {
    var id = nextId++;
    commandBuffer.push(55, id);
    return new WebGLFramebuffer(id);
  };
  this.deleteFramebuffer = function(framebuffer) {
    if (!framebuffer) return;
    commandBuffer.push(56, framebuffer.id);
  };
  this.bindFramebuffer = function(target, framebuffer) {
    commandBuffer.push(57, target, framebuffer ? framebuffer.id : 0);
    bindings.framebuffer = framebuffer;
  };
  this.framebufferTexture2D = function(target, attachment, textarget, texture, level) {
    commandBuffer.push(58, target, attachment, textarget, texture ? texture.id : 0, level);
  };
  this.checkFramebufferStatus = function(target) {
    return this.FRAMEBUFFER_COMPLETE; // XXX totally wrong
  };
  this.createRenderbuffer = function() {
    var id = nextId++;
    commandBuffer.push(59, id);
    return new WebGLRenderbuffer(id);
  };
  this.deleteRenderbuffer = function(renderbuffer) {
    if (!renderbuffer) return;
    commandBuffer.push(60, renderbuffer.id);
  };
  this.bindRenderbuffer = function(target, renderbuffer) {
    commandBuffer.push(61, target, renderbuffer ? renderbuffer.id : 0);
  };
  this.renderbufferStorage = function(target, internalformat, width, height) {
    commandBuffer.push(62, target, internalformat, width, height);
  };
  this.framebufferRenderbuffer = function(target, attachment, renderbuffertarget, renderbuffer) {
    commandBuffer.push(63, target, attachment, renderbuffertarget, renderbuffer ? renderbuffer.id : 0);
  };
  this.debugPrint = function(text) { // useful to interleave debug output properly with client GL commands
    commandBuffer.push(64, text);
  };
  this.hint = function(target, mode) {
    commandBuffer.push(65, target, mode);
    if (target == this.GENERATE_MIPMAP_HINT) bindings.generateMipmapHint = mode;
  };
  this.blendEquation = function(mode) {
    commandBuffer.push(66, mode);
    bindings.blendEquationRGB = bindings.blendEquationAlpha = mode;
  };
  this.generateMipmap = function(target) {
    commandBuffer.push(67, target);
  };
  this.uniformMatrix3fv = function(location, transpose, data) {
    if (!location) return;
    commandBuffer.push(68, location.id, transpose, new Float32Array(data));
  };
  this.stencilMask = function(mask) {
    commandBuffer.push(69, mask);
  };
  this.clearStencil = function(s) {
    commandBuffer.push(70, s);
  };
  this.texSubImage2D = function(target, level, xoffset, yoffset, width, height, format, type, pixels) {
    if (pixels === undefined) {
      // shorter overload:      target, level, xoffset, yoffset, format,  type, pixels
      var formatTemp = format;
      format = width;
      type = height;
      pixels = formatTemp;
      assert(pixels instanceof Image);
      assert(format === this.RGBA); // HTML Images are RGBA, 8-bit
      assert(type === this.UNSIGNED_BYTE);
      var data = pixels.data;
      width = data.width;
      height = data.height;
      pixels = new Uint8Array(data.data); // XXX transform from clamped to normal, could have been done in duplicate
    }
    commandBuffer.push(71, target, level, xoffset, yoffset, width, height, format, type, duplicate(pixels));
  };
  this.uniform3f = function(location, x, y, z) {
    if (!location) return;
    commandBuffer.push(72, location.id, x, y, z);
  };
  this.blendFuncSeparate = function(srcRGB, dstRGB, srcAlpha, dstAlpha) {
    commandBuffer.push(73, srcRGB, dstRGB, srcAlpha, dstAlpha);
    bindings.blendSrcRGB = srcRGB;
    bindings.blendSrcAlpha = srcAlpha;
    bindings.blendDstRGB = dstRGB;
    bindings.blendDstAlpha = dstAlpha;
  }
  this.uniform2fv = function(location, data) {
    if (!location) return;
    commandBuffer.push(74, location.id, new Float32Array(data));
  };
  this.texParameterf = function(target, pname, param) {
    commandBuffer.push(75, target, pname, param);
  };
  this.isContextLost = function() {
    // optimisticaly return that everything is ok; client will abort on an actual context loss. we assume an error-free async workflow
    commandBuffer.push(76);
    return false;
  };
  this.isProgram = function(program) {
    return program && program.what === 'program';
  };
  this.blendEquationSeparate = function(rgb, alpha) {
    commandBuffer.push(77, rgb, alpha);
    bindings.blendEquationRGB = rgb;
    bindings.blendEquationAlpha = alpha;
  };
  this.stencilFuncSeparate = function(face, func, ref, mask) {
    commandBuffer.push(78, face, func, ref, mask);
  };
  this.stencilOpSeparate = function(face, fail, zfail, zpass) {
    commandBuffer.push(79, face, fail, zfail, zpass);
  };
  this.drawBuffersWEBGL = function(buffers) {
    commandBuffer.push(80, buffers);
  };
  this.uniform1iv = function(location, data) {
    if (!location) return;
    commandBuffer.push(81, location.id, new Int32Array(data));
  };
  this.uniform1fv = function(location, data) {
    if (!location) return;
    commandBuffer.push(82, location.id, new Float32Array(data));
  };

  // Setup

  var theoreticalTracker = new FPSTracker('server (theoretical)');
  var throttledTracker = new FPSTracker('server (client-throttled)');

  function preRAF() {
    //theoreticalTracker.tick();
    // if too many frames in queue, skip a main loop iter
    if (Math.abs(frameId - clientFrameId) >= 4) {
      return false;
    }
    //throttledTracker.tick();
  }

  var postRAFed = false;

  function postRAF() {
    if (commandBuffer.length > 0) {
      postMessage({ target: 'gl', op: 'render', commandBuffer: commandBuffer });
      commandBuffer = [];
    }
    postRAFed = true;
  }

  assert(!Browser.doSwapBuffers);
  Browser.doSwapBuffers = postRAF;

  var trueRAF = window.requestAnimationFrame;
  window.requestAnimationFrame = function(func) {
    trueRAF(function() {
      if (preRAF() === false) {
        window.requestAnimationFrame(func); // skip this frame, do it later
        return;
      }
      postRAFed = false;
      func();
      if (!postRAFed) { // if we already posted this frame (e.g. from doSwapBuffers) do not post again
        postRAF();
      }
    });
  }

}

// share prefetched data among all instances

WebGLWorker.prototype.prefetchedParameters = {};
WebGLWorker.prototype.prefetchedExtensions = {};
WebGLWorker.prototype.prefetchedPrecisions = {};

