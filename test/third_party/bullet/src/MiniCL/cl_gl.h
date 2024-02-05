/**********************************************************************************
 * Copyright (c) 2008-2009 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 **********************************************************************************/

#ifndef __OPENCL_CL_GL_H
#define __OPENCL_CL_GL_H

#ifdef __APPLE__
#include <OpenCL/cl_platform.h>
#else
#include <MiniCL/cl_platform.h>
#endif	

#ifdef __cplusplus
extern "C" {
#endif

// NOTE:  Make sure that appropriate GL header file is included separately

typedef cl_uint     cl_gl_object_type;
typedef cl_uint     cl_gl_texture_info;
typedef cl_uint     cl_gl_platform_info;

// cl_gl_object_type
#define CL_GL_OBJECT_BUFFER             0x2000
#define CL_GL_OBJECT_TEXTURE2D          0x2001
#define CL_GL_OBJECT_TEXTURE3D          0x2002
#define CL_GL_OBJECT_RENDERBUFFER       0x2003

// cl_gl_texture_info
#define CL_GL_TEXTURE_TARGET            0x2004
#define CL_GL_MIPMAP_LEVEL              0x2005

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLBuffer(cl_context     /* context */,
                     cl_mem_flags   /* flags */,
                     GLuint         /* bufobj */,
                     int *          /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLTexture2D(cl_context      /* context */,
                        cl_mem_flags    /* flags */,
                        GLenum          /* target */,
                        GLint           /* miplevel */,
                        GLuint          /* texture */,
                        cl_int *        /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLTexture3D(cl_context      /* context */,
                        cl_mem_flags    /* flags */,
                        GLenum          /* target */,
                        GLint           /* miplevel */,
                        GLuint          /* texture */,
                        cl_int *        /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromGLRenderbuffer(cl_context   /* context */,
                           cl_mem_flags /* flags */,
                           GLuint       /* renderbuffer */,
                           cl_int *     /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clGetGLObjectInfo(cl_mem                /* memobj */,
                  cl_gl_object_type *   /* gl_object_type */,
                  GLuint *              /* gl_object_name */) CL_API_SUFFIX__VERSION_1_0;
                  
extern CL_API_ENTRY cl_int CL_API_CALL
clGetGLTextureInfo(cl_mem               /* memobj */,
                   cl_gl_texture_info   /* param_name */,
                   size_t               /* param_value_size */,
                   void *               /* param_value */,
                   size_t *             /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAcquireGLObjects(cl_command_queue      /* command_queue */,
                          cl_uint               /* num_objects */,
                          const cl_mem *        /* mem_objects */,
                          cl_uint               /* num_events_in_wait_list */,
                          const cl_event *      /* event_wait_list */,
                          cl_event *            /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReleaseGLObjects(cl_command_queue      /* command_queue */,
                          cl_uint               /* num_objects */,
                          const cl_mem *        /* mem_objects */,
                          cl_uint               /* num_events_in_wait_list */,
                          const cl_event *      /* event_wait_list */,
                          cl_event *            /* event */) CL_API_SUFFIX__VERSION_1_0;

#ifdef __cplusplus
}
#endif

#endif  // __OPENCL_CL_GL_H
