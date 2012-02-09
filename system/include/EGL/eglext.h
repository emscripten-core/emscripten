#ifndef __eglext_h_
#define __eglext_h_

#ifdef __cplusplus
extern "C" {
#endif

/*
** Copyright (c) 2007-2010 The Khronos Group Inc.
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

#include <EGL/eglplatform.h>

/*************************************************************/

/* Header file version number */
/* Current version at http://www.khronos.org/registry/egl/ */
/* $Revision: 12124 $ on $Date: 2010-07-27 20:12:35 -0700 (Tue, 27 Jul 2010) $ */
#define EGL_EGLEXT_VERSION 7

#ifndef EGL_KHR_config_attribs
#define EGL_KHR_config_attribs 1
#define EGL_CONFORMANT_KHR			0x3042	/* EGLConfig attribute */
#define EGL_VG_COLORSPACE_LINEAR_BIT_KHR	0x0020	/* EGL_SURFACE_TYPE bitfield */
#define EGL_VG_ALPHA_FORMAT_PRE_BIT_KHR		0x0040	/* EGL_SURFACE_TYPE bitfield */
#endif

#ifndef EGL_KHR_lock_surface
#define EGL_KHR_lock_surface 1
#define EGL_READ_SURFACE_BIT_KHR		0x0001	/* EGL_LOCK_USAGE_HINT_KHR bitfield */
#define EGL_WRITE_SURFACE_BIT_KHR		0x0002	/* EGL_LOCK_USAGE_HINT_KHR bitfield */
#define EGL_LOCK_SURFACE_BIT_KHR		0x0080	/* EGL_SURFACE_TYPE bitfield */
#define EGL_OPTIMAL_FORMAT_BIT_KHR		0x0100	/* EGL_SURFACE_TYPE bitfield */
#define EGL_MATCH_FORMAT_KHR			0x3043	/* EGLConfig attribute */
#define EGL_FORMAT_RGB_565_EXACT_KHR		0x30C0	/* EGL_MATCH_FORMAT_KHR value */
#define EGL_FORMAT_RGB_565_KHR			0x30C1	/* EGL_MATCH_FORMAT_KHR value */
#define EGL_FORMAT_RGBA_8888_EXACT_KHR		0x30C2	/* EGL_MATCH_FORMAT_KHR value */
#define EGL_FORMAT_RGBA_8888_KHR		0x30C3	/* EGL_MATCH_FORMAT_KHR value */
#define EGL_MAP_PRESERVE_PIXELS_KHR		0x30C4	/* eglLockSurfaceKHR attribute */
#define EGL_LOCK_USAGE_HINT_KHR			0x30C5	/* eglLockSurfaceKHR attribute */
#define EGL_BITMAP_POINTER_KHR			0x30C6	/* eglQuerySurface attribute */
#define EGL_BITMAP_PITCH_KHR			0x30C7	/* eglQuerySurface attribute */
#define EGL_BITMAP_ORIGIN_KHR			0x30C8	/* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_RED_OFFSET_KHR		0x30C9	/* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR	0x30CA	/* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR	0x30CB	/* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR	0x30CC	/* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR	0x30CD	/* eglQuerySurface attribute */
#define EGL_LOWER_LEFT_KHR			0x30CE	/* EGL_BITMAP_ORIGIN_KHR value */
#define EGL_UPPER_LEFT_KHR			0x30CF	/* EGL_BITMAP_ORIGIN_KHR value */
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglLockSurfaceKHR (EGLDisplay display, EGLSurface surface, const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglUnlockSurfaceKHR (EGLDisplay display, EGLSurface surface);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLLOCKSURFACEKHRPROC) (EGLDisplay display, EGLSurface surface, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLUNLOCKSURFACEKHRPROC) (EGLDisplay display, EGLSurface surface);
#endif

#ifndef EGL_KHR_image
#define EGL_KHR_image 1
#define EGL_NATIVE_PIXMAP_KHR			0x30B0	/* eglCreateImageKHR target */
typedef void *EGLImageKHR;
#define EGL_NO_IMAGE_KHR			((EGLImageKHR)0)
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLImageKHR EGLAPIENTRY eglCreateImageKHR (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglDestroyImageKHR (EGLDisplay dpy, EGLImageKHR image);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC) (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC) (EGLDisplay dpy, EGLImageKHR image);
#endif

#ifndef EGL_KHR_vg_parent_image
#define EGL_KHR_vg_parent_image 1
#define EGL_VG_PARENT_IMAGE_KHR			0x30BA	/* eglCreateImageKHR target */
#endif

#ifndef EGL_KHR_gl_texture_2D_image
#define EGL_KHR_gl_texture_2D_image 1
#define EGL_GL_TEXTURE_2D_KHR			0x30B1	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_LEVEL_KHR		0x30BC	/* eglCreateImageKHR attribute */
#endif

#ifndef EGL_KHR_gl_texture_cubemap_image
#define EGL_KHR_gl_texture_cubemap_image 1
#define EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR	0x30B3	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR	0x30B4	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR	0x30B5	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR	0x30B6	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR	0x30B7	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR	0x30B8	/* eglCreateImageKHR target */
#endif

#ifndef EGL_KHR_gl_texture_3D_image
#define EGL_KHR_gl_texture_3D_image 1
#define EGL_GL_TEXTURE_3D_KHR			0x30B2	/* eglCreateImageKHR target */
#define EGL_GL_TEXTURE_ZOFFSET_KHR		0x30BD	/* eglCreateImageKHR attribute */
#endif

#ifndef EGL_KHR_gl_renderbuffer_image
#define EGL_KHR_gl_renderbuffer_image 1
#define EGL_GL_RENDERBUFFER_KHR			0x30B9	/* eglCreateImageKHR target */
#endif

#ifndef EGL_MESA_drm_image
#define EGL_MESA_drm_image 1
#define EGL_DRM_BUFFER_FORMAT_MESA		0x31D0	/* eglCreateImageKHR attribute */
#define EGL_DRM_BUFFER_USE_MESA			0x31D1

/* EGL_DRM_BUFFER_FORMAT_MESA tokens */
#define EGL_DRM_BUFFER_FORMAT_ARGB32_MESA	0x31D2

/* EGL_DRM_BUFFER_USE_MESA bits */
#define EGL_DRM_BUFFER_USE_SCANOUT_MESA		0x0001
#define EGL_DRM_BUFFER_USE_SHARE_MESA		0x0002
#define EGL_DRM_BUFFER_USE_CURSOR_MESA		0x0004

#define EGL_DRM_BUFFER_MESA			0x31D3  /* eglCreateImageKHR target */
#define EGL_DRM_BUFFER_STRIDE_MESA		0x31D4	/* eglCreateImageKHR attribute */

#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLImageKHR EGLAPIENTRY eglCreateDRMImageMESA(EGLDisplay dpy, const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglExportDRMImageMESA(EGLDisplay dpy, EGLImageKHR image, EGLint *name, EGLint *handle, EGLint *stride);
#endif
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEDRMIMAGEMESA) (EGLDisplay dpy, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLEXPORTDRMIMAGEMESA) (EGLDisplay dpy, EGLImageKHR image, EGLint *name, EGLint *handle, EGLint *stride);
#endif

#ifndef EGL_WL_bind_wayland_display
#define EGL_WL_bind_wayland_display 1

#define EGL_WAYLAND_BUFFER_WL			0x31D5 /* eglCreateImageKHR target */
struct wl_display;
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglBindWaylandDisplayWL(EGLDisplay dpy, struct wl_display *display);
EGLAPI EGLBoolean EGLAPIENTRY eglUnbindWaylandDisplayWL(EGLDisplay dpy, struct wl_display *display);
#endif
typedef EGLBoolean (EGLAPIENTRYP PFNEGLBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLUNBINDWAYLANDDISPLAYWL) (EGLDisplay dpy, struct wl_display *display);
#endif

#if KHRONOS_SUPPORT_INT64   /* EGLTimeKHR requires 64-bit uint support */
#ifndef EGL_KHR_reusable_sync
#define EGL_KHR_reusable_sync 1

typedef void* EGLSyncKHR;
typedef khronos_utime_nanoseconds_t EGLTimeKHR;

#define EGL_SYNC_STATUS_KHR			0x30F1
#define EGL_SIGNALED_KHR			0x30F2
#define EGL_UNSIGNALED_KHR			0x30F3
#define EGL_TIMEOUT_EXPIRED_KHR			0x30F5
#define EGL_CONDITION_SATISFIED_KHR		0x30F6
#define EGL_SYNC_TYPE_KHR			0x30F7
#define EGL_SYNC_REUSABLE_KHR			0x30FA
#define EGL_SYNC_FLUSH_COMMANDS_BIT_KHR		0x0001	/* eglClientWaitSyncKHR <flags> bitfield */
#define EGL_FOREVER_KHR				0xFFFFFFFFFFFFFFFFull
#define EGL_NO_SYNC_KHR				((EGLSyncKHR)0)
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLSyncKHR EGLAPIENTRY eglCreateSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR sync);
EGLAPI EGLint EGLAPIENTRY eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
EGLAPI EGLBoolean EGLAPIENTRY eglSignalSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode);
EGLAPI EGLBoolean EGLAPIENTRY eglGetSyncAttribKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLSyncKHR (EGLAPIENTRYP PFNEGLCREATESYNCKHRPROC) (EGLDisplay dpy, EGLenum type, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYSYNCKHRPROC) (EGLDisplay dpy, EGLSyncKHR sync);
typedef EGLint (EGLAPIENTRYP PFNEGLCLIENTWAITSYNCKHRPROC) (EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSIGNALSYNCKHRPROC) (EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSYNCATTRIBKHRPROC) (EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value);
#endif
#endif

/* EGL_MESA_screen extension  >>> PRELIMINARY <<< */
#ifndef EGL_MESA_screen_surface
#define EGL_MESA_screen_surface 1

#define EGL_BAD_SCREEN_MESA                    0x4000
#define EGL_BAD_MODE_MESA                      0x4001
#define EGL_SCREEN_COUNT_MESA                  0x4002
#define EGL_SCREEN_POSITION_MESA               0x4003
#define EGL_SCREEN_POSITION_GRANULARITY_MESA   0x4004
#define EGL_MODE_ID_MESA                       0x4005
#define EGL_REFRESH_RATE_MESA                  0x4006
#define EGL_OPTIMAL_MESA                       0x4007
#define EGL_INTERLACED_MESA                    0x4008
#define EGL_SCREEN_BIT_MESA                    0x08

typedef khronos_uint32_t EGLScreenMESA;
typedef khronos_uint32_t EGLModeMESA;

#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglChooseModeMESA(EGLDisplay dpy, EGLScreenMESA screen, const EGLint *attrib_list, EGLModeMESA *modes, EGLint modes_size, EGLint *num_modes);
EGLAPI EGLBoolean EGLAPIENTRY eglGetModesMESA(EGLDisplay dpy, EGLScreenMESA screen, EGLModeMESA *modes, EGLint modes_size, EGLint *num_modes);
EGLAPI EGLBoolean EGLAPIENTRY eglGetModeAttribMESA(EGLDisplay dpy, EGLModeMESA mode, EGLint attribute, EGLint *value);
EGLAPI EGLBoolean EGLAPIENTRY eglGetScreensMESA(EGLDisplay dpy, EGLScreenMESA *screens, EGLint max_screens, EGLint *num_screens);
EGLAPI EGLSurface EGLAPIENTRY eglCreateScreenSurfaceMESA(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglShowScreenSurfaceMESA(EGLDisplay dpy, EGLint screen, EGLSurface surface, EGLModeMESA mode);
EGLAPI EGLBoolean EGLAPIENTRY eglScreenPositionMESA(EGLDisplay dpy, EGLScreenMESA screen, EGLint x, EGLint y);
EGLAPI EGLBoolean EGLAPIENTRY eglQueryScreenMESA(EGLDisplay dpy, EGLScreenMESA screen, EGLint attribute, EGLint *value);
EGLAPI EGLBoolean EGLAPIENTRY eglQueryScreenSurfaceMESA(EGLDisplay dpy, EGLScreenMESA screen, EGLSurface *surface);
EGLAPI EGLBoolean EGLAPIENTRY eglQueryScreenModeMESA(EGLDisplay dpy, EGLScreenMESA screen, EGLModeMESA *mode);
EGLAPI const char * EGLAPIENTRY eglQueryModeStringMESA(EGLDisplay dpy, EGLModeMESA mode);
#endif /* EGL_EGLEXT_PROTOTYPES */

typedef EGLBoolean (EGLAPIENTRYP PFNEGLCHOOSEMODEMESA) (EGLDisplay dpy, EGLScreenMESA screen, const EGLint *attrib_list, EGLModeMESA *modes, EGLint modes_size, EGLint *num_modes);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETMODESMESA) (EGLDisplay dpy, EGLScreenMESA screen, EGLModeMESA *modes, EGLint modes_size, EGLint *num_modes);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGetModeATTRIBMESA) (EGLDisplay dpy, EGLModeMESA mode, EGLint attribute, EGLint *value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSCRREENSMESA) (EGLDisplay dpy, EGLScreenMESA *screens, EGLint max_screens, EGLint *num_screens);
typedef EGLSurface (EGLAPIENTRYP PFNEGLCREATESCREENSURFACEMESA) (EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSHOWSCREENSURFACEMESA) (EGLDisplay dpy, EGLint screen, EGLSurface surface, EGLModeMESA mode);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSCREENPOSIITONMESA) (EGLDisplay dpy, EGLScreenMESA screen, EGLint x, EGLint y);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYSCREENMESA) (EGLDisplay dpy, EGLScreenMESA screen, EGLint attribute, EGLint *value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYSCREENSURFACEMESA) (EGLDisplay dpy, EGLScreenMESA screen, EGLSurface *surface);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYSCREENMODEMESA) (EGLDisplay dpy, EGLScreenMESA screen, EGLModeMESA *mode);
typedef const char * (EGLAPIENTRYP PFNEGLQUERYMODESTRINGMESA) (EGLDisplay dpy, EGLModeMESA mode);

#endif /* EGL_MESA_screen_surface */


#ifndef EGL_MESA_copy_context
#define EGL_MESA_copy_context 1

#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglCopyContextMESA(EGLDisplay dpy, EGLContext source, EGLContext dest, EGLint mask);
#endif /* EGL_EGLEXT_PROTOTYPES */

typedef EGLBoolean (EGLAPIENTRYP PFNEGLCOPYCONTEXTMESA) (EGLDisplay dpy, EGLContext source, EGLContext dest, EGLint mask);

#endif /* EGL_MESA_copy_context */

#ifndef EGL_MESA_drm_display
#define EGL_MESA_drm_display 1

#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLDisplay EGLAPIENTRY eglGetDRMDisplayMESA(int fd);
#endif /* EGL_EGLEXT_PROTOTYPES */

typedef EGLDisplay (EGLAPIENTRYP PFNEGLGETDRMDISPLAYMESA) (int fd);

#endif /* EGL_MESA_drm_display */

#ifndef EGL_KHR_image_base
#define EGL_KHR_image_base 1
/* Most interfaces defined by EGL_KHR_image_pixmap above */
#define EGL_IMAGE_PRESERVED_KHR			0x30D2	/* eglCreateImageKHR attribute */
#endif

#ifndef EGL_KHR_image_pixmap
#define EGL_KHR_image_pixmap 1
/* Interfaces defined by EGL_KHR_image above */
#endif

#ifndef EGL_IMG_context_priority
#define EGL_IMG_context_priority 1
#define EGL_CONTEXT_PRIORITY_LEVEL_IMG		0x3100
#define EGL_CONTEXT_PRIORITY_HIGH_IMG		0x3101
#define EGL_CONTEXT_PRIORITY_MEDIUM_IMG		0x3102
#define EGL_CONTEXT_PRIORITY_LOW_IMG		0x3103
#endif

#ifndef EGL_KHR_lock_surface2
#define EGL_KHR_lock_surface2 1
#define EGL_BITMAP_PIXEL_SIZE_KHR		0x3110
#endif

#ifndef EGL_NV_coverage_sample
#define EGL_NV_coverage_sample 1
#define EGL_COVERAGE_BUFFERS_NV 0x30E0
#define EGL_COVERAGE_SAMPLES_NV 0x30E1
#endif

#ifndef EGL_NV_depth_nonlinear
#define EGL_NV_depth_nonlinear 1
#define EGL_DEPTH_ENCODING_NV 0x30E2
#define EGL_DEPTH_ENCODING_NONE_NV 0
#define EGL_DEPTH_ENCODING_NONLINEAR_NV 0x30E3
#endif

#if KHRONOS_SUPPORT_INT64   /* EGLTimeNV requires 64-bit uint support */
#ifndef EGL_NV_sync
#define EGL_NV_sync 1
#define EGL_SYNC_PRIOR_COMMANDS_COMPLETE_NV	0x30E6
#define EGL_SYNC_STATUS_NV			0x30E7
#define EGL_SIGNALED_NV				0x30E8
#define EGL_UNSIGNALED_NV			0x30E9
#define EGL_SYNC_FLUSH_COMMANDS_BIT_NV		0x0001
#define EGL_FOREVER_NV				0xFFFFFFFFFFFFFFFFull
#define EGL_ALREADY_SIGNALED_NV			0x30EA
#define EGL_TIMEOUT_EXPIRED_NV			0x30EB
#define EGL_CONDITION_SATISFIED_NV		0x30EC
#define EGL_SYNC_TYPE_NV			0x30ED
#define EGL_SYNC_CONDITION_NV			0x30EE
#define EGL_SYNC_FENCE_NV			0x30EF
#define EGL_NO_SYNC_NV				((EGLSyncNV)0)
typedef void* EGLSyncNV;
typedef khronos_utime_nanoseconds_t EGLTimeNV;
#ifdef EGL_EGLEXT_PROTOTYPES
EGLSyncNV eglCreateFenceSyncNV (EGLDisplay dpy, EGLenum condition, const EGLint *attrib_list);
EGLBoolean eglDestroySyncNV (EGLSyncNV sync);
EGLBoolean eglFenceNV (EGLSyncNV sync);
EGLint eglClientWaitSyncNV (EGLSyncNV sync, EGLint flags, EGLTimeNV timeout);
EGLBoolean eglSignalSyncNV (EGLSyncNV sync, EGLenum mode);
EGLBoolean eglGetSyncAttribNV (EGLSyncNV sync, EGLint attribute, EGLint *value);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLSyncNV (EGLAPIENTRYP PFNEGLCREATEFENCESYNCNVPROC) (EGLDisplay dpy, EGLenum condition, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYSYNCNVPROC) (EGLSyncNV sync);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLFENCENVPROC) (EGLSyncNV sync);
typedef EGLint (EGLAPIENTRYP PFNEGLCLIENTWAITSYNCNVPROC) (EGLSyncNV sync, EGLint flags, EGLTimeNV timeout);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSIGNALSYNCNVPROC) (EGLSyncNV sync, EGLenum mode);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSYNCATTRIBNVPROC) (EGLSyncNV sync, EGLint attribute, EGLint *value);
#endif
#endif

#if KHRONOS_SUPPORT_INT64   /* Dependent on EGL_KHR_reusable_sync which requires 64-bit uint support */
#ifndef EGL_KHR_fence_sync
#define EGL_KHR_fence_sync 1
/* Reuses most tokens and entry points from EGL_KHR_reusable_sync */
#define EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR	0x30F0
#define EGL_SYNC_CONDITION_KHR			0x30F8
#define EGL_SYNC_FENCE_KHR			0x30F9
#endif
#endif

#ifndef EGL_HI_clientpixmap
#define EGL_HI_clientpixmap 1

/* Surface Attribute */
#define EGL_CLIENT_PIXMAP_POINTER_HI		0x8F74
/*
 * Structure representing a client pixmap
 * (pixmap's data is in client-space memory).
 */
struct EGLClientPixmapHI
{
	void*		pData;
	EGLint		iWidth;
	EGLint		iHeight;
	EGLint		iStride;
};

#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLSurface EGLAPIENTRY eglCreatePixmapSurfaceHI(EGLDisplay dpy, EGLConfig config, struct EGLClientPixmapHI* pixmap);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLSurface (EGLAPIENTRYP PFNEGLCREATEPIXMAPSURFACEHIPROC) (EGLDisplay dpy, EGLConfig config, struct EGLClientPixmapHI* pixmap);
#endif	/* EGL_HI_clientpixmap */

#ifndef EGL_HI_colorformats
#define EGL_HI_colorformats 1
/* Config Attribute */
#define EGL_COLOR_FORMAT_HI			0x8F70
/* Color Formats */
#define EGL_COLOR_RGB_HI			0x8F71
#define EGL_COLOR_RGBA_HI			0x8F72
#define EGL_COLOR_ARGB_HI			0x8F73
#endif /* EGL_HI_colorformats */

#ifndef EGL_NOK_swap_region
#define EGL_NOK_swap_region 1

#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffersRegionNOK(EGLDisplay dpy, EGLSurface surface, EGLint numRects, const EGLint* rects);
#endif

typedef EGLBoolean (EGLAPIENTRYP PFNEGLSWAPBUFFERSREGIONNOK) (EGLDisplay dpy, EGLSurface surface, EGLint numRects, const EGLint* rects);
#endif


#ifndef EGL_NOK_texture_from_pixmap
#define EGL_NOK_texture_from_pixmap 1

#define EGL_Y_INVERTED_NOK			0x307F
#endif /* EGL_NOK_texture_from_pixmap */


#ifdef __cplusplus
}
#endif

#endif
