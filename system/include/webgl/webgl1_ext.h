#pragma once

/* This header webgl1_ext.h provides static linkage entry points to all WebGL extensions that
  the Khronos WebGL registry adds on top of the WebGL 1 API.

  In Emscripten, all GL extension function entry points are provided via static linkage.
  For best WebGL performance, call the statically linked gl*() functions in this header
  instead of using a dynamic function pointers via the glGetProcAddress() function.

  Include this header instead of the headers GLES2/gl2ext.h or GL/glext.h if you are
  developing a WebGL renderer as a first tier platform, and want to get "fail fast"
  compiler errors of GL symbols that are not supported on WebGL.

  Other features:
  - If you want to use one of the WebGL specific extensions that do not exist in
    GLES or desktop GL (such as WEBGL_lose_context or WEBGL_debug_shaders), include
    this header to get the function declarations and defines.

  - Unlike GLES and desktop GL, in WebGL one must explicitly enable an extension
    before using it. See below in the section of each extension for instructions
    on how to enable it, or link with -sGL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1
    to automatically enable all non-debugging related WebGL extensions at startup.

  - If you are targeting multiple Emscripten compiler versions (e.g. a rendering
    library middleware), you can query whether static linkage to a particular
    extension is provided, by including this header and then checking

        #if EMSCRIPTEN_GL_WEBGL_polygon_mode
            // we can call glPolygonModeWEBGL() function
        #endif

  - To disable a particular WebGL extension from being declared in this header,
    you can add e.g.
       #define EMSCRIPTEN_GL_OES_texture_float 0
    before including this header.

  - For technical reasons, each function declaration comes in two variants:
      a glFoo() declaration, and a second emscripten_glFoo() copy.
    The emscripten_glFoo() variants exist for internal *GetProcAddress() and
    Emscripten -sOFFSCREEN_FRAMEBUFFER=1 features linkage purposes, and should
    be ignored by end users.
*/
#include "webgl1.h"
#include <emscripten/html5.h>

// 1. https://www.khronos.org/registry/webgl/extensions/OES_texture_float/
#ifndef EMSCRIPTEN_GL_OES_texture_float
#define EMSCRIPTEN_GL_OES_texture_float 1
// To enable: call emscripten_webgl_enable_extension(ctx, "OES_texture_float");
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_OES_texture_float */

// 2. https://www.khronos.org/registry/webgl/extensions/OES_texture_half_float/
#ifndef EMSCRIPTEN_GL_OES_texture_half_float
#define EMSCRIPTEN_GL_OES_texture_half_float 1
// To enable: call emscripten_webgl_enable_extension(ctx, "OES_texture_half_float");
#define GL_HALF_FLOAT_OES 0x8D61
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_OES_texture_half_float */

// 3. https://www.khronos.org/registry/webgl/extensions/WEBGL_lose_context/
#ifndef EMSCRIPTEN_GL_WEBGL_lose_context
//#define EMSCRIPTEN_GL_WEBGL_lose_context 1
// TODO:
//WEBGL_APICALL EMSCRIPTEN_RESULT GL_APIENTRY emscripten_webgl_loseContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE contextHandle);
//WEBGL_APICALL EMSCRIPTEN_RESULT GL_APIENTRY emscripten_webgl_restoreContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE contextHandle);
#endif /* EMSCRIPTEN_GL_WEBGL_lose_context */

// 4. https://www.khronos.org/registry/webgl/extensions/OES_standard_derivatives/
#ifndef EMSCRIPTEN_GL_OES_standard_derivatives
#define EMSCRIPTEN_GL_OES_standard_derivatives 1
// To enable: call emscripten_webgl_enable_extension(ctx, "OES_standard_derivatives");
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES 0x8B8B
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_OES_standard_derivatives */

// 5. https://www.khronos.org/registry/webgl/extensions/OES_vertex_array_object/
#ifndef EMSCRIPTEN_GL_OES_vertex_array_object
#define EMSCRIPTEN_GL_OES_vertex_array_object 1
// To enable: call
bool emscripten_webgl_enable_OES_vertex_array_object(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);
// or link with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=1 and
// call emscripten_webgl_enable_extension(ctx, "OES_vertex_array_object");
#define GL_VERTEX_ARRAY_BINDING_OES 0x85B5
WEBGL_APICALL void GL_APIENTRY emscripten_glBindVertexArrayOES(GLuint array);
WEBGL_APICALL void GL_APIENTRY emscripten_glDeleteVertexArraysOES(GLsizei n, const GLuint *arrays __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY emscripten_glGenVertexArraysOES(GLsizei n, GLuint *arrays __attribute__((nonnull)));
WEBGL_APICALL GLboolean GL_APIENTRY emscripten_glIsVertexArrayOES(GLuint array);
WEBGL_APICALL void GL_APIENTRY glBindVertexArrayOES(GLuint array);
WEBGL_APICALL void GL_APIENTRY glDeleteVertexArraysOES(GLsizei n, const GLuint *arrays __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY glGenVertexArraysOES(GLsizei n, GLuint *arrays __attribute__((nonnull)));
WEBGL_APICALL GLboolean GL_APIENTRY glIsVertexArrayOES(GLuint array);
#endif /* EMSCRIPTEN_GL_OES_vertex_array_object */

// 6. https://www.khronos.org/registry/webgl/extensions/WEBGL_debug_renderer_info/
#ifndef EMSCRIPTEN_GL_WEBGL_debug_renderer_info
#define EMSCRIPTEN_GL_WEBGL_debug_renderer_info 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_debug_renderer_info");
#define GL_UNMASKED_VENDOR_WEBGL 0x9245
#define GL_UNMASKED_RENDERER_WEBGL 0x9246
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_WEBGL_debug_renderer_info */

// 7. https://www.khronos.org/registry/webgl/extensions/WEBGL_debug_shaders/
#ifndef EMSCRIPTEN_GL_WEBGL_debug_shaders
#define EMSCRIPTEN_GL_WEBGL_debug_shaders 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_debug_shaders");
//TODO:
//WEBGL_APICALL void GL_APIENTRY emscripten_webgl_getTranslatedShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length __attribute__((nonnull)), GLchar *source __attribute__((nonnull)));
#endif /* EMSCRIPTEN_GL_WEBGL_debug_shaders */

// 8. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_s3tc/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_compressed_texture_s3tc");
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc */

// 9. https://www.khronos.org/registry/webgl/extensions/WEBGL_depth_texture/
#ifndef EMSCRIPTEN_GL_WEBGL_depth_texture
#define EMSCRIPTEN_GL_WEBGL_depth_texture 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_depth_texture");
#define GL_UNSIGNED_INT_24_8_WEBGL 0x84FA
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_WEBGL_depth_texture */

// 10. https://www.khronos.org/registry/webgl/extensions/OES_element_index_uint/
#ifndef EMSCRIPTEN_GL_OES_element_index_uint
#define EMSCRIPTEN_GL_OES_element_index_uint 1
// To enable: call emscripten_webgl_enable_extension(ctx, "OES_element_index_uint");
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_OES_element_index_uint */

// 11. https://www.khronos.org/registry/webgl/extensions/EXT_texture_filter_anisotropic/
#ifndef EMSCRIPTEN_GL_EXT_texture_filter_anisotropic
#define EMSCRIPTEN_GL_EXT_texture_filter_anisotropic 1
// To enable: call emscripten_webgl_enable_extension(ctx, "EXT_texture_filter_anisotropic");
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_EXT_texture_filter_anisotropic */

// 16. https://www.khronos.org/registry/webgl/extensions/EXT_frag_depth/
#ifndef EMSCRIPTEN_GL_EXT_frag_depth
#define EMSCRIPTEN_GL_EXT_frag_depth 1
// To enable: call emscripten_webgl_enable_extension(ctx, "EXT_frag_depth");
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_EXT_frag_depth */

// 18. https://www.khronos.org/registry/webgl/extensions/WEBGL_draw_buffers/
#ifndef EMSCRIPTEN_GL_WEBGL_draw_buffers
#define EMSCRIPTEN_GL_WEBGL_draw_buffers 1
// To enable: call 
bool emscripten_webgl_enable_WEBGL_draw_buffers(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);
// or link with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=1 and
// call emscripten_webgl_enable_extension(ctx, "WEBGL_draw_buffers");
#define GL_COLOR_ATTACHMENT0_WEBGL 0x8CE0
#define GL_COLOR_ATTACHMENT1_WEBGL 0x8CE1
#define GL_COLOR_ATTACHMENT2_WEBGL 0x8CE2
#define GL_COLOR_ATTACHMENT3_WEBGL 0x8CE3
#define GL_COLOR_ATTACHMENT4_WEBGL 0x8CE4
#define GL_COLOR_ATTACHMENT5_WEBGL 0x8CE5
#define GL_COLOR_ATTACHMENT6_WEBGL 0x8CE6
#define GL_COLOR_ATTACHMENT7_WEBGL 0x8CE7
#define GL_COLOR_ATTACHMENT8_WEBGL 0x8CE8
#define GL_COLOR_ATTACHMENT9_WEBGL 0x8CE9
#define GL_COLOR_ATTACHMENT10_WEBGL 0x8CEA
#define GL_COLOR_ATTACHMENT11_WEBGL 0x8CEB
#define GL_COLOR_ATTACHMENT12_WEBGL 0x8CEC
#define GL_COLOR_ATTACHMENT13_WEBGL 0x8CED
#define GL_COLOR_ATTACHMENT14_WEBGL 0x8CEE
#define GL_COLOR_ATTACHMENT15_WEBGL 0x8CEF
#define GL_DRAW_BUFFER0_WEBGL 0x8825
#define GL_DRAW_BUFFER1_WEBGL 0x8826
#define GL_DRAW_BUFFER2_WEBGL 0x8827
#define GL_DRAW_BUFFER3_WEBGL 0x8828
#define GL_DRAW_BUFFER4_WEBGL 0x8829
#define GL_DRAW_BUFFER5_WEBGL 0x882A
#define GL_DRAW_BUFFER6_WEBGL 0x882B
#define GL_DRAW_BUFFER7_WEBGL 0x882C
#define GL_DRAW_BUFFER8_WEBGL 0x882D
#define GL_DRAW_BUFFER9_WEBGL 0x882E
#define GL_DRAW_BUFFER10_WEBGL 0x882F
#define GL_DRAW_BUFFER11_WEBGL 0x8830
#define GL_DRAW_BUFFER12_WEBGL 0x8831
#define GL_DRAW_BUFFER13_WEBGL 0x8832
#define GL_DRAW_BUFFER14_WEBGL 0x8833
#define GL_DRAW_BUFFER15_WEBGL 0x8834
#define GL_MAX_COLOR_ATTACHMENTS_WEBGL 0x8CDF
#define GL_MAX_DRAW_BUFFERS_WEBGL 0x8824
WEBGL_APICALL void GL_APIENTRY emscripten_glDrawBuffersWEBGL(GLsizei n, const GLenum *buffers);
WEBGL_APICALL void GL_APIENTRY glDrawBuffersWEBGL(GLsizei n, const GLenum *buffers);
#endif /* EMSCRIPTEN_GL_WEBGL_draw_buffers */

// 19. https://www.khronos.org/registry/webgl/extensions/ANGLE_instanced_arrays/
#ifndef EMSCRIPTEN_GL_ANGLE_instanced_arrays
#define EMSCRIPTEN_GL_ANGLE_instanced_arrays 1
// To enable: call
bool emscripten_webgl_enable_ANGLE_instanced_arrays(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);
// or link with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=1 and
// call emscripten_webgl_enable_extension(ctx, "ANGLE_instanced_arrays");
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE 0x88FE
WEBGL_APICALL void GL_APIENTRY emscripten_glDrawArraysInstancedANGLE(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
WEBGL_APICALL void GL_APIENTRY emscripten_glDrawElementsInstancedANGLE(GLenum mode, GLsizei count, GLenum type, GLintptr offset, GLsizei primcount);
WEBGL_APICALL void GL_APIENTRY emscripten_glVertexAttribDivisorANGLE(GLuint index, GLuint divisor);
WEBGL_APICALL void GL_APIENTRY glDrawArraysInstancedANGLE(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
WEBGL_APICALL void GL_APIENTRY glDrawElementsInstancedANGLE(GLenum mode, GLsizei count, GLenum type, GLintptr offset, GLsizei primcount);
WEBGL_APICALL void GL_APIENTRY glVertexAttribDivisorANGLE(GLuint index, GLuint divisor);
#endif /* EMSCRIPTEN_GL_ANGLE_instanced_arrays */

// 20. https://www.khronos.org/registry/webgl/extensions/OES_texture_float_linear/
#ifndef EMSCRIPTEN_GL_OES_texture_float_linear
#define EMSCRIPTEN_GL_OES_texture_float_linear 1
// To enable: call emscripten_webgl_enable_extension(ctx, "OES_texture_float_linear");
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_OES_texture_float_linear */

// 21. https://www.khronos.org/registry/webgl/extensions/OES_texture_half_float_linear/
#ifndef EMSCRIPTEN_GL_OES_texture_half_float_linear
#define EMSCRIPTEN_GL_OES_texture_half_float_linear 1
// To enable: call emscripten_webgl_enable_extension(ctx, "OES_texture_half_float_linear");
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_OES_texture_half_float_linear */

// 25. https://www.khronos.org/registry/webgl/extensions/EXT_blend_minmax/
#ifndef EMSCRIPTEN_GL_EXT_blend_minmax
#define EMSCRIPTEN_GL_EXT_blend_minmax 1
// To enable: call emscripten_webgl_enable_extension(ctx, "EXT_blend_minmax");
#define GL_MIN_EXT 0x8007
#define GL_MAX_EXT 0x8008
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_EXT_blend_minmax */

// 27. https://www.khronos.org/registry/webgl/extensions/EXT_shader_texture_lod/
#ifndef EMSCRIPTEN_GL_EXT_shader_texture_lod
#define EMSCRIPTEN_GL_EXT_shader_texture_lod 1
// To enable: call emscripten_webgl_enable_extension(ctx, "EXT_shader_texture_lod");
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_EXT_shader_texture_lod */

// 13. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_pvrtc/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_pvrtc
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_pvrtc 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_compressed_texture_pvrtc");
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_pvrtc */

// 14. https://www.khronos.org/registry/webgl/extensions/EXT_color_buffer_half_float/
#ifndef EMSCRIPTEN_GL_EXT_color_buffer_half_float
#define EMSCRIPTEN_GL_EXT_color_buffer_half_float 1
// To enable: call emscripten_webgl_enable_extension(ctx, "EXT_color_buffer_half_float");
#define GL_RGBA16F_EXT 0x881A
#define GL_RGB16F_EXT 0x881B
#define GL_RG16F_EXT 0x822F
#define GL_R16F_EXT 0x822D
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT 0x8211
#define GL_UNSIGNED_NORMALIZED_EXT 0x8C17
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_EXT_color_buffer_half_float */

// 15. https://www.khronos.org/registry/webgl/extensions/WEBGL_color_buffer_float/
#ifndef EMSCRIPTEN_GL_WEBGL_color_buffer_float
#define EMSCRIPTEN_GL_WEBGL_color_buffer_float 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_color_buffer_float");
#define GL_RGBA32F_EXT 0x8814
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT 0x8211
#define GL_UNSIGNED_NORMALIZED_EXT 0x8C17
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_WEBGL_color_buffer_float */

// 17. https://www.khronos.org/registry/webgl/extensions/EXT_sRGB/
#ifndef EMSCRIPTEN_GL_EXT_sRGB
#define EMSCRIPTEN_GL_EXT_sRGB 1
// To enable: call emscripten_webgl_enable_extension(ctx, "EXT_sRGB");
#define GL_SRGB_EXT 0x8C40
#define GL_SRGB_ALPHA_EXT 0x8C42
#define GL_SRGB8_ALPHA8_EXT 0x8C43
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT 0x8210
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_EXT_sRGB */

// 24. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_etc1/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_etc1
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_etc1 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_compressed_texture_etc1");
#define GL_COMPRESSED_RGB_ETC1_WEBGL 0x8D64
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_etc1 */

// 26. https://www.khronos.org/registry/webgl/extensions/EXT_disjoint_timer_query/
#ifndef EMSCRIPTEN_GL_EXT_disjoint_timer_query
#define EMSCRIPTEN_GL_EXT_disjoint_timer_query 1
#define GL_QUERY_COUNTER_BITS_EXT 0x8864
#define GL_CURRENT_QUERY_EXT 0x8865
#define GL_QUERY_RESULT_EXT 0x8866
#define GL_QUERY_RESULT_AVAILABLE_EXT 0x8867
#define GL_TIME_ELAPSED_EXT 0x88BF
#define GL_TIMESTAMP_EXT 0x8E28
#define GL_GPU_DISJOINT_EXT 0x8FBB
WEBGL_APICALL void GL_APIENTRY emscripten_glGenQueriesEXT(GLsizei n, GLuint *ids __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY emscripten_glDeleteQueriesEXT(GLsizei n, const GLuint *ids __attribute__((nonnull)));
WEBGL_APICALL GLboolean GL_APIENTRY emscripten_glIsQueryEXT(GLuint id);
WEBGL_APICALL void GL_APIENTRY emscripten_glBeginQueryEXT(GLenum target, GLuint id);
WEBGL_APICALL void GL_APIENTRY emscripten_glEndQueryEXT(GLenum target);
WEBGL_APICALL void GL_APIENTRY emscripten_glQueryCounterEXT(GLuint id, GLenum target);
WEBGL_APICALL void GL_APIENTRY emscripten_glGetQueryivEXT(GLenum target, GLenum pname, GLint *params __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY emscripten_glGetQueryObjectivEXT(GLuint id, GLenum pname, GLint *params __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY emscripten_glGetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint *params __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY emscripten_glGetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64 *params __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY emscripten_glGetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64 *params __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY glGenQueriesEXT(GLsizei n, GLuint *ids __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY glDeleteQueriesEXT(GLsizei n, const GLuint *ids __attribute__((nonnull)));
WEBGL_APICALL GLboolean GL_APIENTRY glIsQueryEXT(GLuint id);
WEBGL_APICALL void GL_APIENTRY glBeginQueryEXT(GLenum target, GLuint id);
WEBGL_APICALL void GL_APIENTRY glEndQueryEXT(GLenum target);
WEBGL_APICALL void GL_APIENTRY glQueryCounterEXT(GLuint id, GLenum target);
WEBGL_APICALL void GL_APIENTRY glGetQueryivEXT(GLenum target, GLenum pname, GLint *params __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY glGetQueryObjectivEXT(GLuint id, GLenum pname, GLint *params __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY glGetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint *params __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY glGetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64 *params __attribute__((nonnull)));
WEBGL_APICALL void GL_APIENTRY glGetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64 *params __attribute__((nonnull)));
#endif /* EMSCRIPTEN_GL_EXT_disjoint_timer_query */

// 29. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_etc/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_etc
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_etc 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_compressed_texture_etc");
#define GL_COMPRESSED_R11_EAC 0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC 0x9271
#define GL_COMPRESSED_RG11_EAC 0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC 0x9273
#define GL_COMPRESSED_RGB8_ETC2  0x9274
#define GL_COMPRESSED_SRGB8_ETC2 0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_etc */

// 30. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_astc/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_astc
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_astc 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_compressed_texture_astc");
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR 0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR 0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR 0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR 0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR 0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR 0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR 0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR 0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR 0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR 0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR 0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR 0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR 0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR 0x93BD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR 0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR 0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR 0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR 0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR 0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR 0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR 0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR 0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR 0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR 0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR 0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR 0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR 0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR 0x93DD
//TODO:
//WEBGL_APICALL void GL_APIENTRY emscripten_webgl_getSupportedAstcProfiles(GLsizei bufSize, GLsizei *length __attribute__((nonnull)), GLchar *buf __attribute__((nonnull)));
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_astc */

// 31. https://www.khronos.org/registry/webgl/extensions/EXT_color_buffer_float/
#ifndef EMSCRIPTEN_GL_EXT_color_buffer_float
#define EMSCRIPTEN_GL_EXT_color_buffer_float 1
// To enable: call emscripten_webgl_enable_extension(ctx, "EXT_color_buffer_float");
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_EXT_color_buffer_float */

// 32. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_s3tc_srgb/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc_srgb
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc_srgb 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_compressed_texture_s3tc_srgb");
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc_srgb */

// 37. https://www.khronos.org/registry/webgl/extensions/KHR_parallel_shader_compile/
#ifndef EMSCRIPTEN_GL_KHR_parallel_shader_compile
#define EMSCRIPTEN_GL_KHR_parallel_shader_compile 1
// To enable: call emscripten_webgl_enable_extension(ctx, "KHR_parallel_shader_compile");
#define GL_COMPLETION_STATUS_KHR 0x91B1
// <no functions exposed>
#endif

// 40. https://www.khronos.org/registry/webgl/extensions/WEBGL_multi_draw/
#ifndef EMSCRIPTEN_GL_WEBGL_multi_draw
#define EMSCRIPTEN_GL_WEBGL_multi_draw 1
// To enable: call
bool emscripten_webgl_enable_WEBGL_multi_draw(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);
// or link with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=1 and
// call emscripten_webgl_enable_extension(ctx, "WEBGL_multi_draw");
WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawArraysWEBGL(GLenum mode,
                                                                 const GLint* firsts __attribute__((nonnull)),
                                                                 const GLsizei* counts __attribute__((nonnull)),
                                                                 GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawArraysInstancedWEBGL(GLenum mode,
                                                                          const GLint* firsts __attribute__((nonnull)),
                                                                          const GLsizei* counts __attribute__((nonnull)),
                                                                          const GLsizei* instanceCounts __attribute__((nonnull)),
                                                                          GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawElementsWEBGL(GLenum mode,
                                                                   const GLsizei* counts __attribute__((nonnull)),
                                                                   GLenum type,
                                                                   const GLvoid* const* offsets __attribute__((nonnull)),
                                                                   GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawElementsInstancedWEBGL(GLenum mode,
                                                                            const GLsizei* counts __attribute__((nonnull)),
                                                                            GLenum type,
                                                                            const GLvoid* const* offsets __attribute__((nonnull)),
                                                                            const GLsizei* instanceCounts __attribute__((nonnull)),
                                                                            GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY glMultiDrawArraysWEBGL(GLenum mode,
                                                      const GLint* firsts __attribute__((nonnull)),
                                                      const GLsizei* counts __attribute__((nonnull)),
                                                      GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY glMultiDrawArraysInstancedWEBGL(GLenum mode,
                                                               const GLint* firsts __attribute__((nonnull)),
                                                               const GLsizei* counts __attribute__((nonnull)),
                                                               const GLsizei* instanceCounts __attribute__((nonnull)),
                                                               GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY glMultiDrawElementsWEBGL(GLenum mode,
                                                        const GLsizei* counts __attribute__((nonnull)),
                                                        GLenum type,
                                                        const GLvoid* const* offsets __attribute__((nonnull)),
                                                        GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY glMultiDrawElementsInstancedWEBGL(GLenum mode,
                                                                 const GLsizei* counts __attribute__((nonnull)),
                                                                 GLenum type,
                                                                 const GLvoid* const* offsets __attribute__((nonnull)),
                                                                 const GLsizei* instanceCounts __attribute__((nonnull)),
                                                                 GLsizei drawcount);
#endif /* EMSCRIPTEN_GL_WEBGL_multi_draw */

// 44. https://www.khronos.org/registry/webgl/extensions/EXT_texture_norm16/
#ifndef EMSCRIPTEN_GL_EXT_texture_norm16
#define EMSCRIPTEN_GL_EXT_texture_norm16 1
// To enable: call emscripten_webgl_enable_extension(ctx, "EXT_texture_norm16");
#define GL_R16_EXT 0x822A
#define GL_RG16_EXT 0x822C
#define GL_RGB16_EXT 0x8054
#define GL_RGBA16_EXT 0x805B
#define GL_R16_SNORM_EXT 0x8F98
#define GL_RG16_SNORM_EXT 0x8F99
#define GL_RGB16_SNORM_EXT 0x8F9A
#define GL_RGBA16_SNORM_EXT 0x8F9B
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_EXT_texture_norm16 */

// EMSCRIPTEN_explicit_uniform_location
// https://github.com/emscripten-core/emscripten/blob/main/docs/EMSCRIPTEN_explicit_uniform_location.txt
#ifndef EMSCRIPTEN_explicit_uniform_location
#define EMSCRIPTEN_explicit_uniform_location 1
// To enable: link with -sGL_EXPLICIT_UNIFORM_LOCATION=1
#define GL_MAX_UNIFORM_LOCATIONS          0x826E
// <no functions exposed>
#endif

// EMSCRIPTEN_explicit_uniform_binding
// https://github.com/emscripten-core/emscripten/blob/main/docs/EMSCRIPTEN_explicit_uniform_binding.txt
#ifndef EMSCRIPTEN_explicit_uniform_binding
#define EMSCRIPTEN_explicit_uniform_binding 1
// To enable: link with -sGL_EXPLICIT_UNIFORM_BINDING=1
// <no functions or defines exposed>
#endif

// 50. https://registry.khronos.org/webgl/extensions/EXT_polygon_offset_clamp/
#ifndef EMSCRIPTEN_GL_EXT_polygon_offset_clamp
#define EMSCRIPTEN_GL_EXT_polygon_offset_clamp 1
// To enable: call
bool emscripten_webgl_enable_EXT_polygon_offset_clamp(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);
// or link with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=1 and
// call emscripten_webgl_enable_extension(ctx, "EXT_polygon_offset_clamp");
#define GL_POLYGON_OFFSET_CLAMP_EXT 0x8E1B
WEBGL_APICALL void GL_APIENTRY emscripten_glPolygonOffsetClampEXT(GLfloat factor, GLfloat units, GLfloat clamp);
WEBGL_APICALL void GL_APIENTRY glPolygonOffsetClampEXT(GLfloat factor, GLfloat units, GLfloat clamp);
#endif

// 51. https://registry.khronos.org/webgl/extensions/EXT_clip_control/
#ifndef EMSCRIPTEN_GL_EXT_clip_control
#define EMSCRIPTEN_GL_EXT_clip_control 1
// To enable: call
bool emscripten_webgl_enable_EXT_clip_control(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);
// or link with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=1 and
// call emscripten_webgl_enable_extension(ctx, "EXT_clip_control");
#define GL_LOWER_LEFT_EXT          0x8CA1
#define GL_UPPER_LEFT_EXT          0x8CA2
#define GL_NEGATIVE_ONE_TO_ONE_EXT 0x935E
#define GL_ZERO_TO_ONE_EXT         0x935F
#define GL_CLIP_ORIGIN_EXT         0x935C
#define GL_CLIP_DEPTH_MODE_EXT     0x935D
WEBGL_APICALL void GL_APIENTRY emscripten_glClipControlEXT(GLenum origin, GLenum depth);
WEBGL_APICALL void GL_APIENTRY glClipControlEXT(GLenum origin, GLenum depth);
#endif

// 52. https://registry.khronos.org/webgl/extensions/EXT_depth_clamp/
#ifndef EMSCRIPTEN_GL_EXT_depth_clamp
#define EMSCRIPTEN_GL_EXT_depth_clamp 1
// To enable: call emscripten_webgl_enable_extension(ctx, "EXT_depth_clamp");
#define GL_DEPTH_CLAMP_EXT 0x864F
// <no functions exposed>
#endif

// 53. https://registry.khronos.org/webgl/extensions/WEBGL_polygon_mode/
#ifndef EMSCRIPTEN_GL_WEBGL_polygon_mode
#define EMSCRIPTEN_GL_WEBGL_polygon_mode 1
// To enable: call
bool emscripten_webgl_enable_WEBGL_polygon_mode(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);
// or link with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=1 and
// call emscripten_webgl_enable_extension(ctx, "WEBGL_polygon_mode");
#define GL_POLYGON_MODE_WEBGL 0x0B40
#define GL_POLYGON_OFFSET_LINE_WEBGL 0x2A02
#define GL_LINE_WEBGL 0x1B01
#define GL_FILL_WEBGL 0x1B02
WEBGL_APICALL void GL_APIENTRY emscripten_glPolygonModeWEBGL(GLenum face, GLenum mode);
WEBGL_APICALL void GL_APIENTRY glPolygonModeWEBGL(GLenum face, GLenum mode);
#endif

/* To add a new GL extension here, follow the template

// <num>. <online URL to extension documentation>
#ifndef EMSCRIPTEN_GL_<extension_name>
#ifndef EMSCRIPTEN_GL_<extension_name> 1
// To enable: <enable_instructions>
<exposed defines>
<exposed emscripten_gl* function declarations>
<exposed gl* function declarations>
#endif
*/
