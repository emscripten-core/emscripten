#pragma once

#include "webgl1.h"
#include <emscripten/html5.h>

// 1. https://www.khronos.org/registry/webgl/extensions/OES_texture_float/
#ifndef EMSCRIPTEN_GL_OES_texture_float
#define EMSCRIPTEN_GL_OES_texture_float 1
// <no symbols exposed>
#endif /* EMSCRIPTEN_GL_OES_texture_float */

// 2. https://www.khronos.org/registry/webgl/extensions/OES_texture_half_float/
#ifndef EMSCRIPTEN_GL_OES_texture_half_float
#define EMSCRIPTEN_GL_OES_texture_half_float 1
#define GL_HALF_FLOAT_OES 0x8D61
#endif /* EMSCRIPTEN_GL_OES_texture_half_float */

// 3. https://www.khronos.org/registry/webgl/extensions/WEBGL_lose_context/
#ifndef EMSCRIPTEN_GL_WEBGL_lose_context
#define EMSCRIPTEN_GL_WEBGL_lose_context 1
WEBGL_APICALL EMSCRIPTEN_RESULT GL_APIENTRY emscripten_webgl_loseContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE contextHandle);
WEBGL_APICALL EMSCRIPTEN_RESULT GL_APIENTRY emscripten_webgl_restoreContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE contextHandle);
#endif /* EMSCRIPTEN_GL_WEBGL_lose_context */

// 4. https://www.khronos.org/registry/webgl/extensions/OES_standard_derivatives/
#ifndef EMSCRIPTEN_GL_OES_standard_derivatives
#define EMSCRIPTEN_GL_OES_standard_derivatives 1
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES 0x8B8B
#endif /* EMSCRIPTEN_GL_OES_standard_derivatives */

// 5. https://www.khronos.org/registry/webgl/extensions/OES_vertex_array_object/
#ifndef EMSCRIPTEN_GL_OES_vertex_array_object
#define EMSCRIPTEN_GL_OES_vertex_array_object 1
#define GL_VERTEX_ARRAY_BINDING_OES 0x85B5
WEBGL_APICALL void GL_APIENTRY emscripten_glBindVertexArrayOES(GLuint array);
WEBGL_APICALL void GL_APIENTRY emscripten_glDeleteVertexArraysOES(GLsizei n, const GLuint *arrays);
WEBGL_APICALL void GL_APIENTRY emscripten_glGenVertexArraysOES(GLsizei n, GLuint *arrays);
WEBGL_APICALL GLboolean GL_APIENTRY emscripten_glIsVertexArrayOES(GLuint array);
WEBGL_APICALL void GL_APIENTRY glBindVertexArrayOES(GLuint array);
WEBGL_APICALL void GL_APIENTRY glDeleteVertexArraysOES(GLsizei n, const GLuint *arrays);
WEBGL_APICALL void GL_APIENTRY glGenVertexArraysOES(GLsizei n, GLuint *arrays);
WEBGL_APICALL GLboolean GL_APIENTRY glIsVertexArrayOES(GLuint array);
#endif /* EMSCRIPTEN_GL_OES_vertex_array_object */

// 6. https://www.khronos.org/registry/webgl/extensions/WEBGL_debug_renderer_info/
#ifndef EMSCRIPTEN_GL_WEBGL_debug_renderer_info
#define EMSCRIPTEN_GL_WEBGL_debug_renderer_info 1
#define GL_UNMASKED_VENDOR_WEBGL 0x9245
#define GL_UNMASKED_RENDERER_WEBGL 0x9246
#endif /* EMSCRIPTEN_GL_WEBGL_debug_renderer_info */

// 7. https://www.khronos.org/registry/webgl/extensions/WEBGL_debug_shaders/
#ifndef EMSCRIPTEN_GL_WEBGL_debug_shaders
#define EMSCRIPTEN_GL_WEBGL_debug_shaders 1
WEBGL_APICALL void GL_APIENTRY emscripten_webgl_getTranslatedShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
#endif /* EMSCRIPTEN_GL_WEBGL_debug_shaders */

// 8. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_s3tc/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc 1
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc */

// 9. https://www.khronos.org/registry/webgl/extensions/WEBGL_depth_texture/
#ifndef EMSCRIPTEN_GL_WEBGL_depth_texture
#define EMSCRIPTEN_GL_WEBGL_depth_texture 1
#define GL_UNSIGNED_INT_24_8_WEBGL 0x84FA
#endif /* EMSCRIPTEN_GL_WEBGL_depth_texture */

// 10. https://www.khronos.org/registry/webgl/extensions/OES_element_index_uint/
#ifndef EMSCRIPTEN_GL_OES_element_index_uint
#define EMSCRIPTEN_GL_OES_element_index_uint 1
// <no symbols exposed>
#endif /* EMSCRIPTEN_GL_OES_element_index_uint */

// 11. https://www.khronos.org/registry/webgl/extensions/EXT_texture_filter_anisotropic/
#ifndef EMSCRIPTEN_GL_EXT_texture_filter_anisotropic
#define EMSCRIPTEN_GL_EXT_texture_filter_anisotropic 1
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif /* EMSCRIPTEN_GL_EXT_texture_filter_anisotropic */

// 16. https://www.khronos.org/registry/webgl/extensions/EXT_frag_depth/
#ifndef EMSCRIPTEN_GL_EXT_frag_depth
#define EMSCRIPTEN_GL_EXT_frag_depth 1
// <no symbols exposed>
#endif /* EMSCRIPTEN_GL_EXT_frag_depth */

// 18. https://www.khronos.org/registry/webgl/extensions/WEBGL_draw_buffers/
#ifndef EMSCRIPTEN_GL_WEBGL_draw_buffers
#define EMSCRIPTEN_GL_WEBGL_draw_buffers 1
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
// <no symbols exposed>
#endif /* EMSCRIPTEN_GL_OES_texture_float_linear */

// 21. https://www.khronos.org/registry/webgl/extensions/OES_texture_half_float_linear/
#ifndef EMSCRIPTEN_GL_OES_texture_half_float_linear
#define EMSCRIPTEN_GL_OES_texture_half_float_linear 1
// <no symbols exposed>
#endif /* EMSCRIPTEN_GL_OES_texture_half_float_linear */

// 25. https://www.khronos.org/registry/webgl/extensions/EXT_blend_minmax/
#ifndef EMSCRIPTEN_GL_EXT_blend_minmax
#define EMSCRIPTEN_GL_EXT_blend_minmax 1
#define GL_MIN_EXT 0x8007
#define GL_MAX_EXT 0x8008
#endif /* EMSCRIPTEN_GL_EXT_blend_minmax */

// 27. https://www.khronos.org/registry/webgl/extensions/EXT_shader_texture_lod/
#ifndef EMSCRIPTEN_GL_EXT_shader_texture_lod
#define EMSCRIPTEN_GL_EXT_shader_texture_lod 1
// <no symbols exposed>
#endif /* EMSCRIPTEN_GL_EXT_shader_texture_lod */

// 13. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_pvrtc/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_pvrtc
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_pvrtc 1
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_pvrtc */

// 14. https://www.khronos.org/registry/webgl/extensions/EXT_color_buffer_half_float/
#ifndef EMSCRIPTEN_GL_EXT_color_buffer_half_float
#define EMSCRIPTEN_GL_EXT_color_buffer_half_float 1
#define GL_RGBA16F_EXT 0x881A
#define GL_RGB16F_EXT 0x881B
#define GL_RG16F_EXT 0x822F
#define GL_R16F_EXT 0x822D
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT 0x8211
#define GL_UNSIGNED_NORMALIZED_EXT 0x8C17
#endif /* EMSCRIPTEN_GL_EXT_color_buffer_half_float */

// 15. https://www.khronos.org/registry/webgl/extensions/WEBGL_color_buffer_float/
#ifndef EMSCRIPTEN_GL_WEBGL_color_buffer_float
#define EMSCRIPTEN_GL_WEBGL_color_buffer_float 1
#define GL_RGBA32F_EXT 0x8814
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT 0x8211
#define GL_UNSIGNED_NORMALIZED_EXT 0x8C17
#endif /* EMSCRIPTEN_GL_WEBGL_color_buffer_float */

// 17. https://www.khronos.org/registry/webgl/extensions/EXT_sRGB/
#ifndef EMSCRIPTEN_GL_EXT_sRGB
#define EMSCRIPTEN_GL_EXT_sRGB 1
#define GL_SRGB_EXT 0x8C40
#define GL_SRGB_ALPHA_EXT 0x8C42
#define GL_SRGB8_ALPHA8_EXT 0x8C43
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT 0x8210
#endif /* EMSCRIPTEN_GL_EXT_sRGB */

// 24. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_etc1/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_etc1
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_etc1 1
#define GL_COMPRESSED_RGB_ETC1_WEBGL 0x8D64
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
WEBGL_APICALL void GL_APIENTRY emscripten_glGenQueriesEXT(GLsizei n, GLuint *ids);
WEBGL_APICALL void GL_APIENTRY emscripten_glDeleteQueriesEXT(GLsizei n, const GLuint *ids);
WEBGL_APICALL GLboolean GL_APIENTRY emscripten_glIsQueryEXT(GLuint id);
WEBGL_APICALL void GL_APIENTRY emscripten_glBeginQueryEXT(GLenum target, GLuint id);
WEBGL_APICALL void GL_APIENTRY emscripten_glEndQueryEXT(GLenum target);
WEBGL_APICALL void GL_APIENTRY emscripten_glQueryCounterEXT(GLuint id, GLenum target);
WEBGL_APICALL void GL_APIENTRY emscripten_glGetQueryivEXT(GLenum target, GLenum pname, GLint *params);
WEBGL_APICALL void GL_APIENTRY emscripten_glGetQueryObjectivEXT(GLuint id, GLenum pname, GLint *params);
WEBGL_APICALL void GL_APIENTRY emscripten_glGetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint *params);
WEBGL_APICALL void GL_APIENTRY emscripten_glGetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64 *params);
WEBGL_APICALL void GL_APIENTRY emscripten_glGetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64 *params);
WEBGL_APICALL void GL_APIENTRY glGenQueriesEXT(GLsizei n, GLuint *ids);
WEBGL_APICALL void GL_APIENTRY glDeleteQueriesEXT(GLsizei n, const GLuint *ids);
WEBGL_APICALL GLboolean GL_APIENTRY glIsQueryEXT(GLuint id);
WEBGL_APICALL void GL_APIENTRY glBeginQueryEXT(GLenum target, GLuint id);
WEBGL_APICALL void GL_APIENTRY glEndQueryEXT(GLenum target);
WEBGL_APICALL void GL_APIENTRY glQueryCounterEXT(GLuint id, GLenum target);
WEBGL_APICALL void GL_APIENTRY glGetQueryivEXT(GLenum target, GLenum pname, GLint *params);
WEBGL_APICALL void GL_APIENTRY glGetQueryObjectivEXT(GLuint id, GLenum pname, GLint *params);
WEBGL_APICALL void GL_APIENTRY glGetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint *params);
WEBGL_APICALL void GL_APIENTRY glGetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64 *params);
WEBGL_APICALL void GL_APIENTRY glGetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64 *params);
#endif /* EMSCRIPTEN_GL_EXT_disjoint_timer_query */

// 29. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_etc/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_etc
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_etc 1
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
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_etc */

// 30. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_astc/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_astc
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_astc 1
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
WEBGL_APICALL void GL_APIENTRY emscripten_webgl_getSupportedAstcProfiles(GLsizei bufSize, GLsizei *length, GLchar *buf);
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_astc */

// 31. https://www.khronos.org/registry/webgl/extensions/EXT_color_buffer_float/
#ifndef EMSCRIPTEN_GL_EXT_color_buffer_float
#define EMSCRIPTEN_GL_EXT_color_buffer_float 1
// <no symbols exposed>
#endif /* EMSCRIPTEN_GL_EXT_color_buffer_float */

// 32. https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_s3tc_srgb/
#ifndef EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc_srgb
#define EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc_srgb 1
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#endif /* EMSCRIPTEN_GL_WEBGL_compressed_texture_s3tc_srgb */

// 40. https://www.khronos.org/registry/webgl/extensions/WEBGL_multi_draw/
#ifndef EMSCRIPTEN_GL_WEBGL_multi_draw
#define EMSCRIPTEN_GL_WEBGL_multi_draw 1
WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawArraysWEBGL(GLenum mode, const GLint* firsts, const GLsizei* counts, GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawArraysInstancedWEBGL(GLenum mode, const GLint* firsts, const GLsizei* counts, const GLsizei* instanceCounts, GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawElementsWEBGL(GLenum mode, const GLsizei* counts, GLenum type, const GLvoid* const* offsets, GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawElementsInstancedWEBGL(GLenum mode, const GLsizei* counts, GLenum type, const GLvoid* const* offsets, const GLsizei* instanceCounts, GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY glMultiDrawArraysWEBGL(GLenum mode, const GLint* firsts, const GLsizei* counts, GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY glMultiDrawArraysInstancedWEBGL(GLenum mode, const GLint* firsts, const GLsizei* counts, const GLsizei* instanceCounts, GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY glMultiDrawElementsWEBGL(GLenum mode, const GLsizei* counts, GLenum type, const GLvoid* const* offsets, GLsizei drawcount);
WEBGL_APICALL void GL_APIENTRY glMultiDrawElementsInstancedWEBGL(GLenum mode, const GLsizei* counts, GLenum type, const GLvoid* const* offsets, const GLsizei* instanceCounts, GLsizei drawcount);
#endif /* EMSCRIPTEN_GL_WEBGL_multi_draw */

// 44. https://www.khronos.org/registry/webgl/extensions/EXT_texture_norm16/
#ifndef EMSCRIPTEN_GL_EXT_texture_norm16
#define EMSCRIPTEN_GL_EXT_texture_norm16 1
#define GL_R16_EXT 0x822A
#define GL_RG16_EXT 0x822C
#define GL_RGB16_EXT 0x8054
#define GL_RGBA16_EXT 0x805B
#define GL_R16_SNORM_EXT 0x8F98
#define GL_RG16_SNORM_EXT 0x8F99
#define GL_RGB16_SNORM_EXT 0x8F9A
#define GL_RGBA16_SNORM_EXT 0x8F9B
#endif /* EMSCRIPTEN_GL_EXT_texture_norm16 */
