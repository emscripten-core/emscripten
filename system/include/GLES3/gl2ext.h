#ifndef __gl2ext_h_
#define __gl2ext_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

/*
** Copyright (c) 2013-2014 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

/*
** This header specifies the available
** WebGL 1.0 and 2.0 extensions that can be used when compiling with Emscripten.
** Note that with Emscripten, it is not necessary to dynamically query
** function pointers to obtain entry points for extensions, but all extensions
** are available via static linking by including this header.
**
** Some of the extensions specified in this file are direct matches to the
** the native GLES2 extensions, whereas some others are WebGL-specific.
*/

#ifndef GL_APIENTRYP
#define GL_APIENTRYP GL_APIENTRY*
#endif

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_astc/
#ifndef GL_KHR_texture_compression_astc_hdr
#define GL_KHR_texture_compression_astc_hdr 1
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR   0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR   0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR   0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR   0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR   0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR   0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR   0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR   0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR  0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR  0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR  0x93BA
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
#endif /* GL_KHR_texture_compression_astc_hdr */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_astc/
#ifndef GL_KHR_texture_compression_astc_ldr
#define GL_KHR_texture_compression_astc_ldr 1
#endif /* GL_KHR_texture_compression_astc_ldr */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_etc1/
#ifndef GL_OES_compressed_ETC1_RGB8_texture
#define GL_OES_compressed_ETC1_RGB8_texture 1
#define GL_ETC1_RGB8_OES                  0x8D64
#endif /* GL_OES_compressed_ETC1_RGB8_texture */

// Partially provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_depth_texture/
// Some restrictions apply
#ifndef GL_OES_depth_texture
#define GL_OES_depth_texture 1
#endif /* GL_OES_depth_texture */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_depth_texture/
#ifndef GL_WEBGL_depth_texture
#define GL_WEBGL_depth_texture 1
#define GL_UNSIGNED_INT_24_8_WEBGL 0x84FA
#endif /* GL_WEBGL_depth_texture */

// Provided by https://www.khronos.org/registry/webgl/extensions/OES_element_index_uint/
#ifndef GL_OES_element_index_uint
#define GL_OES_element_index_uint 1
#endif /* GL_OES_element_index_uint */

// Provided by https://www.khronos.org/registry/webgl/extensions/OES_fbo_render_mipmap/
#ifndef GL_OES_fbo_render_mipmap
#define GL_OES_fbo_render_mipmap 1
#endif /* GL_OES_fbo_render_mipmap */

// Provided by Emscripten's full OpenGL ES 2.0 emulation layer, which is enabled by building with the
// linker flag -s FULL_ES2=1
#ifndef GL_OES_mapbuffer
#define GL_OES_mapbuffer 1
#define GL_WRITE_ONLY_OES                 0x88B9
#define GL_BUFFER_ACCESS_OES              0x88BB
#define GL_BUFFER_MAPPED_OES              0x88BC
#define GL_BUFFER_MAP_POINTER_OES         0x88BD
typedef void *(GL_APIENTRYP PFNGLMAPBUFFEROESPROC) (GLenum target, GLenum access);
typedef GLboolean (GL_APIENTRYP PFNGLUNMAPBUFFEROESPROC) (GLenum target);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPOINTERVOESPROC) (GLenum target, GLenum pname, void **params);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void *GL_APIENTRY glMapBufferOES (GLenum target, GLenum access);
GL_APICALL GLboolean GL_APIENTRY glUnmapBufferOES (GLenum target);
GL_APICALL void GL_APIENTRY glGetBufferPointervOES (GLenum target, GLenum pname, void **params);
#endif
#endif /* GL_OES_mapbuffer */

// Partially provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_depth_texture/
// Some restrictions apply
#ifndef GL_OES_packed_depth_stencil
#define GL_OES_packed_depth_stencil 1
#define GL_DEPTH_STENCIL_OES              0x84F9
#define GL_UNSIGNED_INT_24_8_OES          0x84FA
#define GL_DEPTH24_STENCIL8_OES           0x88F0
#endif /* GL_OES_packed_depth_stencil */

// Provided by https://www.khronos.org/registry/webgl/extensions/OES_standard_derivatives/
#ifndef GL_OES_standard_derivatives
#define GL_OES_standard_derivatives 1
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES 0x8B8B
#endif /* GL_OES_standard_derivatives */

// Provided by https://www.khronos.org/registry/webgl/extensions/OES_texture_float/
#ifndef GL_OES_texture_float
#define GL_OES_texture_float 1
#endif /* GL_OES_texture_float */

// Provided by https://www.khronos.org/registry/webgl/extensions/OES_texture_float_linear/
#ifndef GL_OES_texture_float_linear
#define GL_OES_texture_float_linear 1
#endif /* GL_OES_texture_float_linear */

// Provided by https://www.khronos.org/registry/webgl/extensions/OES_texture_half_float/
#ifndef GL_OES_texture_half_float
#define GL_OES_texture_half_float 1
#define GL_HALF_FLOAT_OES                 0x8D61
#endif /* GL_OES_texture_half_float */

// Provided by https://www.khronos.org/registry/webgl/extensions/OES_texture_half_float_linear/
#ifndef GL_OES_texture_half_float_linear
#define GL_OES_texture_half_float_linear 1
#endif /* GL_OES_texture_half_float_linear */

// Provided by https://www.khronos.org/registry/webgl/extensions/OES_vertex_array_object/
#ifndef GL_OES_vertex_array_object
#define GL_OES_vertex_array_object 1
#define GL_VERTEX_ARRAY_BINDING_OES       0x85B5
typedef void (GL_APIENTRYP PFNGLBINDVERTEXARRAYOESPROC) (GLuint array);
typedef void (GL_APIENTRYP PFNGLDELETEVERTEXARRAYSOESPROC) (GLsizei n, const GLuint *arrays);
typedef void (GL_APIENTRYP PFNGLGENVERTEXARRAYSOESPROC) (GLsizei n, GLuint *arrays);
typedef GLboolean (GL_APIENTRYP PFNGLISVERTEXARRAYOESPROC) (GLuint array);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glBindVertexArrayOES (GLuint array);
GL_APICALL void GL_APIENTRY glDeleteVertexArraysOES (GLsizei n, const GLuint *arrays);
GL_APICALL void GL_APIENTRY glGenVertexArraysOES (GLsizei n, GLuint *arrays);
GL_APICALL GLboolean GL_APIENTRY glIsVertexArrayOES (GLuint array);
#endif
#endif /* GL_OES_vertex_array_object */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_atc/
#ifndef GL_AMD_compressed_ATC_texture
#define GL_AMD_compressed_ATC_texture 1
#define GL_ATC_RGB_AMD                    0x8C92
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD    0x8C93
#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD 0x87EE
#endif /* GL_AMD_compressed_ATC_texture */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_atc/
#ifndef GL_WEBGL_compressed_texture_atc
#define GL_WEBGL_compressed_texture_atc 1
#define GL_COMPRESSED_RGB_ATC_WEBGL                     0x8C92
#define GL_COMPRESSED_RGBA_ATC_EXPLICIT_ALPHA_WEBGL     0x8C93
#define GL_COMPRESSED_RGBA_ATC_INTERPOLATED_ALPHA_WEBGL 0x87EE
#endif /* GL_WEBGL_compressed_texture_atc */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_depth_texture/
#ifndef GL_ANGLE_depth_texture
#define GL_ANGLE_depth_texture 1
#endif /* GL_ANGLE_depth_texture */

// Provided by https://www.khronos.org/registry/webgl/extensions/ANGLE_instanced_arrays/
#ifndef GL_ANGLE_instanced_arrays
#define GL_ANGLE_instanced_arrays 1
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE 0x88FE
typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDANGLEPROC) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDANGLEPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISORANGLEPROC) (GLuint index, GLuint divisor);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glDrawArraysInstancedANGLE (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
GL_APICALL void GL_APIENTRY glDrawElementsInstancedANGLE (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
GL_APICALL void GL_APIENTRY glVertexAttribDivisorANGLE (GLuint index, GLuint divisor);
#endif
#endif /* GL_ANGLE_instanced_arrays */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_s3tc/
#ifndef GL_ANGLE_texture_compression_dxt3
#define GL_ANGLE_texture_compression_dxt3 1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE 0x83F2
#endif /* GL_ANGLE_texture_compression_dxt3 */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_s3tc/
#ifndef GL_ANGLE_texture_compression_dxt5
#define GL_ANGLE_texture_compression_dxt5 1
#define GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE 0x83F3
#endif /* GL_ANGLE_texture_compression_dxt5 */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_debug_shaders/
#ifndef GL_ANGLE_translated_shader_source
#define GL_ANGLE_translated_shader_source 1
#define GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE 0x93A0
typedef void (GL_APIENTRYP PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC) (GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGetTranslatedShaderSourceANGLE (GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source);
#endif
#endif /* GL_ANGLE_translated_shader_source */

// Provided by https://www.khronos.org/registry/webgl/extensions/EXT_blend_minmax/
#ifndef GL_EXT_blend_minmax
#define GL_EXT_blend_minmax 1
#define GL_MIN_EXT                        0x8007
#define GL_MAX_EXT                        0x8008
#endif /* GL_EXT_blend_minmax */

// Partially provided by https://www.khronos.org/registry/webgl/extensions/EXT_color_buffer_half_float/
// Some restrictions apply
#ifndef GL_EXT_color_buffer_half_float
#define GL_EXT_color_buffer_half_float 1
#define GL_RGBA16F_EXT                    0x881A
#define GL_RGB16F_EXT                     0x881B
#define GL_RG16F_EXT                      0x822F
#define GL_R16F_EXT                       0x822D
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT 0x8211
#define GL_UNSIGNED_NORMALIZED_EXT        0x8C17
#endif /* GL_EXT_color_buffer_half_float */

// Provided by https://www.khronos.org/registry/webgl/extensions/EXT_disjoint_timer_query/
#ifndef GL_EXT_disjoint_timer_query
#define GL_EXT_disjoint_timer_query 1
#define GL_QUERY_COUNTER_BITS_EXT         0x8864
#define GL_CURRENT_QUERY_EXT              0x8865
#define GL_QUERY_RESULT_EXT               0x8866
#define GL_QUERY_RESULT_AVAILABLE_EXT     0x8867
#define GL_TIME_ELAPSED_EXT               0x88BF
#define GL_TIMESTAMP_EXT                  0x8E28
#define GL_GPU_DISJOINT_EXT               0x8FBB
typedef void (GL_APIENTRYP PFNGLGENQUERIESEXTPROC) (GLsizei n, GLuint *ids);
typedef void (GL_APIENTRYP PFNGLDELETEQUERIESEXTPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean (GL_APIENTRYP PFNGLISQUERYEXTPROC) (GLuint id);
typedef void (GL_APIENTRYP PFNGLBEGINQUERYEXTPROC) (GLenum target, GLuint id);
typedef void (GL_APIENTRYP PFNGLENDQUERYEXTPROC) (GLenum target);
typedef void (GL_APIENTRYP PFNGLQUERYCOUNTEREXTPROC) (GLuint id, GLenum target);
typedef void (GL_APIENTRYP PFNGLGETQUERYIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTIVEXTPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUIVEXTPROC) (GLuint id, GLenum pname, GLuint *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTI64VEXTPROC) (GLuint id, GLenum pname, GLint64 *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUI64VEXTPROC) (GLuint id, GLenum pname, GLuint64 *params);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGenQueriesEXT (GLsizei n, GLuint *ids);
GL_APICALL void GL_APIENTRY glDeleteQueriesEXT (GLsizei n, const GLuint *ids);
GL_APICALL GLboolean GL_APIENTRY glIsQueryEXT (GLuint id);
GL_APICALL void GL_APIENTRY glBeginQueryEXT (GLenum target, GLuint id);
GL_APICALL void GL_APIENTRY glEndQueryEXT (GLenum target);
GL_APICALL void GL_APIENTRY glQueryCounterEXT (GLuint id, GLenum target);
GL_APICALL void GL_APIENTRY glGetQueryivEXT (GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectivEXT (GLuint id, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectuivEXT (GLuint id, GLenum pname, GLuint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjecti64vEXT (GLuint id, GLenum pname, GLint64 *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectui64vEXT (GLuint id, GLenum pname, GLuint64 *params);
#endif
#endif /* GL_EXT_disjoint_timer_query */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_draw_buffers/
#ifndef GL_EXT_draw_buffers
#define GL_EXT_draw_buffers 1
#define GL_MAX_COLOR_ATTACHMENTS_EXT      0x8CDF
#define GL_MAX_DRAW_BUFFERS_EXT           0x8824
#define GL_DRAW_BUFFER0_EXT               0x8825
#define GL_DRAW_BUFFER1_EXT               0x8826
#define GL_DRAW_BUFFER2_EXT               0x8827
#define GL_DRAW_BUFFER3_EXT               0x8828
#define GL_DRAW_BUFFER4_EXT               0x8829
#define GL_DRAW_BUFFER5_EXT               0x882A
#define GL_DRAW_BUFFER6_EXT               0x882B
#define GL_DRAW_BUFFER7_EXT               0x882C
#define GL_DRAW_BUFFER8_EXT               0x882D
#define GL_DRAW_BUFFER9_EXT               0x882E
#define GL_DRAW_BUFFER10_EXT              0x882F
#define GL_DRAW_BUFFER11_EXT              0x8830
#define GL_DRAW_BUFFER12_EXT              0x8831
#define GL_DRAW_BUFFER13_EXT              0x8832
#define GL_DRAW_BUFFER14_EXT              0x8833
#define GL_DRAW_BUFFER15_EXT              0x8834
#define GL_COLOR_ATTACHMENT0_EXT          0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT          0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT          0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT          0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT          0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT          0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT          0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT          0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT          0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT          0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT         0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT         0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT         0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT         0x8CED
#define GL_COLOR_ATTACHMENT14_EXT         0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT         0x8CEF
typedef void (GL_APIENTRYP PFNGLDRAWBUFFERSEXTPROC) (GLsizei n, const GLenum *bufs);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glDrawBuffersEXT (GLsizei n, const GLenum *bufs);
#endif
#endif /* GL_EXT_draw_buffers */

// Provided by https://www.khronos.org/registry/webgl/extensions/ANGLE_instanced_arrays/
#ifndef GL_EXT_draw_instanced
#define GL_EXT_draw_instanced 1
typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDEXTPROC) (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glDrawArraysInstancedEXT (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
GL_APICALL void GL_APIENTRY glDrawElementsInstancedEXT (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
#endif
#endif /* GL_EXT_draw_instanced */

// Provided by https://www.khronos.org/registry/webgl/extensions/ANGLE_instanced_arrays/
#ifndef GL_EXT_instanced_arrays
#define GL_EXT_instanced_arrays 1
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_EXT 0x88FE
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISOREXTPROC) (GLuint index, GLuint divisor);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glVertexAttribDivisorEXT (GLuint index, GLuint divisor);
#endif
#endif /* GL_EXT_instanced_arrays */

// Provided by https://www.khronos.org/registry/webgl/extensions/EXT_sRGB/
#ifndef GL_EXT_sRGB
#define GL_EXT_sRGB 1
#define GL_SRGB_EXT                       0x8C40
#define GL_SRGB_ALPHA_EXT                 0x8C42
#define GL_SRGB8_ALPHA8_EXT               0x8C43
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT 0x8210
#endif /* GL_EXT_sRGB */

// Provided by https://www.khronos.org/registry/webgl/extensions/EXT_shader_texture_lod/
#ifndef GL_EXT_shader_texture_lod
#define GL_EXT_shader_texture_lod 1
#endif /* GL_EXT_shader_texture_lod */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_s3tc/
#ifndef GL_EXT_texture_compression_dxt1
#define GL_EXT_texture_compression_dxt1 1
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#endif /* GL_EXT_texture_compression_dxt1 */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_s3tc/
#ifndef GL_EXT_texture_compression_s3tc
#define GL_EXT_texture_compression_s3tc 1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif /* GL_EXT_texture_compression_s3tc */

// Provided by https://www.khronos.org/registry/webgl/extensions/EXT_texture_filter_anisotropic/
#ifndef GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_filter_anisotropic 1
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif /* GL_EXT_texture_filter_anisotropic */

// Provided by https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_pvrtc/
#ifndef GL_IMG_texture_compression_pvrtc
#define GL_IMG_texture_compression_pvrtc 1
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
#endif /* GL_IMG_texture_compression_pvrtc */

// Provided by https://www.khronos.org/registry/webgl/extensions/ANGLE_instanced_arrays/
#ifndef GL_NV_draw_instanced
#define GL_NV_draw_instanced 1
typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDNVPROC) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDNVPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glDrawArraysInstancedNV (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
GL_APICALL void GL_APIENTRY glDrawElementsInstancedNV (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
#endif
#endif /* GL_NV_draw_instanced */

// Provided by https://www.khronos.org/registry/webgl/extensions/ANGLE_instanced_arrays/
#ifndef GL_NV_instanced_arrays
#define GL_NV_instanced_arrays 1
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_NV 0x88FE
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISORNVPROC) (GLuint index, GLuint divisor);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glVertexAttribDivisorNV (GLuint index, GLuint divisor);
#endif
#endif /* GL_NV_instanced_arrays */

#ifdef __cplusplus
}
#endif

#endif
