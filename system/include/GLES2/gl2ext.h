#ifndef __gl2ext_h_
#define __gl2ext_h_

/* $Revision: 10969 $ on $Date:: 2010-04-09 02:27:15 -0700 #$ */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This document is licensed under the SGI Free Software B License Version
 * 2.0. For details, see http://oss.sgi.com/projects/FreeB/ .
 */

#ifndef GL_APIENTRYP
#   define GL_APIENTRYP GL_APIENTRY*
#endif

/*------------------------------------------------------------------------*
 * OES extension tokens
 *------------------------------------------------------------------------*/

/* GL_OES_compressed_ETC1_RGB8_texture */
#ifndef GL_OES_compressed_ETC1_RGB8_texture
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif

/* GL_OES_compressed_paletted_texture */
#ifndef GL_OES_compressed_paletted_texture
#define GL_PALETTE4_RGB8_OES                                    0x8B90
#define GL_PALETTE4_RGBA8_OES                                   0x8B91
#define GL_PALETTE4_R5_G6_B5_OES                                0x8B92
#define GL_PALETTE4_RGBA4_OES                                   0x8B93
#define GL_PALETTE4_RGB5_A1_OES                                 0x8B94
#define GL_PALETTE8_RGB8_OES                                    0x8B95
#define GL_PALETTE8_RGBA8_OES                                   0x8B96
#define GL_PALETTE8_R5_G6_B5_OES                                0x8B97
#define GL_PALETTE8_RGBA4_OES                                   0x8B98
#define GL_PALETTE8_RGB5_A1_OES                                 0x8B99
#endif

/* GL_OES_depth24 */
#ifndef GL_OES_depth24
#define GL_DEPTH_COMPONENT24_OES                                0x81A6
#endif

/* GL_OES_depth32 */
#ifndef GL_OES_depth32
#define GL_DEPTH_COMPONENT32_OES                                0x81A7
#endif

/* GL_OES_depth_texture */
/* No new tokens introduced by this extension. */

/* GL_OES_EGL_image */
#ifndef GL_OES_EGL_image
typedef void* GLeglImageOES;
#endif

/* GL_OES_element_index_uint */
#ifndef GL_OES_element_index_uint
#define GL_UNSIGNED_INT                                         0x1405
#endif

/* GL_OES_get_program_binary */
#ifndef GL_OES_get_program_binary
#define GL_PROGRAM_BINARY_LENGTH_OES                            0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS_OES                       0x87FE
#define GL_PROGRAM_BINARY_FORMATS_OES                           0x87FF
#endif

/* GL_OES_mapbuffer */
#ifndef GL_OES_mapbuffer
#define GL_WRITE_ONLY_OES                                       0x88B9
#define GL_BUFFER_ACCESS_OES                                    0x88BB
#define GL_BUFFER_MAPPED_OES                                    0x88BC
#define GL_BUFFER_MAP_POINTER_OES                               0x88BD
#endif

/* GL_OES_packed_depth_stencil */
#ifndef GL_OES_packed_depth_stencil
#define GL_DEPTH_STENCIL_OES                                    0x84F9
#define GL_UNSIGNED_INT_24_8_OES                                0x84FA
#define GL_DEPTH24_STENCIL8_OES                                 0x88F0
#endif

/* GL_OES_rgb8_rgba8 */
#ifndef GL_OES_rgb8_rgba8
#define GL_RGB8_OES                                             0x8051
#define GL_RGBA8_OES                                            0x8058
#endif

/* GL_OES_standard_derivatives */
#ifndef GL_OES_standard_derivatives
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES                  0x8B8B
#endif

/* GL_OES_stencil1 */
#ifndef GL_OES_stencil1
#define GL_STENCIL_INDEX1_OES                                   0x8D46
#endif

/* GL_OES_stencil4 */
#ifndef GL_OES_stencil4
#define GL_STENCIL_INDEX4_OES                                   0x8D47
#endif

/* GL_OES_texture_3D */
#ifndef GL_OES_texture_3D
#define GL_TEXTURE_WRAP_R_OES                                   0x8072
#define GL_TEXTURE_3D_OES                                       0x806F
#define GL_TEXTURE_BINDING_3D_OES                               0x806A
#define GL_MAX_3D_TEXTURE_SIZE_OES                              0x8073
#define GL_SAMPLER_3D_OES                                       0x8B5F
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES        0x8CD4
#endif

/* GL_OES_texture_float */
/* No new tokens introduced by this extension. */

/* GL_OES_texture_float_linear */
/* No new tokens introduced by this extension. */

/* GL_OES_texture_half_float */
#ifndef GL_OES_texture_half_float
#define GL_HALF_FLOAT_OES                                       0x8D61
#endif

/* GL_OES_texture_half_float_linear */
/* No new tokens introduced by this extension. */

/* GL_OES_texture_npot */
/* No new tokens introduced by this extension. */

/* GL_OES_vertex_array_object */
#ifndef GL_OES_vertex_array_object
#define GL_VERTEX_ARRAY_BINDING_OES                             0x85B5
#endif

/* GL_OES_vertex_half_float */
/* GL_HALF_FLOAT_OES defined in GL_OES_texture_half_float already. */

/* GL_OES_vertex_type_10_10_10_2 */
#ifndef GL_OES_vertex_type_10_10_10_2
#define GL_UNSIGNED_INT_10_10_10_2_OES                          0x8DF6
#define GL_INT_10_10_10_2_OES                                   0x8DF7
#endif

/*------------------------------------------------------------------------*
 * AMD extension tokens
 *------------------------------------------------------------------------*/

/* GL_AMD_compressed_3DC_texture */
#ifndef GL_AMD_compressed_3DC_texture
#define GL_3DC_X_AMD                                            0x87F9
#define GL_3DC_XY_AMD                                           0x87FA
#endif

/* GL_AMD_compressed_ATC_texture */
#ifndef GL_AMD_compressed_ATC_texture
#define GL_ATC_RGB_AMD                                          0x8C92
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD                          0x8C93
#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD                      0x87EE
#endif

/* GL_AMD_performance_monitor */
#ifndef GL_AMD_performance_monitor
#define GL_COUNTER_TYPE_AMD                                     0x8BC0
#define GL_COUNTER_RANGE_AMD                                    0x8BC1
#define GL_UNSIGNED_INT64_AMD                                   0x8BC2
#define GL_PERCENTAGE_AMD                                       0x8BC3
#define GL_PERFMON_RESULT_AVAILABLE_AMD                         0x8BC4
#define GL_PERFMON_RESULT_SIZE_AMD                              0x8BC5
#define GL_PERFMON_RESULT_AMD                                   0x8BC6
#endif

/* GL_AMD_program_binary_Z400 */
#ifndef GL_AMD_program_binary_Z400
#define GL_Z400_BINARY_AMD                                      0x8740
#endif

/*------------------------------------------------------------------------*
 * EXT extension tokens
 *------------------------------------------------------------------------*/

/* GL_EXT_blend_minmax */
#ifndef GL_EXT_blend_minmax
#define GL_MIN_EXT                                              0x8007
#define GL_MAX_EXT                                              0x8008
#endif

/* GL_EXT_discard_framebuffer */
#ifndef GL_EXT_discard_framebuffer
#define GL_COLOR_EXT                                            0x1800
#define GL_DEPTH_EXT                                            0x1801
#define GL_STENCIL_EXT                                          0x1802
#endif

/* GL_EXT_multi_draw_arrays */
/* No new tokens introduced by this extension. */

/* GL_EXT_read_format_bgra */
#ifndef GL_EXT_read_format_bgra
#define GL_BGRA_EXT                                             0x80E1
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT                       0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT                       0x8366
#endif

/* GL_EXT_texture_filter_anisotropic */
#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT                           0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT                       0x84FF
#endif

/* GL_EXT_texture_format_BGRA8888 */
#ifndef GL_EXT_texture_format_BGRA8888
#define GL_BGRA_EXT                                             0x80E1
#endif

/* GL_EXT_texture_type_2_10_10_10_REV */
#ifndef GL_EXT_texture_type_2_10_10_10_REV
#define GL_UNSIGNED_INT_2_10_10_10_REV_EXT                      0x8368
#endif

/* GL_EXT_texture_compression_dxt1 */
#ifndef GL_EXT_texture_compression_dxt1
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                         0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                        0x83F1
#endif

/*------------------------------------------------------------------------*
 * IMG extension tokens
 *------------------------------------------------------------------------*/

/* GL_IMG_program_binary */
#ifndef GL_IMG_program_binary
#define GL_SGX_PROGRAM_BINARY_IMG                               0x9130
#endif

/* GL_IMG_read_format */
#ifndef GL_IMG_read_format
#define GL_BGRA_IMG                                             0x80E1
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_IMG                       0x8365
#endif

/* GL_IMG_shader_binary */
#ifndef GL_IMG_shader_binary
#define GL_SGX_BINARY_IMG                                       0x8C0A
#endif

/* GL_IMG_texture_compression_pvrtc */
#ifndef GL_IMG_texture_compression_pvrtc
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03
#endif

/* GL_IMG_multisampled_render_to_texture */
#ifndef GL_IMG_multisampled_render_to_texture
#define GL_RENDERBUFFER_SAMPLES_IMG                             0x9133
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG               0x9134
#define GL_MAX_SAMPLES_IMG                                      0x9135
#define GL_TEXTURE_SAMPLES_IMG                                  0x9136
#endif

/*------------------------------------------------------------------------*
 * NV extension tokens
 *------------------------------------------------------------------------*/

/* GL_NV_fence */
#ifndef GL_NV_fence
#define GL_ALL_COMPLETED_NV                                     0x84F2
#define GL_FENCE_STATUS_NV                                      0x84F3
#define GL_FENCE_CONDITION_NV                                   0x84F4
#endif

/* GL_NV_coverage_sample */
#ifndef GL_NV_coverage_sample
#define GL_COVERAGE_COMPONENT_NV                                0x8ED0
#define GL_COVERAGE_COMPONENT4_NV                               0x8ED1
#define GL_COVERAGE_ATTACHMENT_NV                               0x8ED2
#define GL_COVERAGE_BUFFERS_NV                                  0x8ED3
#define GL_COVERAGE_SAMPLES_NV                                  0x8ED4
#define GL_COVERAGE_ALL_FRAGMENTS_NV                            0x8ED5
#define GL_COVERAGE_EDGE_FRAGMENTS_NV                           0x8ED6
#define GL_COVERAGE_AUTOMATIC_NV                                0x8ED7
#define GL_COVERAGE_BUFFER_BIT_NV                               0x8000
#endif

/* GL_NV_depth_nonlinear */
#ifndef GL_NV_depth_nonlinear
#define GL_DEPTH_COMPONENT16_NONLINEAR_NV                       0x8E2C
#endif

/*------------------------------------------------------------------------*
 * QCOM extension tokens
 *------------------------------------------------------------------------*/

/* GL_QCOM_driver_control */
/* No new tokens introduced by this extension. */

/* GL_QCOM_extended_get */
#ifndef GL_QCOM_extended_get
#define GL_TEXTURE_WIDTH_QCOM                                   0x8BD2
#define GL_TEXTURE_HEIGHT_QCOM                                  0x8BD3
#define GL_TEXTURE_DEPTH_QCOM                                   0x8BD4
#define GL_TEXTURE_INTERNAL_FORMAT_QCOM                         0x8BD5
#define GL_TEXTURE_FORMAT_QCOM                                  0x8BD6
#define GL_TEXTURE_TYPE_QCOM                                    0x8BD7
#define GL_TEXTURE_IMAGE_VALID_QCOM                             0x8BD8
#define GL_TEXTURE_NUM_LEVELS_QCOM                              0x8BD9
#define GL_TEXTURE_TARGET_QCOM                                  0x8BDA
#define GL_TEXTURE_OBJECT_VALID_QCOM                            0x8BDB
#define GL_STATE_RESTORE                                        0x8BDC
#endif

/* GL_QCOM_extended_get2 */
/* No new tokens introduced by this extension. */

/* GL_QCOM_perfmon_global_mode */
#ifndef GL_QCOM_perfmon_global_mode
#define GL_PERFMON_GLOBAL_MODE_QCOM                             0x8FA0
#endif

/* GL_QCOM_writeonly_rendering */
#ifndef GL_QCOM_writeonly_rendering
#define GL_WRITEONLY_RENDERING_QCOM                             0x8823
#endif

/* GL_QCOM_tiled_rendering */
#ifndef GL_QCOM_tiled_rendering
#define GL_COLOR_BUFFER_BIT0_QCOM                               0x00000001
#define GL_COLOR_BUFFER_BIT1_QCOM                               0x00000002
#define GL_COLOR_BUFFER_BIT2_QCOM                               0x00000004
#define GL_COLOR_BUFFER_BIT3_QCOM                               0x00000008
#define GL_COLOR_BUFFER_BIT4_QCOM                               0x00000010
#define GL_COLOR_BUFFER_BIT5_QCOM                               0x00000020
#define GL_COLOR_BUFFER_BIT6_QCOM                               0x00000040
#define GL_COLOR_BUFFER_BIT7_QCOM                               0x00000080
#define GL_DEPTH_BUFFER_BIT0_QCOM                               0x00000100
#define GL_DEPTH_BUFFER_BIT1_QCOM                               0x00000200
#define GL_DEPTH_BUFFER_BIT2_QCOM                               0x00000400
#define GL_DEPTH_BUFFER_BIT3_QCOM                               0x00000800
#define GL_DEPTH_BUFFER_BIT4_QCOM                               0x00001000
#define GL_DEPTH_BUFFER_BIT5_QCOM                               0x00002000
#define GL_DEPTH_BUFFER_BIT6_QCOM                               0x00004000
#define GL_DEPTH_BUFFER_BIT7_QCOM                               0x00008000
#define GL_STENCIL_BUFFER_BIT0_QCOM                             0x00010000
#define GL_STENCIL_BUFFER_BIT1_QCOM                             0x00020000
#define GL_STENCIL_BUFFER_BIT2_QCOM                             0x00040000
#define GL_STENCIL_BUFFER_BIT3_QCOM                             0x00080000
#define GL_STENCIL_BUFFER_BIT4_QCOM                             0x00100000
#define GL_STENCIL_BUFFER_BIT5_QCOM                             0x00200000
#define GL_STENCIL_BUFFER_BIT6_QCOM                             0x00400000
#define GL_STENCIL_BUFFER_BIT7_QCOM                             0x00800000
#define GL_MULTISAMPLE_BUFFER_BIT0_QCOM                         0x01000000
#define GL_MULTISAMPLE_BUFFER_BIT1_QCOM                         0x02000000
#define GL_MULTISAMPLE_BUFFER_BIT2_QCOM                         0x04000000
#define GL_MULTISAMPLE_BUFFER_BIT3_QCOM                         0x08000000
#define GL_MULTISAMPLE_BUFFER_BIT4_QCOM                         0x10000000
#define GL_MULTISAMPLE_BUFFER_BIT5_QCOM                         0x20000000
#define GL_MULTISAMPLE_BUFFER_BIT6_QCOM                         0x40000000
#define GL_MULTISAMPLE_BUFFER_BIT7_QCOM                         0x80000000
#endif

/*------------------------------------------------------------------------*
 * End of extension tokens, start of corresponding extension functions
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*
 * OES extension functions
 *------------------------------------------------------------------------*/

/* GL_OES_compressed_ETC1_RGB8_texture */
#ifndef GL_OES_compressed_ETC1_RGB8_texture
#define GL_OES_compressed_ETC1_RGB8_texture 1
#endif

/* GL_OES_compressed_paletted_texture */
#ifndef GL_OES_compressed_paletted_texture
#define GL_OES_compressed_paletted_texture 1
#endif

/* GL_OES_depth24 */
#ifndef GL_OES_depth24
#define GL_OES_depth24 1
#endif

/* GL_OES_depth32 */
#ifndef GL_OES_depth32
#define GL_OES_depth32 1
#endif

/* GL_OES_depth_texture */
#ifndef GL_OES_depth_texture
#define GL_OES_depth_texture 1
#endif

/* GL_OES_EGL_image */
#ifndef GL_OES_EGL_image
#define GL_OES_EGL_image 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image);
GL_APICALL void GL_APIENTRY glEGLImageTargetRenderbufferStorageOES (GLenum target, GLeglImageOES image);
#endif
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, GLeglImageOES image);
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) (GLenum target, GLeglImageOES image);
#endif

/* GL_OES_element_index_uint */
#ifndef GL_OES_element_index_uint
#define GL_OES_element_index_uint 1
#endif

/* GL_OES_fbo_render_mipmap */
#ifndef GL_OES_fbo_render_mipmap
#define GL_OES_fbo_render_mipmap 1
#endif

/* GL_OES_fragment_precision_high */
#ifndef GL_OES_fragment_precision_high
#define GL_OES_fragment_precision_high 1
#endif

/* GL_OES_get_program_binary */
#ifndef GL_OES_get_program_binary
#define GL_OES_get_program_binary 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGetProgramBinaryOES (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
GL_APICALL void GL_APIENTRY glProgramBinaryOES (GLuint program, GLenum binaryFormat, const GLvoid *binary, GLint length);
#endif
typedef void (GL_APIENTRYP PFNGLGETPROGRAMBINARYOESPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
typedef void (GL_APIENTRYP PFNGLPROGRAMBINARYOESPROC) (GLuint program, GLenum binaryFormat, const GLvoid *binary, GLint length);
#endif

/* GL_OES_mapbuffer */
#ifndef GL_OES_mapbuffer
#define GL_OES_mapbuffer 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void* GL_APIENTRY glMapBufferOES (GLenum target, GLenum access);
GL_APICALL GLboolean GL_APIENTRY glUnmapBufferOES (GLenum target);
GL_APICALL void GL_APIENTRY glGetBufferPointervOES (GLenum target, GLenum pname, GLvoid** params);
#endif
typedef void* (GL_APIENTRYP PFNGLMAPBUFFEROESPROC) (GLenum target, GLenum access);
typedef GLboolean (GL_APIENTRYP PFNGLUNMAPBUFFEROESPROC) (GLenum target);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPOINTERVOESPROC) (GLenum target, GLenum pname, GLvoid** params);
#endif

/* GL_OES_packed_depth_stencil */
#ifndef GL_OES_packed_depth_stencil
#define GL_OES_packed_depth_stencil 1
#endif

/* GL_OES_rgb8_rgba8 */
#ifndef GL_OES_rgb8_rgba8
#define GL_OES_rgb8_rgba8 1
#endif

/* GL_OES_standard_derivatives */
#ifndef GL_OES_standard_derivatives
#define GL_OES_standard_derivatives 1
#endif

/* GL_OES_stencil1 */
#ifndef GL_OES_stencil1
#define GL_OES_stencil1 1
#endif

/* GL_OES_stencil4 */
#ifndef GL_OES_stencil4
#define GL_OES_stencil4 1
#endif

/* GL_OES_texture_3D */
#ifndef GL_OES_texture_3D
#define GL_OES_texture_3D 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glTexImage3DOES (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
GL_APICALL void GL_APIENTRY glTexSubImage3DOES (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
GL_APICALL void GL_APIENTRY glCopyTexSubImage3DOES (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glCompressedTexImage3DOES (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
GL_APICALL void GL_APIENTRY glCompressedTexSubImage3DOES (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
GL_APICALL void GL_APIENTRY glFramebufferTexture3DOES (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
#endif
typedef void (GL_APIENTRYP PFNGLTEXIMAGE3DOESPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (GL_APIENTRYP PFNGLTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (GL_APIENTRYP PFNGLCOPYTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXIMAGE3DOESPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTURE3DOES) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
#endif

/* GL_OES_texture_float */
#ifndef GL_OES_texture_float
#define GL_OES_texture_float 0
#endif

/* GL_OES_texture_float_linear */
#ifndef GL_OES_texture_float_linear
#define GL_OES_texture_float_linear 0
#endif

/* GL_OES_texture_half_float */
#ifndef GL_OES_texture_half_float
#define GL_OES_texture_half_float 0
#endif

/* GL_OES_texture_half_float_linear */
#ifndef GL_OES_texture_half_float_linear
#define GL_OES_texture_half_float_linear 0
#endif

/* GL_OES_texture_npot */
#ifndef GL_OES_texture_npot
#define GL_OES_texture_npot 1
#endif

/* GL_OES_vertex_array_object */
#ifndef GL_OES_vertex_array_object
#define GL_OES_vertex_array_object 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glBindVertexArrayOES (GLuint array);
GL_APICALL void GL_APIENTRY glDeleteVertexArraysOES (GLsizei n, const GLuint *arrays);
GL_APICALL void GL_APIENTRY glGenVertexArraysOES (GLsizei n, GLuint *arrays);
GL_APICALL GLboolean GL_APIENTRY glIsVertexArrayOES (GLuint array);
#endif
typedef void (GL_APIENTRYP PFNGLBINDVERTEXARRAYOESPROC) (GLuint array);
typedef void (GL_APIENTRYP PFNGLDELETEVERTEXARRAYSOESPROC) (GLsizei n, const GLuint *arrays);
typedef void (GL_APIENTRYP PFNGLGENVERTEXARRAYSOESPROC) (GLsizei n, GLuint *arrays);
typedef GLboolean (GL_APIENTRYP PFNGLISVERTEXARRAYOESPROC) (GLuint array);
#endif

/* GL_OES_vertex_half_float */
#ifndef GL_OES_vertex_half_float
#define GL_OES_vertex_half_float 1
#endif

/* GL_OES_vertex_type_10_10_10_2 */
#ifndef GL_OES_vertex_type_10_10_10_2
#define GL_OES_vertex_type_10_10_10_2 1
#endif

/*------------------------------------------------------------------------*
 * AMD extension functions
 *------------------------------------------------------------------------*/

/* GL_AMD_compressed_3DC_texture */
#ifndef GL_AMD_compressed_3DC_texture
#define GL_AMD_compressed_3DC_texture 1
#endif

/* GL_AMD_compressed_ATC_texture */
#ifndef GL_AMD_compressed_ATC_texture
#define GL_AMD_compressed_ATC_texture 1
#endif

/* AMD_performance_monitor */
#ifndef GL_AMD_performance_monitor
#define GL_AMD_performance_monitor 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGetPerfMonitorGroupsAMD (GLint *numGroups, GLsizei groupsSize, GLuint *groups);
GL_APICALL void GL_APIENTRY glGetPerfMonitorCountersAMD (GLuint group, GLint *numCounters, GLint *maxActiveCounters, GLsizei counterSize, GLuint *counters);
GL_APICALL void GL_APIENTRY glGetPerfMonitorGroupStringAMD (GLuint group, GLsizei bufSize, GLsizei *length, GLchar *groupString);
GL_APICALL void GL_APIENTRY glGetPerfMonitorCounterStringAMD (GLuint group, GLuint counter, GLsizei bufSize, GLsizei *length, GLchar *counterString);
GL_APICALL void GL_APIENTRY glGetPerfMonitorCounterInfoAMD (GLuint group, GLuint counter, GLenum pname, GLvoid *data);
GL_APICALL void GL_APIENTRY glGenPerfMonitorsAMD (GLsizei n, GLuint *monitors);
GL_APICALL void GL_APIENTRY glDeletePerfMonitorsAMD (GLsizei n, GLuint *monitors);
GL_APICALL void GL_APIENTRY glSelectPerfMonitorCountersAMD (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint *countersList);
GL_APICALL void GL_APIENTRY glBeginPerfMonitorAMD (GLuint monitor);
GL_APICALL void GL_APIENTRY glEndPerfMonitorAMD (GLuint monitor);
GL_APICALL void GL_APIENTRY glGetPerfMonitorCounterDataAMD (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint *data, GLint *bytesWritten);
#endif
typedef void (GL_APIENTRYP PFNGLGETPERFMONITORGROUPSAMDPROC) (GLint *numGroups, GLsizei groupsSize, GLuint *groups);
typedef void (GL_APIENTRYP PFNGLGETPERFMONITORCOUNTERSAMDPROC) (GLuint group, GLint *numCounters, GLint *maxActiveCounters, GLsizei counterSize, GLuint *counters);
typedef void (GL_APIENTRYP PFNGLGETPERFMONITORGROUPSTRINGAMDPROC) (GLuint group, GLsizei bufSize, GLsizei *length, GLchar *groupString);
typedef void (GL_APIENTRYP PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC) (GLuint group, GLuint counter, GLsizei bufSize, GLsizei *length, GLchar *counterString);
typedef void (GL_APIENTRYP PFNGLGETPERFMONITORCOUNTERINFOAMDPROC) (GLuint group, GLuint counter, GLenum pname, GLvoid *data);
typedef void (GL_APIENTRYP PFNGLGENPERFMONITORSAMDPROC) (GLsizei n, GLuint *monitors);
typedef void (GL_APIENTRYP PFNGLDELETEPERFMONITORSAMDPROC) (GLsizei n, GLuint *monitors);
typedef void (GL_APIENTRYP PFNGLSELECTPERFMONITORCOUNTERSAMDPROC) (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint *countersList);
typedef void (GL_APIENTRYP PFNGLBEGINPERFMONITORAMDPROC) (GLuint monitor);
typedef void (GL_APIENTRYP PFNGLENDPERFMONITORAMDPROC) (GLuint monitor);
typedef void (GL_APIENTRYP PFNGLGETPERFMONITORCOUNTERDATAAMDPROC) (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint *data, GLint *bytesWritten);
#endif

/* GL_AMD_program_binary_Z400 */
#ifndef GL_AMD_program_binary_Z400
#define GL_AMD_program_binary_Z400 1
#endif

/*------------------------------------------------------------------------*
 * EXT extension functions
 *------------------------------------------------------------------------*/

/* GL_EXT_blend_minmax */
#ifndef GL_EXT_blend_minmax
#define GL_EXT_blend_minmax 1
#endif

/* GL_EXT_discard_framebuffer */
#ifndef GL_EXT_discard_framebuffer
#define GL_EXT_discard_framebuffer 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glDiscardFramebufferEXT (GLenum target, GLsizei numAttachments, const GLenum *attachments);
#endif
typedef void (GL_APIENTRYP PFNGLDISCARDFRAMEBUFFEREXTPROC) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
#endif

#ifndef GL_EXT_multi_draw_arrays
#define GL_EXT_multi_draw_arrays 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glMultiDrawArraysEXT (GLenum, GLint *, GLsizei *, GLsizei);
GL_APICALL void GL_APIENTRY glMultiDrawElementsEXT (GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GL_APIENTRYP PFNGLMULTIDRAWARRAYSEXTPROC) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLMULTIDRAWELEMENTSEXTPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
#endif

/* GL_EXT_read_format_bgra */
#ifndef GL_EXT_read_format_bgra
#define GL_EXT_read_format_bgra 1
#endif

/* GL_EXT_texture_filter_anisotropic */
#ifndef GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_filter_anisotropic 1
#endif

/* GL_EXT_texture_format_BGRA8888 */
#ifndef GL_EXT_texture_format_BGRA8888
#define GL_EXT_texture_format_BGRA8888 1
#endif

/* GL_EXT_texture_type_2_10_10_10_REV */
#ifndef GL_EXT_texture_type_2_10_10_10_REV
#define GL_EXT_texture_type_2_10_10_10_REV 1
#endif

/* GL_EXT_texture_compression_dxt1 */
#ifndef GL_EXT_texture_compression_dxt1
#define GL_EXT_texture_compression_dxt1 1
#endif

/*------------------------------------------------------------------------*
 * IMG extension functions
 *------------------------------------------------------------------------*/

/* GL_IMG_program_binary */
#ifndef GL_IMG_program_binary
#define GL_IMG_program_binary 1
#endif

/* GL_IMG_read_format */
#ifndef GL_IMG_read_format
#define GL_IMG_read_format 1
#endif

/* GL_IMG_shader_binary */
#ifndef GL_IMG_shader_binary
#define GL_IMG_shader_binary 1
#endif

/* GL_IMG_texture_compression_pvrtc */
#ifndef GL_IMG_texture_compression_pvrtc
#define GL_IMG_texture_compression_pvrtc 1
#endif

/* GL_IMG_multisampled_render_to_texture */
#ifndef GL_IMG_multisampled_render_to_texture
#define GL_IMG_multisampled_render_to_texture 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glRenderbufferStorageMultisampleIMG (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
GL_APICALL void GL_APIENTRY glFramebufferTexture2DMultisampleIMG (GLenum, GLenum, GLenum, GLuint, GLint, GLsizei);
#endif
typedef void (GL_APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMG) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLCLIPPLANEXIMG) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
#endif

/*------------------------------------------------------------------------*
 * NV extension functions
 *------------------------------------------------------------------------*/

/* GL_NV_fence */
#ifndef GL_NV_fence
#define GL_NV_fence 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glDeleteFencesNV (GLsizei, const GLuint *);
GL_APICALL void GL_APIENTRY glGenFencesNV (GLsizei, GLuint *);
GL_APICALL GLboolean GL_APIENTRY glIsFenceNV (GLuint);
GL_APICALL GLboolean GL_APIENTRY glTestFenceNV (GLuint);
GL_APICALL void GL_APIENTRY glGetFenceivNV (GLuint, GLenum, GLint *);
GL_APICALL void GL_APIENTRY glFinishFenceNV (GLuint);
GL_APICALL void GL_APIENTRY glSetFenceNV (GLuint, GLenum);
#endif
typedef void (GL_APIENTRYP PFNGLDELETEFENCESNVPROC) (GLsizei n, const GLuint *fences);
typedef void (GL_APIENTRYP PFNGLGENFENCESNVPROC) (GLsizei n, GLuint *fences);
typedef GLboolean (GL_APIENTRYP PFNGLISFENCENVPROC) (GLuint fence);
typedef GLboolean (GL_APIENTRYP PFNGLTESTFENCENVPROC) (GLuint fence);
typedef void (GL_APIENTRYP PFNGLGETFENCEIVNVPROC) (GLuint fence, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLFINISHFENCENVPROC) (GLuint fence);
typedef void (GL_APIENTRYP PFNGLSETFENCENVPROC) (GLuint fence, GLenum condition);
#endif

/* GL_NV_coverage_sample */
#ifndef GL_NV_coverage_sample
#define GL_NV_coverage_sample 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glCoverageMaskNV (GLboolean mask);
GL_APICALL void GL_APIENTRY glCoverageOperationNV (GLenum operation);
#endif
typedef void (GL_APIENTRYP PFNGLCOVERAGEMASKNVPROC) (GLboolean mask);
typedef void (GL_APIENTRYP PFNGLCOVERAGEOPERATIONNVPROC) (GLenum operation);
#endif

/* GL_NV_depth_nonlinear */
#ifndef GL_NV_depth_nonlinear
#define GL_NV_depth_nonlinear 1
#endif

/*------------------------------------------------------------------------*
 * QCOM extension functions
 *------------------------------------------------------------------------*/

/* GL_QCOM_driver_control */
#ifndef GL_QCOM_driver_control
#define GL_QCOM_driver_control 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGetDriverControlsQCOM (GLint *num, GLsizei size, GLuint *driverControls);
GL_APICALL void GL_APIENTRY glGetDriverControlStringQCOM (GLuint driverControl, GLsizei bufSize, GLsizei *length, GLchar *driverControlString);
GL_APICALL void GL_APIENTRY glEnableDriverControlQCOM (GLuint driverControl);
GL_APICALL void GL_APIENTRY glDisableDriverControlQCOM (GLuint driverControl);
#endif
typedef void (GL_APIENTRYP PFNGLGETDRIVERCONTROLSQCOMPROC) (GLint *num, GLsizei size, GLuint *driverControls);
typedef void (GL_APIENTRYP PFNGLGETDRIVERCONTROLSTRINGQCOMPROC) (GLuint driverControl, GLsizei bufSize, GLsizei *length, GLchar *driverControlString);
typedef void (GL_APIENTRYP PFNGLENABLEDRIVERCONTROLQCOMPROC) (GLuint driverControl);
typedef void (GL_APIENTRYP PFNGLDISABLEDRIVERCONTROLQCOMPROC) (GLuint driverControl);
#endif

#ifndef GL_EXT_direct_state_access
#define GL_EXT_direct_state_access 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glClientAttribDefaultEXT (GLbitfield mask);
GL_APICALL void GL_APIENTRY glPushClientAttribDefaultEXT (GLbitfield mask);
GL_APICALL void GL_APIENTRY glMatrixLoadfEXT (GLenum mode, const GLfloat *m);
  //GL_APICALL void GL_APIENTRY glMatrixLoaddEXT (GLenum mode, const GLdouble *m);
GL_APICALL void GL_APIENTRY glMatrixMultfEXT (GLenum mode, const GLfloat *m);
  //GL_APICALL void GL_APIENTRY glMatrixMultdEXT (GLenum mode, const GLdouble *m);
GL_APICALL void GL_APIENTRY glMatrixLoadIdentityEXT (GLenum mode);
GL_APICALL void GL_APIENTRY glMatrixRotatefEXT (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
  //GL_APICALL void GL_APIENTRY glMatrixRotatedEXT (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
GL_APICALL void GL_APIENTRY glMatrixScalefEXT (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
  //GL_APICALL void GL_APIENTRY glMatrixScaledEXT (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
GL_APICALL void GL_APIENTRY glMatrixTranslatefEXT (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
  //GL_APICALL void GL_APIENTRY glMatrixTranslatedEXT (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
  //GL_APICALL void GL_APIENTRY glMatrixFrustumEXT (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
  //GL_APICALL void GL_APIENTRY glMatrixOrthoEXT (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GL_APICALL void GL_APIENTRY glMatrixPopEXT (GLenum mode);
GL_APICALL void GL_APIENTRY glMatrixPushEXT (GLenum mode);
GL_APICALL void GL_APIENTRY glMatrixLoadTransposefEXT (GLenum mode, const GLfloat *m);
  //GL_APICALL void GL_APIENTRY glMatrixLoadTransposedEXT (GLenum mode, const GLdouble *m);
GL_APICALL void GL_APIENTRY glMatrixMultTransposefEXT (GLenum mode, const GLfloat *m);
  //GL_APICALL void GL_APIENTRY glMatrixMultTransposedEXT (GLenum mode, const GLdouble *m);
GL_APICALL void GL_APIENTRY glTextureParameterfEXT (GLuint texture, GLenum target, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glTextureParameterfvEXT (GLuint texture, GLenum target, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glTextureParameteriEXT (GLuint texture, GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glTextureParameterivEXT (GLuint texture, GLenum target, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY glTextureImage1DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glTextureImage2DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glTextureSubImage1DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glTextureSubImage2DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glCopyTextureImage1DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
GL_APICALL void GL_APIENTRY glCopyTextureImage2DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GL_APICALL void GL_APIENTRY glCopyTextureSubImage1DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
GL_APICALL void GL_APIENTRY glCopyTextureSubImage2DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glGetTextureImageEXT (GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
GL_APICALL void GL_APIENTRY glGetTextureParameterfvEXT (GLuint texture, GLenum target, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetTextureParameterivEXT (GLuint texture, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetTextureLevelParameterfvEXT (GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetTextureLevelParameterivEXT (GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glTextureImage3DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glTextureSubImage3DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glCopyTextureSubImage3DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glMultiTexParameterfEXT (GLenum texunit, GLenum target, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glMultiTexParameterfvEXT (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glMultiTexParameteriEXT (GLenum texunit, GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glMultiTexParameterivEXT (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY glMultiTexImage1DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glMultiTexImage2DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glMultiTexSubImage1DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glMultiTexSubImage2DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glCopyMultiTexImage1DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
GL_APICALL void GL_APIENTRY glCopyMultiTexImage2DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GL_APICALL void GL_APIENTRY glCopyMultiTexSubImage1DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
GL_APICALL void GL_APIENTRY glCopyMultiTexSubImage2DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glGetMultiTexImageEXT (GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
GL_APICALL void GL_APIENTRY glGetMultiTexParameterfvEXT (GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetMultiTexParameterivEXT (GLenum texunit, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetMultiTexLevelParameterfvEXT (GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetMultiTexLevelParameterivEXT (GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glMultiTexImage3DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glMultiTexSubImage3DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glCopyMultiTexSubImage3DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glBindMultiTextureEXT (GLenum texunit, GLenum target, GLuint texture);
GL_APICALL void GL_APIENTRY glEnableClientStateIndexedEXT (GLenum array, GLuint index);
GL_APICALL void GL_APIENTRY glDisableClientStateIndexedEXT (GLenum array, GLuint index);
GL_APICALL void GL_APIENTRY glMultiTexCoordPointerEXT (GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GL_APICALL void GL_APIENTRY glMultiTexEnvfEXT (GLenum texunit, GLenum target, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glMultiTexEnvfvEXT (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glMultiTexEnviEXT (GLenum texunit, GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glMultiTexEnvivEXT (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
  //GL_APICALL void GL_APIENTRY glMultiTexGendEXT (GLenum texunit, GLenum coord, GLenum pname, GLdouble param);
  //GL_APICALL void GL_APIENTRY glMultiTexGendvEXT (GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params);
GL_APICALL void GL_APIENTRY glMultiTexGenfEXT (GLenum texunit, GLenum coord, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glMultiTexGenfvEXT (GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glMultiTexGeniEXT (GLenum texunit, GLenum coord, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glMultiTexGenivEXT (GLenum texunit, GLenum coord, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY glGetMultiTexEnvfvEXT (GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetMultiTexEnvivEXT (GLenum texunit, GLenum target, GLenum pname, GLint *params);
  //GL_APICALL void GL_APIENTRY glGetMultiTexGendvEXT (GLenum texunit, GLenum coord, GLenum pname, GLdouble *params);
GL_APICALL void GL_APIENTRY glGetMultiTexGenfvEXT (GLenum texunit, GLenum coord, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetMultiTexGenivEXT (GLenum texunit, GLenum coord, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetFloatIndexedvEXT (GLenum target, GLuint index, GLfloat *data);
  //GL_APICALL void GL_APIENTRY glGetDoubleIndexedvEXT (GLenum target, GLuint index, GLdouble *data);
GL_APICALL void GL_APIENTRY glGetPointerIndexedvEXT (GLenum target, GLuint index, GLvoid* *data);
GL_APICALL void GL_APIENTRY glCompressedTextureImage3DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glCompressedTextureImage2DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glCompressedTextureImage1DEXT (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glCompressedTextureSubImage3DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glCompressedTextureSubImage2DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glCompressedTextureSubImage1DEXT (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glGetCompressedTextureImageEXT (GLuint texture, GLenum target, GLint lod, GLvoid *img);
GL_APICALL void GL_APIENTRY glCompressedMultiTexImage3DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glCompressedMultiTexImage2DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glCompressedMultiTexImage1DEXT (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glCompressedMultiTexSubImage3DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glCompressedMultiTexSubImage2DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glCompressedMultiTexSubImage1DEXT (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *bits);
GL_APICALL void GL_APIENTRY glGetCompressedMultiTexImageEXT (GLenum texunit, GLenum target, GLint lod, GLvoid *img);
GL_APICALL void GL_APIENTRY glNamedProgramStringEXT (GLuint program, GLenum target, GLenum format, GLsizei len, const GLvoid *string);
  //GL_APICALL void GL_APIENTRY glNamedProgramLocalParameter4dEXT (GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  //GL_APICALL void GL_APIENTRY glNamedProgramLocalParameter4dvEXT (GLuint program, GLenum target, GLuint index, const GLdouble *params);
GL_APICALL void GL_APIENTRY glNamedProgramLocalParameter4fEXT (GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_APICALL void GL_APIENTRY glNamedProgramLocalParameter4fvEXT (GLuint program, GLenum target, GLuint index, const GLfloat *params);
  //GL_APICALL void GL_APIENTRY glGetNamedProgramLocalParameterdvEXT (GLuint program, GLenum target, GLuint index, GLdouble *params);
GL_APICALL void GL_APIENTRY glGetNamedProgramLocalParameterfvEXT (GLuint program, GLenum target, GLuint index, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetNamedProgramivEXT (GLuint program, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetNamedProgramStringEXT (GLuint program, GLenum target, GLenum pname, GLvoid *string);
GL_APICALL void GL_APIENTRY glNamedProgramLocalParameters4fvEXT (GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat *params);
GL_APICALL void GL_APIENTRY glNamedProgramLocalParameterI4iEXT (GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
GL_APICALL void GL_APIENTRY glNamedProgramLocalParameterI4ivEXT (GLuint program, GLenum target, GLuint index, const GLint *params);
GL_APICALL void GL_APIENTRY glNamedProgramLocalParametersI4ivEXT (GLuint program, GLenum target, GLuint index, GLsizei count, const GLint *params);
GL_APICALL void GL_APIENTRY glNamedProgramLocalParameterI4uiEXT (GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
GL_APICALL void GL_APIENTRY glNamedProgramLocalParameterI4uivEXT (GLuint program, GLenum target, GLuint index, const GLuint *params);
GL_APICALL void GL_APIENTRY glNamedProgramLocalParametersI4uivEXT (GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint *params);
GL_APICALL void GL_APIENTRY glGetNamedProgramLocalParameterIivEXT (GLuint program, GLenum target, GLuint index, GLint *params);
GL_APICALL void GL_APIENTRY glGetNamedProgramLocalParameterIuivEXT (GLuint program, GLenum target, GLuint index, GLuint *params);
GL_APICALL void GL_APIENTRY glTextureParameterIivEXT (GLuint texture, GLenum target, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY glTextureParameterIuivEXT (GLuint texture, GLenum target, GLenum pname, const GLuint *params);
GL_APICALL void GL_APIENTRY glGetTextureParameterIivEXT (GLuint texture, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetTextureParameterIuivEXT (GLuint texture, GLenum target, GLenum pname, GLuint *params);
GL_APICALL void GL_APIENTRY glMultiTexParameterIivEXT (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY glMultiTexParameterIuivEXT (GLenum texunit, GLenum target, GLenum pname, const GLuint *params);
GL_APICALL void GL_APIENTRY glGetMultiTexParameterIivEXT (GLenum texunit, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetMultiTexParameterIuivEXT (GLenum texunit, GLenum target, GLenum pname, GLuint *params);
GL_APICALL void GL_APIENTRY glProgramUniform1fEXT (GLuint program, GLint location, GLfloat v0);
GL_APICALL void GL_APIENTRY glProgramUniform2fEXT (GLuint program, GLint location, GLfloat v0, GLfloat v1);
GL_APICALL void GL_APIENTRY glProgramUniform3fEXT (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GL_APICALL void GL_APIENTRY glProgramUniform4fEXT (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GL_APICALL void GL_APIENTRY glProgramUniform1iEXT (GLuint program, GLint location, GLint v0);
GL_APICALL void GL_APIENTRY glProgramUniform2iEXT (GLuint program, GLint location, GLint v0, GLint v1);
GL_APICALL void GL_APIENTRY glProgramUniform3iEXT (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
GL_APICALL void GL_APIENTRY glProgramUniform4iEXT (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GL_APICALL void GL_APIENTRY glProgramUniform1fvEXT (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniform2fvEXT (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniform3fvEXT (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniform4fvEXT (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniform1ivEXT (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glProgramUniform2ivEXT (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glProgramUniform3ivEXT (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glProgramUniform4ivEXT (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix2fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix3fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix4fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix2x3fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix3x2fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix2x4fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix4x2fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix3x4fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix4x3fvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniform1uiEXT (GLuint program, GLint location, GLuint v0);
GL_APICALL void GL_APIENTRY glProgramUniform2uiEXT (GLuint program, GLint location, GLuint v0, GLuint v1);
GL_APICALL void GL_APIENTRY glProgramUniform3uiEXT (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
GL_APICALL void GL_APIENTRY glProgramUniform4uiEXT (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GL_APICALL void GL_APIENTRY glProgramUniform1uivEXT (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glProgramUniform2uivEXT (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glProgramUniform3uivEXT (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glProgramUniform4uivEXT (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glNamedBufferDataEXT (GLuint buffer, GLsizeiptr size, const GLvoid *data, GLenum usage);
GL_APICALL void GL_APIENTRY glNamedBufferSubDataEXT (GLuint buffer, GLintptr offset, GLsizeiptr size, const GLvoid *data);
GL_APICALL GLvoid* GL_APIENTRY glMapNamedBufferEXT (GLuint buffer, GLenum access);
GL_APICALL GLboolean GL_APIENTRY glUnmapNamedBufferEXT (GLuint buffer);
GL_APICALL GLvoid* GL_APIENTRY glMapNamedBufferRangeEXT (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
GL_APICALL void GL_APIENTRY glFlushMappedNamedBufferRangeEXT (GLuint buffer, GLintptr offset, GLsizeiptr length);
GL_APICALL void GL_APIENTRY glNamedCopyBufferSubDataEXT (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
GL_APICALL void GL_APIENTRY glGetNamedBufferParameterivEXT (GLuint buffer, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetNamedBufferPointervEXT (GLuint buffer, GLenum pname, GLvoid* *params);
GL_APICALL void GL_APIENTRY glGetNamedBufferSubDataEXT (GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid *data);
GL_APICALL void GL_APIENTRY glTextureBufferEXT (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer);
GL_APICALL void GL_APIENTRY glMultiTexBufferEXT (GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer);
GL_APICALL void GL_APIENTRY glNamedRenderbufferStorageEXT (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glGetNamedRenderbufferParameterivEXT (GLuint renderbuffer, GLenum pname, GLint *params);
GL_APICALL GLenum GL_APIENTRY glCheckNamedFramebufferStatusEXT (GLuint framebuffer, GLenum target);
GL_APICALL void GL_APIENTRY glNamedFramebufferTexture1DEXT (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GL_APICALL void GL_APIENTRY glNamedFramebufferTexture2DEXT (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GL_APICALL void GL_APIENTRY glNamedFramebufferTexture3DEXT (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
GL_APICALL void GL_APIENTRY glNamedFramebufferRenderbufferEXT (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GL_APICALL void GL_APIENTRY glGetNamedFramebufferAttachmentParameterivEXT (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGenerateTextureMipmapEXT (GLuint texture, GLenum target);
GL_APICALL void GL_APIENTRY glGenerateMultiTexMipmapEXT (GLenum texunit, GLenum target);
GL_APICALL void GL_APIENTRY glFramebufferDrawBufferEXT (GLuint framebuffer, GLenum mode);
GL_APICALL void GL_APIENTRY glFramebufferDrawBuffersEXT (GLuint framebuffer, GLsizei n, const GLenum *bufs);
GL_APICALL void GL_APIENTRY glFramebufferReadBufferEXT (GLuint framebuffer, GLenum mode);
GL_APICALL void GL_APIENTRY glGetFramebufferParameterivEXT (GLuint framebuffer, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glNamedRenderbufferStorageMultisampleEXT (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glNamedRenderbufferStorageMultisampleCoverageEXT (GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glNamedFramebufferTextureEXT (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
GL_APICALL void GL_APIENTRY glNamedFramebufferTextureLayerEXT (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
GL_APICALL void GL_APIENTRY glNamedFramebufferTextureFaceEXT (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face);
GL_APICALL void GL_APIENTRY glTextureRenderbufferEXT (GLuint texture, GLenum target, GLuint renderbuffer);
GL_APICALL void GL_APIENTRY glMultiTexRenderbufferEXT (GLenum texunit, GLenum target, GLuint renderbuffer);
  //GL_APICALL void GL_APIENTRY glProgramUniform1dEXT (GLuint program, GLint location, GLdouble x);
  //GL_APICALL void GL_APIENTRY glProgramUniform2dEXT (GLuint program, GLint location, GLdouble x, GLdouble y);
  //GL_APICALL void GL_APIENTRY glProgramUniform3dEXT (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z);
  //GL_APICALL void GL_APIENTRY glProgramUniform4dEXT (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  //GL_APICALL void GL_APIENTRY glProgramUniform1dvEXT (GLuint program, GLint location, GLsizei count, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniform2dvEXT (GLuint program, GLint location, GLsizei count, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniform3dvEXT (GLuint program, GLint location, GLsizei count, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniform4dvEXT (GLuint program, GLint location, GLsizei count, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniformMatrix2dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniformMatrix3dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniformMatrix4dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniformMatrix2x3dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniformMatrix2x4dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniformMatrix3x2dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniformMatrix3x4dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniformMatrix4x2dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
  //GL_APICALL void GL_APIENTRY glProgramUniformMatrix4x3dvEXT (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GL_APIENTRYP PFNGLCLIENTATTRIBDEFAULTEXTPROC) (GLbitfield mask);
typedef void (GL_APIENTRYP PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC) (GLbitfield mask);
typedef void (GL_APIENTRYP PFNGLMATRIXLOADFEXTPROC) (GLenum mode, const GLfloat *m);
  //typedef void (GL_APIENTRYP PFNGLMATRIXLOADDEXTPROC) (GLenum mode, const GLdouble *m);
typedef void (GL_APIENTRYP PFNGLMATRIXMULTFEXTPROC) (GLenum mode, const GLfloat *m);
  //typedef void (GL_APIENTRYP PFNGLMATRIXMULTDEXTPROC) (GLenum mode, const GLdouble *m);
typedef void (GL_APIENTRYP PFNGLMATRIXLOADIDENTITYEXTPROC) (GLenum mode);
typedef void (GL_APIENTRYP PFNGLMATRIXROTATEFEXTPROC) (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
  //typedef void (GL_APIENTRYP PFNGLMATRIXROTATEDEXTPROC) (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
typedef void (GL_APIENTRYP PFNGLMATRIXSCALEFEXTPROC) (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
  //typedef void (GL_APIENTRYP PFNGLMATRIXSCALEDEXTPROC) (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
typedef void (GL_APIENTRYP PFNGLMATRIXTRANSLATEFEXTPROC) (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
  //typedef void (GL_APIENTRYP PFNGLMATRIXTRANSLATEDEXTPROC) (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
  //typedef void (GL_APIENTRYP PFNGLMATRIXFRUSTUMEXTPROC) (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
  //typedef void (GL_APIENTRYP PFNGLMATRIXORTHOEXTPROC) (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef void (GL_APIENTRYP PFNGLMATRIXPOPEXTPROC) (GLenum mode);
typedef void (GL_APIENTRYP PFNGLMATRIXPUSHEXTPROC) (GLenum mode);
typedef void (GL_APIENTRYP PFNGLMATRIXLOADTRANSPOSEFEXTPROC) (GLenum mode, const GLfloat *m);
  //typedef void (GL_APIENTRYP PFNGLMATRIXLOADTRANSPOSEDEXTPROC) (GLenum mode, const GLdouble *m);
typedef void (GL_APIENTRYP PFNGLMATRIXMULTTRANSPOSEFEXTPROC) (GLenum mode, const GLfloat *m);
  //typedef void (GL_APIENTRYP PFNGLMATRIXMULTTRANSPOSEDEXTPROC) (GLenum mode, const GLdouble *m);
typedef void (GL_APIENTRYP PFNGLTEXTUREPARAMETERFEXTPROC) (GLuint texture, GLenum target, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLTEXTUREPARAMETERFVEXTPROC) (GLuint texture, GLenum target, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLTEXTUREPARAMETERIEXTPROC) (GLuint texture, GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLTEXTUREPARAMETERIVEXTPROC) (GLuint texture, GLenum target, GLenum pname, const GLint *params);
typedef void (GL_APIENTRYP PFNGLTEXTUREIMAGE1DEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLTEXTUREIMAGE2DEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLTEXTURESUBIMAGE1DEXTPROC) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLTEXTURESUBIMAGE2DEXTPROC) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLCOPYTEXTUREIMAGE1DEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (GL_APIENTRYP PFNGLCOPYTEXTUREIMAGE2DEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (GL_APIENTRYP PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (GL_APIENTRYP PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLGETTEXTUREIMAGEEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLGETTEXTUREPARAMETERFVEXTPROC) (GLuint texture, GLenum target, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETTEXTUREPARAMETERIVEXTPROC) (GLuint texture, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLTEXTUREIMAGE3DEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLTEXTURESUBIMAGE3DEXTPROC) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLMULTITEXPARAMETERFEXTPROC) (GLenum texunit, GLenum target, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLMULTITEXPARAMETERFVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLMULTITEXPARAMETERIEXTPROC) (GLenum texunit, GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLMULTITEXPARAMETERIVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
typedef void (GL_APIENTRYP PFNGLMULTITEXIMAGE1DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLMULTITEXIMAGE2DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLMULTITEXSUBIMAGE1DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLMULTITEXSUBIMAGE2DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLCOPYMULTITEXIMAGE1DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (GL_APIENTRYP PFNGLCOPYMULTITEXIMAGE2DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (GL_APIENTRYP PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (GL_APIENTRYP PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXIMAGEEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXPARAMETERFVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXPARAMETERIVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLMULTITEXIMAGE3DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLMULTITEXSUBIMAGE3DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLBINDMULTITEXTUREEXTPROC) (GLenum texunit, GLenum target, GLuint texture);
typedef void (GL_APIENTRYP PFNGLENABLECLIENTSTATEINDEXEDEXTPROC) (GLenum array, GLuint index);
typedef void (GL_APIENTRYP PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC) (GLenum array, GLuint index);
typedef void (GL_APIENTRYP PFNGLMULTITEXCOORDPOINTEREXTPROC) (GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (GL_APIENTRYP PFNGLMULTITEXENVFEXTPROC) (GLenum texunit, GLenum target, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLMULTITEXENVFVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLMULTITEXENVIEXTPROC) (GLenum texunit, GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLMULTITEXENVIVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
  //typedef void (GL_APIENTRYP PFNGLMULTITEXGENDEXTPROC) (GLenum texunit, GLenum coord, GLenum pname, GLdouble param);
  //typedef void (GL_APIENTRYP PFNGLMULTITEXGENDVEXTPROC) (GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params);
typedef void (GL_APIENTRYP PFNGLMULTITEXGENFEXTPROC) (GLenum texunit, GLenum coord, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLMULTITEXGENFVEXTPROC) (GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLMULTITEXGENIEXTPROC) (GLenum texunit, GLenum coord, GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLMULTITEXGENIVEXTPROC) (GLenum texunit, GLenum coord, GLenum pname, const GLint *params);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXENVFVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXENVIVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, GLint *params);
  //typedef void (GL_APIENTRYP PFNGLGETMULTITEXGENDVEXTPROC) (GLenum texunit, GLenum coord, GLenum pname, GLdouble *params);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXGENFVEXTPROC) (GLenum texunit, GLenum coord, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXGENIVEXTPROC) (GLenum texunit, GLenum coord, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETFLOATINDEXEDVEXTPROC) (GLenum target, GLuint index, GLfloat *data);
  //typedef void (GL_APIENTRYP PFNGLGETDOUBLEINDEXEDVEXTPROC) (GLenum target, GLuint index, GLdouble *data);
typedef void (GL_APIENTRYP PFNGLGETPOINTERINDEXEDVEXTPROC) (GLenum target, GLuint index, GLvoid* *data);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC) (GLuint texture, GLenum target, GLint lod, GLvoid *img);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *bits);
typedef void (GL_APIENTRYP PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC) (GLenum texunit, GLenum target, GLint lod, GLvoid *img);
typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMSTRINGEXTPROC) (GLuint program, GLenum target, GLenum format, GLsizei len, const GLvoid *string);
  //typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC) (GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  //typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC) (GLuint program, GLenum target, GLuint index, const GLdouble *params);
typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC) (GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC) (GLuint program, GLenum target, GLuint index, const GLfloat *params);
  //typedef void (GL_APIENTRYP PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC) (GLuint program, GLenum target, GLuint index, GLdouble *params);
typedef void (GL_APIENTRYP PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC) (GLuint program, GLenum target, GLuint index, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETNAMEDPROGRAMIVEXTPROC) (GLuint program, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETNAMEDPROGRAMSTRINGEXTPROC) (GLuint program, GLenum target, GLenum pname, GLvoid *string);
typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC) (GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC) (GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC) (GLuint program, GLenum target, GLuint index, const GLint *params);
typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC) (GLuint program, GLenum target, GLuint index, GLsizei count, const GLint *params);
typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC) (GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC) (GLuint program, GLenum target, GLuint index, const GLuint *params);
typedef void (GL_APIENTRYP PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC) (GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint *params);
typedef void (GL_APIENTRYP PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC) (GLuint program, GLenum target, GLuint index, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC) (GLuint program, GLenum target, GLuint index, GLuint *params);
typedef void (GL_APIENTRYP PFNGLTEXTUREPARAMETERIIVEXTPROC) (GLuint texture, GLenum target, GLenum pname, const GLint *params);
typedef void (GL_APIENTRYP PFNGLTEXTUREPARAMETERIUIVEXTPROC) (GLuint texture, GLenum target, GLenum pname, const GLuint *params);
typedef void (GL_APIENTRYP PFNGLGETTEXTUREPARAMETERIIVEXTPROC) (GLuint texture, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETTEXTUREPARAMETERIUIVEXTPROC) (GLuint texture, GLenum target, GLenum pname, GLuint *params);
typedef void (GL_APIENTRYP PFNGLMULTITEXPARAMETERIIVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
typedef void (GL_APIENTRYP PFNGLMULTITEXPARAMETERIUIVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, const GLuint *params);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXPARAMETERIIVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETMULTITEXPARAMETERIUIVEXTPROC) (GLenum texunit, GLenum target, GLenum pname, GLuint *params);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM1FEXTPROC) (GLuint program, GLint location, GLfloat v0);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM2FEXTPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM3FEXTPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM4FEXTPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM1IEXTPROC) (GLuint program, GLint location, GLint v0);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM2IEXTPROC) (GLuint program, GLint location, GLint v0, GLint v1);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM3IEXTPROC) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM4IEXTPROC) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM1FVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM2FVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM3FVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM4FVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM1IVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM2IVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM3IVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM4IVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM1UIEXTPROC) (GLuint program, GLint location, GLuint v0);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM2UIEXTPROC) (GLuint program, GLint location, GLuint v0, GLuint v1);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM3UIEXTPROC) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM4UIEXTPROC) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM1UIVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM2UIVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM3UIVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM4UIVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (GL_APIENTRYP PFNGLNAMEDBUFFERDATAEXTPROC) (GLuint buffer, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void (GL_APIENTRYP PFNGLNAMEDBUFFERSUBDATAEXTPROC) (GLuint buffer, GLintptr offset, GLsizeiptr size, const GLvoid *data);
typedef GLvoid* (GL_APIENTRYP PFNGLMAPNAMEDBUFFEREXTPROC) (GLuint buffer, GLenum access);
typedef GLboolean (GL_APIENTRYP PFNGLUNMAPNAMEDBUFFEREXTPROC) (GLuint buffer);
typedef GLvoid* (GL_APIENTRYP PFNGLMAPNAMEDBUFFERRANGEEXTPROC) (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (GL_APIENTRYP PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC) (GLuint buffer, GLintptr offset, GLsizeiptr length);
typedef void (GL_APIENTRYP PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC) (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef void (GL_APIENTRYP PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC) (GLuint buffer, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETNAMEDBUFFERPOINTERVEXTPROC) (GLuint buffer, GLenum pname, GLvoid* *params);
typedef void (GL_APIENTRYP PFNGLGETNAMEDBUFFERSUBDATAEXTPROC) (GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid *data);
typedef void (GL_APIENTRYP PFNGLTEXTUREBUFFEREXTPROC) (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer);
typedef void (GL_APIENTRYP PFNGLMULTITEXBUFFEREXTPROC) (GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer);
typedef void (GL_APIENTRYP PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC) (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC) (GLuint renderbuffer, GLenum pname, GLint *params);
typedef GLenum (GL_APIENTRYP PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC) (GLuint framebuffer, GLenum target);
typedef void (GL_APIENTRYP PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC) (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (GL_APIENTRYP PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC) (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (GL_APIENTRYP PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC) (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (GL_APIENTRYP PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC) (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (GL_APIENTRYP PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGENERATETEXTUREMIPMAPEXTPROC) (GLuint texture, GLenum target);
typedef void (GL_APIENTRYP PFNGLGENERATEMULTITEXMIPMAPEXTPROC) (GLenum texunit, GLenum target);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC) (GLuint framebuffer, GLenum mode);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC) (GLuint framebuffer, GLsizei n, const GLenum *bufs);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERREADBUFFEREXTPROC) (GLuint framebuffer, GLenum mode);
typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC) (GLuint framebuffer, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC) (GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
typedef void (GL_APIENTRYP PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void (GL_APIENTRYP PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face);
typedef void (GL_APIENTRYP PFNGLTEXTURERENDERBUFFEREXTPROC) (GLuint texture, GLenum target, GLuint renderbuffer);
typedef void (GL_APIENTRYP PFNGLMULTITEXRENDERBUFFEREXTPROC) (GLenum texunit, GLenum target, GLuint renderbuffer);
  /*typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM1DEXTPROC) (GLuint program, GLint location, GLdouble x);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM2DEXTPROC) (GLuint program, GLint location, GLdouble x, GLdouble y);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM3DEXTPROC) (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM4DEXTPROC) (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM1DVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM2DVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM3DVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM4DVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);*/
#endif

#ifndef GL_VERSION_1_5
#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765
#define GL_QUERY_COUNTER_BITS             0x8864
#define GL_CURRENT_QUERY                  0x8865
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY                      0x88B8
#define GL_WRITE_ONLY                     0x88B9
#define GL_READ_WRITE                     0x88BA
#define GL_BUFFER_ACCESS                  0x88BB
#define GL_BUFFER_MAPPED                  0x88BC
#define GL_BUFFER_MAP_POINTER             0x88BD
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
#define GL_SAMPLES_PASSED                 0x8914
#endif

#ifndef GL_ARB_get_program_binary
#define GL_ARB_get_program_binary 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGetProgramBinary (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
GL_APICALL void GL_APIENTRY glProgramBinary (GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length);
GL_APICALL void GL_APIENTRY glProgramParameteri (GLuint program, GLenum pname, GLint value);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GL_APIENTRYP PFNGLGETPROGRAMBINARYPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
typedef void (GL_APIENTRYP PFNGLPROGRAMBINARYPROC) (GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length);
typedef void (GL_APIENTRYP PFNGLPROGRAMPARAMETERIPROC) (GLuint program, GLenum pname, GLint value);
#endif

#ifndef GL_VERSION_1_5
#define GL_VERSION_1_5 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGenQueries (GLsizei n, GLuint *ids);
GL_APICALL void GL_APIENTRY glDeleteQueries (GLsizei n, const GLuint *ids);
GL_APICALL GLboolean GL_APIENTRY glIsQuery (GLuint id);
GL_APICALL void GL_APIENTRY glBeginQuery (GLenum target, GLuint id);
GL_APICALL void GL_APIENTRY glEndQuery (GLenum target);
GL_APICALL void GL_APIENTRY glGetQueryiv (GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectiv (GLuint id, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint *params);
GL_APICALL void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer);
GL_APICALL void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers);
GL_APICALL void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers);
GL_APICALL GLboolean GL_APIENTRY glIsBuffer (GLuint buffer);
GL_APICALL void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
GL_APICALL void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
GL_APICALL void GL_APIENTRY glGetBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
GL_APICALL GLvoid* GL_APIENTRY glMapBuffer (GLenum target, GLenum access);
GL_APICALL GLboolean GL_APIENTRY glUnmapBuffer (GLenum target);
GL_APICALL void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetBufferPointerv (GLenum target, GLenum pname, GLvoid* *params);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GL_APIENTRYP PFNGLGENQUERIESPROC) (GLsizei n, GLuint *ids);
typedef void (GL_APIENTRYP PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean (GL_APIENTRYP PFNGLISQUERYPROC) (GLuint id);
typedef void (GL_APIENTRYP PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
typedef void (GL_APIENTRYP PFNGLENDQUERYPROC) (GLenum target);
typedef void (GL_APIENTRYP PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint *params);
typedef void (GL_APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (GL_APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (GL_APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef GLboolean (GL_APIENTRYP PFNGLISBUFFERPROC) (GLuint buffer);
typedef void (GL_APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void (GL_APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
typedef void (GL_APIENTRYP PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
typedef GLvoid* (GL_APIENTRYP PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
typedef GLboolean (GL_APIENTRYP PFNGLUNMAPBUFFERPROC) (GLenum target);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname, GLvoid* *params);
#endif

#ifndef GL_VERSION_3_1
#define GL_VERSION_3_1 1
/* OpenGL 3.1 also reuses entry points from these extensions: */
/* ARB_copy_buffer */
/* ARB_uniform_buffer_object */
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
GL_APICALL void GL_APIENTRY glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);
GL_APICALL void GL_APIENTRY glTexBuffer (GLenum target, GLenum internalformat, GLuint buffer);
GL_APICALL void GL_APIENTRY glPrimitiveRestartIndex (GLuint index);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDPROC) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDPROC) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLTEXBUFFERPROC) (GLenum target, GLenum internalformat, GLuint buffer);
typedef void (GL_APIENTRYP PFNGLPRIMITIVERESTARTINDEXPROC) (GLuint index);
#endif

#ifndef GL_VERSION_3_2
#define GL_VERSION_3_2 1
/* OpenGL 3.2 also reuses entry points from these extensions: */
/* ARB_draw_elements_base_vertex */
/* ARB_provoking_vertex */
/* ARB_sync */
/* ARB_texture_multisample */
#ifdef GL_GLEXT_PROTOTYPES
  //GL_APICALL void GL_APIENTRY glGetInteger64i_v (GLenum target, GLuint index, GLint64 *data);
  //GL_APICALL void GL_APIENTRY glGetBufferParameteri64v (GLenum target, GLenum pname, GLint64 *params);
GL_APICALL void GL_APIENTRY glFramebufferTexture (GLenum target, GLenum attachment, GLuint texture, GLint level);
#endif /* GL_GLEXT_PROTOTYPES */
  //typedef void (GL_APIENTRYP PFNGLGETINTEGER64I_VPROC) (GLenum target, GLuint index, GLint64 *data);
  //typedef void (GL_APIENTRYP PFNGLGETBUFFERPARAMETERI64VPROC) (GLenum target, GLenum pname, GLint64 *params);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTUREPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level);
#endif

#ifndef GL_VERSION_3_3
#define GL_VERSION_3_3 1
/* OpenGL 3.3 also reuses entry points from these extensions: */
/* ARB_blend_func_extended */
/* ARB_sampler_objects */
/* ARB_explicit_attrib_location, but it has none */
/* ARB_occlusion_query2 (no entry points) */
/* ARB_shader_bit_encoding (no entry points) */
/* ARB_texture_rgb10_a2ui (no entry points) */
/* ARB_texture_swizzle (no entry points) */
/* ARB_timer_query */
/* ARB_vertex_type_2_10_10_10_rev */
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glVertexAttribDivisor (GLuint index, GLuint divisor);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISORPROC) (GLuint index, GLuint divisor);
#endif

#ifndef GL_VERSION_4_0
#define GL_VERSION_4_0 1
/* OpenGL 4.0 also reuses entry points from these extensions: */
/* ARB_texture_query_lod (no entry points) */
/* ARB_draw_indirect */
/* ARB_gpu_shader5 (no entry points) */
/* ARB_gpu_shader_fp64 */
/* ARB_shader_subroutine */
/* ARB_tessellation_shader */
/* ARB_texture_buffer_object_rgb32 (no entry points) */
/* ARB_texture_cube_map_array (no entry points) */
/* ARB_texture_gather (no entry points) */
/* ARB_transform_feedback2 */
/* ARB_transform_feedback3 */
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glMinSampleShading (GLclampf value);
GL_APICALL void GL_APIENTRY glBlendEquationi (GLuint buf, GLenum mode);
GL_APICALL void GL_APIENTRY glBlendEquationSeparatei (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
GL_APICALL void GL_APIENTRY glBlendFunci (GLuint buf, GLenum src, GLenum dst);
GL_APICALL void GL_APIENTRY glBlendFuncSeparatei (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GL_APIENTRYP PFNGLMINSAMPLESHADINGPROC) (GLclampf value);
typedef void (GL_APIENTRYP PFNGLBLENDEQUATIONIPROC) (GLuint buf, GLenum mode);
typedef void (GL_APIENTRYP PFNGLBLENDEQUATIONSEPARATEIPROC) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
typedef void (GL_APIENTRYP PFNGLBLENDFUNCIPROC) (GLuint buf, GLenum src, GLenum dst);
typedef void (GL_APIENTRYP PFNGLBLENDFUNCSEPARATEIPROC) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
#endif

#ifndef GL_VERSION_4_1
#define GL_VERSION_4_1 1
/* OpenGL 4.1 also reuses entry points from these extensions: */
/* ARB_ES2_compatibility */
/* ARB_get_program_binary */
/* ARB_separate_shader_objects */
/* ARB_shader_precision (no entry points) */
/* ARB_vertex_attrib_64bit */
/* ARB_viewport_array */
#endif

#ifndef GL_ARB_occlusion_query2
#define GL_ANY_SAMPLES_PASSED             0x8C2F
#endif

#ifndef GL_ARB_occlusion_query
#define GL_QUERY_COUNTER_BITS_ARB         0x8864
#define GL_CURRENT_QUERY_ARB              0x8865
#define GL_QUERY_RESULT_ARB               0x8866
#define GL_QUERY_RESULT_AVAILABLE_ARB     0x8867
#define GL_SAMPLES_PASSED_ARB             0x8914
#endif

/* GL_QCOM_extended_get */
#ifndef GL_QCOM_extended_get
#define GL_QCOM_extended_get 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glExtGetTexturesQCOM (GLuint *textures, GLint maxTextures, GLint *numTextures);
GL_APICALL void GL_APIENTRY glExtGetBuffersQCOM (GLuint *buffers, GLint maxBuffers, GLint *numBuffers);
GL_APICALL void GL_APIENTRY glExtGetRenderbuffersQCOM (GLuint *renderbuffers, GLint maxRenderbuffers, GLint *numRenderbuffers);
GL_APICALL void GL_APIENTRY glExtGetFramebuffersQCOM (GLuint *framebuffers, GLint maxFramebuffers, GLint *numFramebuffers);
GL_APICALL void GL_APIENTRY glExtGetTexLevelParameterivQCOM (GLuint texture, GLenum face, GLint level, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glExtTexObjectStateOverrideiQCOM (GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glExtGetTexSubImageQCOM (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLvoid *texels);
GL_APICALL void GL_APIENTRY glExtGetBufferPointervQCOM (GLenum target, GLvoid **params);
#endif
typedef void (GL_APIENTRYP PFNGLEXTGETTEXTURESQCOMPROC) (GLuint *textures, GLint maxTextures, GLint *numTextures);
typedef void (GL_APIENTRYP PFNGLEXTGETBUFFERSQCOMPROC) (GLuint *buffers, GLint maxBuffers, GLint *numBuffers);
typedef void (GL_APIENTRYP PFNGLEXTGETRENDERBUFFERSQCOMPROC) (GLuint *renderbuffers, GLint maxRenderbuffers, GLint *numRenderbuffers);
typedef void (GL_APIENTRYP PFNGLEXTGETFRAMEBUFFERSQCOMPROC) (GLuint *framebuffers, GLint maxFramebuffers, GLint *numFramebuffers);
typedef void (GL_APIENTRYP PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC) (GLuint texture, GLenum face, GLint level, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC) (GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLEXTGETTEXSUBIMAGEQCOMPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLvoid *texels);
typedef void (GL_APIENTRYP PFNGLEXTGETBUFFERPOINTERVQCOMPROC) (GLenum target, GLvoid **params);
#endif

/* GL_QCOM_extended_get2 */
#ifndef GL_QCOM_extended_get2
#define GL_QCOM_extended_get2 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glExtGetShadersQCOM (GLuint *shaders, GLint maxShaders, GLint *numShaders);
GL_APICALL void GL_APIENTRY glExtGetProgramsQCOM (GLuint *programs, GLint maxPrograms, GLint *numPrograms);
GL_APICALL GLboolean GL_APIENTRY glExtIsProgramBinaryQCOM (GLuint program);
GL_APICALL void GL_APIENTRY glExtGetProgramBinarySourceQCOM (GLuint program, GLenum shadertype, GLchar *source, GLint *length);
#endif
typedef void (GL_APIENTRYP PFNGLEXTGETSHADERSQCOMPROC) (GLuint *shaders, GLint maxShaders, GLint *numShaders);
typedef void (GL_APIENTRYP PFNGLEXTGETPROGRAMSQCOMPROC) (GLuint *programs, GLint maxPrograms, GLint *numPrograms);
typedef GLboolean (GL_APIENTRYP PFNGLEXTISPROGRAMBINARYQCOMPROC) (GLuint program);
typedef void (GL_APIENTRYP PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC) (GLuint program, GLenum shadertype, GLchar *source, GLint *length);
#endif

/* GL_QCOM_perfmon_global_mode */
#ifndef GL_QCOM_perfmon_global_mode
#define GL_QCOM_perfmon_global_mode 1
#endif

/* GL_QCOM_writeonly_rendering */
#ifndef GL_QCOM_writeonly_rendering
#define GL_QCOM_writeonly_rendering 1
#endif

/* GL_QCOM_tiled_rendering */
#ifndef GL_QCOM_tiled_rendering
#define GL_QCOM_tiled_rendering 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glStartTilingQCOM (GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);
GL_APICALL void GL_APIENTRY glEndTilingQCOM (GLbitfield preserveMask);
#endif
typedef void (GL_APIENTRYP PFNGLSTARTTILINGQCOMPROC) (GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);
typedef void (GL_APIENTRYP PFNGLENDTILINGQCOMPROC) (GLbitfield preserveMask);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __gl2ext_h_ */
