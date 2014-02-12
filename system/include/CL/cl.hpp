/*******************************************************************************
 * Copyright (c) 2008-2010 The Khronos Group Inc.
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
 ******************************************************************************/

/*! \file
 *
 *   \brief C++ bindings for OpenCL 1.0 (rev 48) and OpenCL 1.1 (rev 33)    
 *   \author Benedict R. Gaster and Laurent Morichetti
 *   
 *   Additions and fixes from Brian Cole, March 3rd 2010.
 *   
 *   \version 1.1
 *   \date June 2010
 *
 *   Optional extension support
 *
 *         cl
 *         cl_ext_device_fission
 *				#define USE_CL_DEVICE_FISSION
 */

/*! \mainpage
 * \section intro Introduction
 * For many large applications C++ is the language of choice and so it seems
 * reasonable to define C++ bindings for OpenCL.
 *
 *
 * The interface is contained with a single C++ header file \em cl.hpp and all
 * definitions are contained within the namespace \em cl. There is no additional
 * requirement to include \em cl.h and to use either the C++ or original C
 * bindings it is enough to simply include \em cl.hpp.
 *
 * The bindings themselves are lightweight and correspond closely to the
 * underlying C API. Using the C++ bindings introduces no additional execution
 * overhead.
 *
 * For detail documentation on the bindings see:
 *
 * The OpenCL C++ Wrapper API 1.1 (revision 04)
 *  http://www.khronos.org/registry/cl/specs/opencl-cplusplus-1.1.pdf
 *
 * \section example Example
 *
 * The following example shows a general use case for the C++
 * bindings, including support for the optional exception feature and
 * also the supplied vector and string classes, see following sections for
 * decriptions of these features.
 *
 * \code
 * #define __CL_ENABLE_EXCEPTIONS
 * 
 * #if defined(__APPLE__) || defined(__MACOSX)
 * #include <OpenCL/cl.hpp>
 * #else
 * #include <CL/cl.hpp>
 * #endif
 * #include <cstdio>
 * #include <cstdlib>
 * #include <iostream>
 * 
 *  const char * helloStr  = "__kernel void "
 *                           "hello(void) "
 *                           "{ "
 *                           "  "
 *                           "} ";
 * 
 *  int
 *  main(void)
 *  {
 *     cl_int err = CL_SUCCESS;
 *     try {
 *
 *       std::vector<cl::Platform> platforms;
 *       cl::Platform::get(&platforms);
 *       if (platforms.size() == 0) {
 *           std::cout << "Platform size 0\n";
 *           return -1;
 *       }
 *
 *       cl_context_properties properties[] = 
 *          { CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};
 *       cl::Context context(CL_DEVICE_TYPE_CPU, properties); 
 * 
 *       std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
 * 
 *       cl::Program::Sources source(1,
 *           std::make_pair(helloStr,strlen(helloStr)));
 *       cl::Program program_ = cl::Program(context, source);
 *       program_.build(devices);
 * 
 *       cl::Kernel kernel(program_, "hello", &err);
 * 
 *       cl::Event event;
 *       cl::CommandQueue queue(context, devices[0], 0, &err);
 *       queue.enqueueNDRangeKernel(
 *           kernel, 
 *           cl::NullRange, 
 *           cl::NDRange(4,4),
 *           cl::NullRange,
 *           NULL,
 *           &event); 
 * 
 *       event.wait();
 *     }
 *     catch (cl::Error err) {
 *        std::cerr 
 *           << "ERROR: "
 *           << err.what()
 *           << "("
 *           << err.err()
 *           << ")"
 *           << std::endl;
 *     }
 * 
 *    return EXIT_SUCCESS;
 *  }
 * 
 * \endcode
 *
 */
#ifndef CL_HPP_
#define CL_HPP_

#ifdef _WIN32
#include <windows.h>
#include <malloc.h>
#if defined(USE_DX_INTEROP)
#include <CL/cl_d3d10.h>
#endif
#endif // _WIN32

// 
#if defined(USE_CL_DEVICE_FISSION)
#include <CL/cl_ext.h>
#endif

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#include <OpenCL/opencl.h>
#else
#include <GL/gl.h>
#include <CL/opencl.h>
#endif // !__APPLE__

#if !defined(CL_CALLBACK)
#define CL_CALLBACK
#endif //CL_CALLBACK

#include <utility>

#if !defined(__NO_STD_VECTOR)
#include <vector>
#endif

#if !defined(__NO_STD_STRING)
#include <string>
#endif 

#if defined(linux) || defined(__APPLE__) || defined(__MACOSX) || defined(__EMSCRIPTEN__)
# include <alloca.h>
#endif // linux

#include <cstring>

/*! \namespace cl
 *
 * \brief The OpenCL C++ bindings are defined within this namespace.
 *
 */
namespace cl {

#define __INIT_CL_EXT_FCN_PTR(name) \
    if(!pfn_##name) { \
        pfn_##name = (PFN_##name) \
            clGetExtensionFunctionAddress(#name); \
        if(!pfn_##name) { \
        } \
    }

class Program;
class Device;
class Context;
class CommandQueue;
class Memory;

#if defined(__CL_ENABLE_EXCEPTIONS)
#include <exception>
/*! \class Error
 * \brief Exception class
 */
class Error : public std::exception
{
private:
    cl_int err_;
    const char * errStr_;
public:
    /*! Create a new CL error exception for a given error code
     *  and corresponding message.
     */
    Error(cl_int err, const char * errStr = NULL) : err_(err), errStr_(errStr)
    {}

    ~Error() throw() {}

    /*! \brief Get error string associated with exception
     *
     * \return A memory pointer to the error message string.
     */
    virtual const char * what() const throw ()
    {
        if (errStr_ == NULL) {
            return "empty";
        }
        else {
            return errStr_;
        }
    }

    /*! \brief Get error code associated with exception
     *
     *  \return The error code.
     */
    const cl_int err(void) const { return err_; }
};

#define __ERR_STR(x) #x
#else
#define __ERR_STR(x) NULL
#endif // __CL_ENABLE_EXCEPTIONS

//! \cond DOXYGEN_DETAIL
#if !defined(__CL_USER_OVERRIDE_ERROR_STRINGS)
#define __GET_DEVICE_INFO_ERR               __ERR_STR(clgetDeviceInfo)
#define __GET_PLATFORM_INFO_ERR             __ERR_STR(clGetPlatformInfo)
#define __GET_DEVICE_IDS_ERR                __ERR_STR(clGetDeviceIDs)
#define __GET_PLATFORM_IDS_ERR              __ERR_STR(clGetPlatformIDs)
#define __GET_CONTEXT_INFO_ERR              __ERR_STR(clGetContextInfo)
#define __GET_EVENT_INFO_ERR                __ERR_STR(clGetEventInfo)
#define __GET_EVENT_PROFILE_INFO_ERR        __ERR_STR(clGetEventProfileInfo)
#define __GET_MEM_OBJECT_INFO_ERR           __ERR_STR(clGetMemObjectInfo)
#define __GET_IMAGE_INFO_ERR                __ERR_STR(clGetImageInfo)
#define __GET_SAMPLER_INFO_ERR              __ERR_STR(clGetSamplerInfo)
#define __GET_KERNEL_INFO_ERR               __ERR_STR(clGetKernelInfo)
#define __GET_KERNEL_WORK_GROUP_INFO_ERR    __ERR_STR(clGetKernelWorkGroupInfo)
#define __GET_PROGRAM_INFO_ERR              __ERR_STR(clGetProgramInfo)
#define __GET_PROGRAM_BUILD_INFO_ERR        __ERR_STR(clGetProgramBuildInfo)
#define __GET_COMMAND_QUEUE_INFO_ERR        __ERR_STR(clGetCommandQueueInfo)

#define __CREATE_CONTEXT_FROM_TYPE_ERR      __ERR_STR(clCreateContextFromType)
#define __GET_SUPPORTED_IMAGE_FORMATS_ERR   __ERR_STR(clGetSupportedImageFormats)

#define __CREATE_BUFFER_ERR                 __ERR_STR(clCreateBuffer)
#define __CREATE_SUBBUFFER_ERR              __ERR_STR(clCreateSubBuffer)
#define __CREATE_GL_BUFFER_ERR              __ERR_STR(clCreateFromGLBuffer)
#define __GET_GL_OBJECT_INFO_ERR            __ERR_STR(clGetGLObjectInfo)
#define __CREATE_IMAGE2D_ERR                __ERR_STR(clCreateImage2D)
#define __CREATE_IMAGE3D_ERR                __ERR_STR(clCreateImage3D)
#define __CREATE_SAMPLER_ERR                __ERR_STR(clCreateSampler)
#define __SET_MEM_OBJECT_DESTRUCTOR_CALLBACK_ERR __ERR_STR(clSetMemObjectDestructorCallback)

#define __CREATE_USER_EVENT_ERR             __ERR_STR(clCreateUserEvent)
#define __SET_USER_EVENT_STATUS_ERR         __ERR_STR(clSetUserEventStatus)
#define __SET_EVENT_CALLBACK_ERR            __ERR_STR(clSetEventCallback)
#define __WAIT_FOR_EVENTS_ERR               __ERR_STR(clWaitForEvents)

#define __CREATE_KERNEL_ERR                 __ERR_STR(clCreateKernel)
#define __SET_KERNEL_ARGS_ERR               __ERR_STR(clSetKernelArg)
#define __CREATE_PROGRAM_WITH_SOURCE_ERR    __ERR_STR(clCreateProgramWithSource)
#define __CREATE_PROGRAM_WITH_BINARY_ERR    __ERR_STR(clCreateProgramWithBinary)
#define __BUILD_PROGRAM_ERR                 __ERR_STR(clBuildProgram)
#define __CREATE_KERNELS_IN_PROGRAM_ERR     __ERR_STR(clCreateKernelsInProgram)

#define __CREATE_COMMAND_QUEUE_ERR          __ERR_STR(clCreateCommandQueue)
#define __SET_COMMAND_QUEUE_PROPERTY_ERR    __ERR_STR(clSetCommandQueueProperty)
#define __ENQUEUE_READ_BUFFER_ERR           __ERR_STR(clEnqueueReadBuffer)
#define __ENQUEUE_READ_BUFFER_RECT_ERR      __ERR_STR(clEnqueueReadBufferRect)
#define __ENQUEUE_WRITE_BUFFER_ERR          __ERR_STR(clEnqueueWriteBuffer)
#define __ENQUEUE_WRITE_BUFFER_RECT_ERR     __ERR_STR(clEnqueueWriteBufferRect)
#define __ENQEUE_COPY_BUFFER_ERR            __ERR_STR(clEnqueueCopyBuffer)
#define __ENQEUE_COPY_BUFFER_RECT_ERR       __ERR_STR(clEnqueueCopyBufferRect)
#define __ENQUEUE_READ_IMAGE_ERR            __ERR_STR(clEnqueueReadImage)
#define __ENQUEUE_WRITE_IMAGE_ERR           __ERR_STR(clEnqueueWriteImage)
#define __ENQUEUE_COPY_IMAGE_ERR            __ERR_STR(clEnqueueCopyImage)
#define __ENQUEUE_COPY_IMAGE_TO_BUFFER_ERR  __ERR_STR(clEnqueueCopyImageToBuffer)
#define __ENQUEUE_COPY_BUFFER_TO_IMAGE_ERR  __ERR_STR(clEnqueueCopyBufferToImage)
#define __ENQUEUE_MAP_BUFFER_ERR            __ERR_STR(clEnqueueMapBuffer)
#define __ENQUEUE_MAP_IMAGE_ERR             __ERR_STR(clEnqueueMapImage)
#define __ENQUEUE_UNMAP_MEM_OBJECT_ERR      __ERR_STR(clEnqueueUnMapMemObject)
#define __ENQUEUE_NDRANGE_KERNEL_ERR        __ERR_STR(clEnqueueNDRangeKernel)
#define __ENQUEUE_TASK_ERR                  __ERR_STR(clEnqueueTask)
#define __ENQUEUE_NATIVE_KERNEL             __ERR_STR(clEnqueueNativeKernel)
#define __ENQUEUE_MARKER_ERR                __ERR_STR(clEnqueueMarker)
#define __ENQUEUE_WAIT_FOR_EVENTS_ERR       __ERR_STR(clEnqueueWaitForEvents)
#define __ENQUEUE_BARRIER_ERR               __ERR_STR(clEnqueueBarrier)

#define __ENQUEUE_ACQUIRE_GL_ERR            __ERR_STR(clEnqueueAcquireGLObjects)
#define __ENQUEUE_RELEASE_GL_ERR            __ERR_STR(clEnqueueReleaseGLObjects)

#define __UNLOAD_COMPILER_ERR               __ERR_STR(clUnloadCompiler)

#define __FLUSH_ERR                         __ERR_STR(clFlush)
#define __FINISH_ERR                        __ERR_STR(clFinish)

#define __CREATE_SUB_DEVICES                __ERR_STR(clCreateSubDevicesEXT)
#endif // __CL_USER_OVERRIDE_ERROR_STRINGS
//! \endcond

/*! \class string
 * \brief Simple string class, that provides a limited subset of std::string
 * functionality but avoids many of the issues that come with that class.
 */
class string
{
private:
    ::size_t size_;
    char * str_;
public:
    string(void) : size_(0), str_(NULL)
    {
    }

    string(char * str, ::size_t size) :
        size_(size),
        str_(NULL)
    {
        str_ = new char[size_+1];
        if (str_ != NULL) {
            memcpy(str_, str, size_  * sizeof(char));
            str_[size_] = '\0';
        }
        else {
            size_ = 0;
        }
    }

    string(char * str) :
        str_(NULL)
    {
        size_= ::strlen(str);
        str_ = new char[size_ + 1];
        if (str_ != NULL) {
            memcpy(str_, str, (size_ + 1) * sizeof(char));
        }
        else {
            size_ = 0;
        }
    }

    string& operator=(const string& rhs)
    {
        if (this == &rhs) {
            return *this;
        }

        if (rhs.size_ == 0 || rhs.str_ == NULL) {
            size_ = 0;
            str_  = NULL;
        } 
        else {
            size_ = rhs.size_;
            str_ = new char[size_ + 1];
            if (str_ != NULL) {
                memcpy(str_, rhs.str_, (size_ + 1) * sizeof(char));
            }
            else {
                size_ = 0;
            }
        }

        return *this;
    }

    string(const string& rhs)
    {
        *this = rhs;
    }

    ~string()
    {
        if (str_ != NULL) {
            delete[] str_;
        }
    }

    ::size_t size(void) const   { return size_; }
    ::size_t length(void) const { return size(); }

    const char * c_str(void) const { return (str_) ? str_ : "";}
};

#if !defined(__USE_DEV_STRING) && !defined(__NO_STD_STRING)
#include <string>
typedef std::string STRING_CLASS;
#elif !defined(__USE_DEV_STRING) 
typedef cl::string STRING_CLASS;
#endif

#if !defined(__USE_DEV_VECTOR) && !defined(__NO_STD_VECTOR)
#include <vector>
#define VECTOR_CLASS std::vector
#elif !defined(__USE_DEV_VECTOR) 
#define VECTOR_CLASS cl::vector 
#endif

#if !defined(__MAX_DEFAULT_VECTOR_SIZE)
#define __MAX_DEFAULT_VECTOR_SIZE 10
#endif

/*! \class vector
 * \brief Fixed sized vector implementation that mirroring 
 * std::vector functionality.
 */
template <typename T, unsigned int N = __MAX_DEFAULT_VECTOR_SIZE>
class vector
{
private:
    T data_[N];
    unsigned int size_;
    bool empty_;
public:
    vector() : 
        size_(-1),
        empty_(true)
    {}

    ~vector() {}

    unsigned int size(void) const
    {
        return size_ + 1;
    }

    void clear()
    {
        size_ = -1;
        empty_ = true;
    }

    void push_back (const T& x)
    { 
        if (size() < N) {
            size_++;  
            data_[size_] = x;
            empty_ = false;
        }
    }

    void pop_back(void)
    {
        if (!empty_) {
            data_[size_].~T();
            size_--;
            if (size_ == -1) {
                empty_ = true;
            }
        }
    }
  
    vector(const vector<T, N>& vec) : 
        size_(vec.size_),
        empty_(vec.empty_)
    {
        if (!empty_) {
            memcpy(&data_[0], &vec.data_[0], size() * sizeof(T));
        }
    } 

    vector(unsigned int size, const T& val = T()) :
        size_(-1),
        empty_(true)
    {
        for (unsigned int i = 0; i < size; i++) {
            push_back(val);
        }
    }

    vector<T, N>& operator=(const vector<T, N>& rhs)
    {
        if (this == &rhs) {
            return *this;
        }

        size_  = rhs.size_;
        empty_ = rhs.empty_;

        if (!empty_) {	
            memcpy(&data_[0], &rhs.data_[0], size() * sizeof(T));
        }
    
        return *this;
    }

    bool operator==(vector<T,N> &vec)
    {
        if (empty_ && vec.empty_) {
            return true;
        }

        if (size() != vec.size()) {
            return false;
        }

        return memcmp(&data_[0], &vec.data_[0], size() * sizeof(T)) == 0 ? true : false;
    }
  
    operator T* ()             { return data_; }
    operator const T* () const { return data_; }
   
    bool empty (void) const
    {
        return empty_;
    }
  
    unsigned int max_size (void) const
    {
        return N;
    }

    unsigned int capacity () const
    {
        return sizeof(T) * N;
    }

    T& operator[](int index)
    {
        return data_[index];
    }
  
    T operator[](int index) const
    {
        return data_[index];
    }
  
    template<class I>
    void assign(I start, I end)
    {
        clear();   
        while(start < end) {
            push_back(*start);
            start++;
        }
    }

    /*! \class iterator
     * \brief Iterator class for vectors
     */
    class iterator
    {
    private:
        vector<T,N> vec_;
        int index_;
        bool initialized_;
    public:
        iterator(void) : 
            index_(-1),
            initialized_(false)
        {
            index_ = -1;
            initialized_ = false;
        }

        ~iterator(void) {}

        static iterator begin(vector<T,N> &vec)
        {
            iterator i;

            if (!vec.empty()) {
                i.index_ = 0;
            }

            i.vec_ = vec;
            i.initialized_ = true;
            return i;
        }

        static iterator end(vector<T,N> &vec)
        {
            iterator i;

            if (!vec.empty()) {
                i.index_ = vec.size();
            }
            i.vec_ = vec;
            i.initialized_ = true;
            return i;
        }
    
        bool operator==(iterator i)
        {
            return ((vec_ == i.vec_) && 
                    (index_ == i.index_) && 
                    (initialized_ == i.initialized_));
        }

        bool operator!=(iterator i)
        {
            return (!(*this==i));
        }

        void operator++()
        {
            index_++;
        }

        void operator++(int x)
        {
            index_ += x;
        }

        void operator--()
        {
            index_--;
        }

        void operator--(int x)
        {
            index_ -= x;
        }

        T operator *()
        {
            return vec_[index_];
        }
    };

    iterator begin(void)
    {
        return iterator::begin(*this);
    }

    iterator end(void)
    {
        return iterator::end(*this);
    }

    T& front(void)
    {
        return data_[0];
    }

    T& back(void)
    {
        return data_[size_];
    }

    const T& front(void) const
    {
        return data_[0];
    }

    const T& back(void) const
    {
        return data_[size_];
    }
};  
    
/*!
 * \brief size_t class used to interface between C++ and
 * OpenCL C calls that require arrays of size_t values, who's
 * size is known statically.
 */
template <int N>
struct size_t : public cl::vector< ::size_t, N> { };

namespace detail {

// GetInfo help struct
template <typename Functor, typename T>
struct GetInfoHelper
{
    static cl_int
    get(Functor f, cl_uint name, T* param)
    {
        return f(name, sizeof(T), param, NULL);
    }
};

// Specialized GetInfoHelper for VECTOR_CLASS params
template <typename Func, typename T>
struct GetInfoHelper<Func, VECTOR_CLASS<T> >
{
    static cl_int get(Func f, cl_uint name, VECTOR_CLASS<T>* param)
    {
        ::size_t required;
        cl_int err = f(name, 0, NULL, &required);
        if (err != CL_SUCCESS) {
            return err;
        }

        T* value = (T*) alloca(required);
        err = f(name, required, value, NULL);
        if (err != CL_SUCCESS) {
            return err;
        }

        param->assign(&value[0], &value[required/sizeof(T)]);
        return CL_SUCCESS;
    }
};

// Specialized for getInfo<CL_PROGRAM_BINARIES>
template <typename Func>
struct GetInfoHelper<Func, VECTOR_CLASS<char *> >
{
    static cl_int
    get(Func f, cl_uint name, VECTOR_CLASS<char *>* param)
    {
      cl_uint err = f(name, param->size() * sizeof(char *), &(*param)[0], NULL);
      if (err != CL_SUCCESS) {
        return err;
      }
      
      return CL_SUCCESS;
    }
};

// Specialized GetInfoHelper for STRING_CLASS params
template <typename Func>
struct GetInfoHelper<Func, STRING_CLASS>
{
    static cl_int get(Func f, cl_uint name, STRING_CLASS* param)
    {
        ::size_t required;
        cl_int err = f(name, 0, NULL, &required);
        if (err != CL_SUCCESS) {
            return err;
        }

        char* value = (char*) alloca(required);
        err = f(name, required, value, NULL);
        if (err != CL_SUCCESS) {
            return err;
        }

        *param = value;
        return CL_SUCCESS;
    }
};

#define __GET_INFO_HELPER_WITH_RETAIN(CPP_TYPE) \
namespace detail { \
template <typename Func> \
struct GetInfoHelper<Func, CPP_TYPE> \
{ \
    static cl_int get(Func f, cl_uint name, CPP_TYPE* param) \
    { \
      cl_uint err = f(name, sizeof(CPP_TYPE), param, NULL); \
      if (err != CL_SUCCESS) { \
        return err; \
      } \
      \
      return ReferenceHandler<CPP_TYPE::cl_type>::retain((*param)()); \
    } \
}; \
} 


#define __PARAM_NAME_INFO_1_0(F) \
    F(cl_platform_info, CL_PLATFORM_PROFILE, STRING_CLASS) \
    F(cl_platform_info, CL_PLATFORM_VERSION, STRING_CLASS) \
    F(cl_platform_info, CL_PLATFORM_NAME, STRING_CLASS) \
    F(cl_platform_info, CL_PLATFORM_VENDOR, STRING_CLASS) \
    F(cl_platform_info, CL_PLATFORM_EXTENSIONS, STRING_CLASS) \
    \
    F(cl_device_info, CL_DEVICE_TYPE, cl_device_type) \
    F(cl_device_info, CL_DEVICE_VENDOR_ID, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_WORK_GROUP_SIZE, ::size_t) \
    F(cl_device_info, CL_DEVICE_MAX_WORK_ITEM_SIZES, VECTOR_CLASS< ::size_t>) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_CLOCK_FREQUENCY, cl_uint) \
    F(cl_device_info, CL_DEVICE_ADDRESS_BITS, cl_bitfield) \
    F(cl_device_info, CL_DEVICE_MAX_READ_IMAGE_ARGS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_MEM_ALLOC_SIZE, cl_ulong) \
    F(cl_device_info, CL_DEVICE_IMAGE2D_MAX_WIDTH, ::size_t) \
    F(cl_device_info, CL_DEVICE_IMAGE2D_MAX_HEIGHT, ::size_t) \
    F(cl_device_info, CL_DEVICE_IMAGE3D_MAX_WIDTH, ::size_t) \
    F(cl_device_info, CL_DEVICE_IMAGE3D_MAX_HEIGHT, ::size_t) \
    F(cl_device_info, CL_DEVICE_IMAGE3D_MAX_DEPTH, ::size_t) \
    F(cl_device_info, CL_DEVICE_IMAGE_SUPPORT, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_PARAMETER_SIZE, ::size_t) \
    F(cl_device_info, CL_DEVICE_MAX_SAMPLERS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MEM_BASE_ADDR_ALIGN, cl_uint) \
    F(cl_device_info, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, cl_uint) \
    F(cl_device_info, CL_DEVICE_SINGLE_FP_CONFIG, cl_device_fp_config) \
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, cl_device_mem_cache_type) \
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, cl_uint)\
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, cl_ulong) \
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_SIZE, cl_ulong) \
    F(cl_device_info, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, cl_ulong) \
    F(cl_device_info, CL_DEVICE_MAX_CONSTANT_ARGS, cl_uint) \
    F(cl_device_info, CL_DEVICE_LOCAL_MEM_TYPE, cl_device_local_mem_type) \
    F(cl_device_info, CL_DEVICE_LOCAL_MEM_SIZE, cl_ulong) \
    F(cl_device_info, CL_DEVICE_ERROR_CORRECTION_SUPPORT, cl_bool) \
    F(cl_device_info, CL_DEVICE_PROFILING_TIMER_RESOLUTION, ::size_t) \
    F(cl_device_info, CL_DEVICE_ENDIAN_LITTLE, cl_bool) \
    F(cl_device_info, CL_DEVICE_AVAILABLE, cl_bool) \
    F(cl_device_info, CL_DEVICE_COMPILER_AVAILABLE, cl_bool) \
    F(cl_device_info, CL_DEVICE_EXECUTION_CAPABILITIES, cl_device_exec_capabilities) \
    F(cl_device_info, CL_DEVICE_QUEUE_PROPERTIES, cl_command_queue_properties) \
    F(cl_device_info, CL_DEVICE_PLATFORM, cl_platform_id) \
    F(cl_device_info, CL_DEVICE_NAME, STRING_CLASS) \
    F(cl_device_info, CL_DEVICE_VENDOR, STRING_CLASS) \
    F(cl_device_info, CL_DRIVER_VERSION, STRING_CLASS) \
    F(cl_device_info, CL_DEVICE_PROFILE, STRING_CLASS) \
    F(cl_device_info, CL_DEVICE_VERSION, STRING_CLASS) \
    F(cl_device_info, CL_DEVICE_EXTENSIONS, STRING_CLASS) \
    \
    F(cl_context_info, CL_CONTEXT_REFERENCE_COUNT, cl_uint) \
    F(cl_context_info, CL_CONTEXT_DEVICES, VECTOR_CLASS<Device>) \
    F(cl_context_info, CL_CONTEXT_PROPERTIES, VECTOR_CLASS<cl_context_properties>) \
    \
    F(cl_event_info, CL_EVENT_COMMAND_QUEUE, cl::CommandQueue) \
    F(cl_event_info, CL_EVENT_COMMAND_TYPE, cl_command_type) \
    F(cl_event_info, CL_EVENT_REFERENCE_COUNT, cl_uint) \
    F(cl_event_info, CL_EVENT_COMMAND_EXECUTION_STATUS, cl_uint) \
    \
    F(cl_profiling_info, CL_PROFILING_COMMAND_QUEUED, cl_ulong) \
    F(cl_profiling_info, CL_PROFILING_COMMAND_SUBMIT, cl_ulong) \
    F(cl_profiling_info, CL_PROFILING_COMMAND_START, cl_ulong) \
    F(cl_profiling_info, CL_PROFILING_COMMAND_END, cl_ulong) \
    \
    F(cl_mem_info, CL_MEM_TYPE, cl_mem_object_type) \
    F(cl_mem_info, CL_MEM_FLAGS, cl_mem_flags) \
    F(cl_mem_info, CL_MEM_SIZE, ::size_t) \
    F(cl_mem_info, CL_MEM_HOST_PTR, void*) \
    F(cl_mem_info, CL_MEM_MAP_COUNT, cl_uint) \
    F(cl_mem_info, CL_MEM_REFERENCE_COUNT, cl_uint) \
    F(cl_mem_info, CL_MEM_CONTEXT, cl::Context) \
    \
    F(cl_image_info, CL_IMAGE_FORMAT, cl_image_format) \
    F(cl_image_info, CL_IMAGE_ELEMENT_SIZE, ::size_t) \
    F(cl_image_info, CL_IMAGE_ROW_PITCH, ::size_t) \
    F(cl_image_info, CL_IMAGE_SLICE_PITCH, ::size_t) \
    F(cl_image_info, CL_IMAGE_WIDTH, ::size_t) \
    F(cl_image_info, CL_IMAGE_HEIGHT, ::size_t) \
    F(cl_image_info, CL_IMAGE_DEPTH, ::size_t) \
    \
    F(cl_sampler_info, CL_SAMPLER_REFERENCE_COUNT, cl_uint) \
    F(cl_sampler_info, CL_SAMPLER_CONTEXT, cl::Context) \
    F(cl_sampler_info, CL_SAMPLER_NORMALIZED_COORDS, cl_addressing_mode) \
    F(cl_sampler_info, CL_SAMPLER_ADDRESSING_MODE, cl_filter_mode) \
    F(cl_sampler_info, CL_SAMPLER_FILTER_MODE, cl_bool) \
    \
    F(cl_program_info, CL_PROGRAM_REFERENCE_COUNT, cl_uint) \
    F(cl_program_info, CL_PROGRAM_CONTEXT, cl::Context) \
    F(cl_program_info, CL_PROGRAM_NUM_DEVICES, cl_uint) \
    F(cl_program_info, CL_PROGRAM_DEVICES, VECTOR_CLASS<cl_device_id>) \
    F(cl_program_info, CL_PROGRAM_SOURCE, STRING_CLASS) \
    F(cl_program_info, CL_PROGRAM_BINARY_SIZES, VECTOR_CLASS< ::size_t>) \
    F(cl_program_info, CL_PROGRAM_BINARIES, VECTOR_CLASS<char *>) \
    \
    F(cl_program_build_info, CL_PROGRAM_BUILD_STATUS, cl_build_status) \
    F(cl_program_build_info, CL_PROGRAM_BUILD_OPTIONS, STRING_CLASS) \
    F(cl_program_build_info, CL_PROGRAM_BUILD_LOG, STRING_CLASS) \
    \
    F(cl_kernel_info, CL_KERNEL_FUNCTION_NAME, STRING_CLASS) \
    F(cl_kernel_info, CL_KERNEL_NUM_ARGS, cl_uint) \
    F(cl_kernel_info, CL_KERNEL_REFERENCE_COUNT, cl_uint) \
    F(cl_kernel_info, CL_KERNEL_CONTEXT, cl::Context) \
    F(cl_kernel_info, CL_KERNEL_PROGRAM, cl::Program) \
    \
    F(cl_kernel_work_group_info, CL_KERNEL_WORK_GROUP_SIZE, ::size_t) \
    F(cl_kernel_work_group_info, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, cl::size_t<3>) \
    F(cl_kernel_work_group_info, CL_KERNEL_LOCAL_MEM_SIZE, cl_ulong) \
    \
    F(cl_command_queue_info, CL_QUEUE_CONTEXT, cl::Context) \
    F(cl_command_queue_info, CL_QUEUE_DEVICE, cl::Device) \
    F(cl_command_queue_info, CL_QUEUE_REFERENCE_COUNT, cl_uint) \
    F(cl_command_queue_info, CL_QUEUE_PROPERTIES, cl_command_queue_properties)

#if defined(CL_VERSION_1_1)
#define __PARAM_NAME_INFO_1_1(F) \
    F(cl_context_info, CL_CONTEXT_NUM_DEVICES, cl_uint)\
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, cl_uint) \
    F(cl_device_info, CL_DEVICE_DOUBLE_FP_CONFIG, cl_device_fp_config) \
    F(cl_device_info, CL_DEVICE_HALF_FP_CONFIG, cl_device_fp_config) \
    F(cl_device_info, CL_DEVICE_HOST_UNIFIED_MEMORY, cl_bool) \
    \
    F(cl_mem_info, CL_MEM_ASSOCIATED_MEMOBJECT, cl::Memory) \
    F(cl_mem_info, CL_MEM_OFFSET, ::size_t) \
    \
    F(cl_kernel_work_group_info, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, ::size_t) \
    F(cl_kernel_work_group_info, CL_KERNEL_PRIVATE_MEM_SIZE, cl_ulong) \
    \
    F(cl_event_info, CL_EVENT_CONTEXT, cl::Context)
#endif // CL_VERSION_1_1

#if defined(USE_CL_DEVICE_FISSION)
#define __PARAM_NAME_DEVICE_FISSION(F) \
    F(cl_device_info, CL_DEVICE_PARENT_DEVICE_EXT, cl_device_id) \
	F(cl_device_info, CL_DEVICE_PARTITION_TYPES_EXT, VECTOR_CLASS<cl_device_partition_property_ext>) \
	F(cl_device_info, CL_DEVICE_AFFINITY_DOMAINS_EXT, VECTOR_CLASS<cl_device_partition_property_ext>) \
	F(cl_device_info, CL_DEVICE_REFERENCE_COUNT_EXT , cl_uint) \
	F(cl_device_info, CL_DEVICE_PARTITION_STYLE_EXT, VECTOR_CLASS<cl_device_partition_property_ext>)
#endif // USE_CL_DEVICE_FISSION

template <typename enum_type, cl_int Name>
struct param_traits {};

#define __DECLARE_PARAM_TRAITS(token, param_name, T) \
struct token;                                        \
template<>                                           \
struct param_traits<detail:: token,param_name>       \
{                                                    \
    enum { value = param_name };                     \
    typedef T param_type;                            \
};

__PARAM_NAME_INFO_1_0(__DECLARE_PARAM_TRAITS);
#if defined(CL_VERSION_1_1)
__PARAM_NAME_INFO_1_1(__DECLARE_PARAM_TRAITS);
#endif // CL_VERSION_1_1

#if defined(USE_CL_DEVICE_FISSION)
__PARAM_NAME_DEVICE_FISSION(__DECLARE_PARAM_TRAITS);
#endif // USE_CL_DEVICE_FISSION

#undef __DECLARE_PARAM_TRAITS

// Convenience functions

template <typename Func, typename T>
inline cl_int
getInfo(Func f, cl_uint name, T* param)
{
    return GetInfoHelper<Func, T>::get(f, name, param);
}

template <typename Func, typename Arg0>
struct GetInfoFunctor0
{
    Func f_; const Arg0& arg0_;
    cl_int operator ()(
        cl_uint param, ::size_t size, void* value, ::size_t* size_ret)
    { return f_(arg0_, param, size, value, size_ret); }
};

template <typename Func, typename Arg0, typename Arg1>
struct GetInfoFunctor1
{
    Func f_; const Arg0& arg0_; const Arg1& arg1_;
    cl_int operator ()(
        cl_uint param, ::size_t size, void* value, ::size_t* size_ret)
    { return f_(arg0_, arg1_, param, size, value, size_ret); }
};

template <typename Func, typename Arg0, typename T>
inline cl_int
getInfo(Func f, const Arg0& arg0, cl_uint name, T* param)
{
    GetInfoFunctor0<Func, Arg0> f0 = { f, arg0 };
    return GetInfoHelper<GetInfoFunctor0<Func, Arg0>, T>
        ::get(f0, name, param);
}

template <typename Func, typename Arg0, typename Arg1, typename T>
inline cl_int
getInfo(Func f, const Arg0& arg0, const Arg1& arg1, cl_uint name, T* param)
{
    GetInfoFunctor1<Func, Arg0, Arg1> f0 = { f, arg0, arg1 };
    return GetInfoHelper<GetInfoFunctor1<Func, Arg0, Arg1>, T>
        ::get(f0, name, param);
}

template<typename T>
struct ReferenceHandler
{ };

template <>
struct ReferenceHandler<cl_device_id>
{
    // cl_device_id does not have retain().
    static cl_int retain(cl_device_id)
    { return CL_INVALID_DEVICE; }
    // cl_device_id does not have release().
    static cl_int release(cl_device_id)
    { return CL_INVALID_DEVICE; }
};

template <>
struct ReferenceHandler<cl_platform_id>
{
    // cl_platform_id does not have retain().
    static cl_int retain(cl_platform_id)
    { return CL_INVALID_PLATFORM; }
    // cl_platform_id does not have release().
    static cl_int release(cl_platform_id)
    { return CL_INVALID_PLATFORM; }
};

template <>
struct ReferenceHandler<cl_context>
{
    static cl_int retain(cl_context context)
    { return ::clRetainContext(context); }
    static cl_int release(cl_context context)
    { return ::clReleaseContext(context); }
};

template <>
struct ReferenceHandler<cl_command_queue>
{
    static cl_int retain(cl_command_queue queue)
    { return ::clRetainCommandQueue(queue); }
    static cl_int release(cl_command_queue queue)
    { return ::clReleaseCommandQueue(queue); }
};

template <>
struct ReferenceHandler<cl_mem>
{
    static cl_int retain(cl_mem memory)
    { return ::clRetainMemObject(memory); }
    static cl_int release(cl_mem memory)
    { return ::clReleaseMemObject(memory); }
};

template <>
struct ReferenceHandler<cl_sampler>
{
    static cl_int retain(cl_sampler sampler)
    { return ::clRetainSampler(sampler); }
    static cl_int release(cl_sampler sampler)
    { return ::clReleaseSampler(sampler); }
};

template <>
struct ReferenceHandler<cl_program>
{
    static cl_int retain(cl_program program)
    { return ::clRetainProgram(program); }
    static cl_int release(cl_program program)
    { return ::clReleaseProgram(program); }
};

template <>
struct ReferenceHandler<cl_kernel>
{
    static cl_int retain(cl_kernel kernel)
    { return ::clRetainKernel(kernel); }
    static cl_int release(cl_kernel kernel)
    { return ::clReleaseKernel(kernel); }
};

template <>
struct ReferenceHandler<cl_event>
{
    static cl_int retain(cl_event event)
    { return ::clRetainEvent(event); }
    static cl_int release(cl_event event)
    { return ::clReleaseEvent(event); }
};

template <typename T>
class Wrapper
{
public:
    typedef T cl_type;

protected:
    cl_type object_;

public:
    Wrapper() : object_(NULL) { }

    ~Wrapper()
    {
        if (object_ != NULL) { release(); }
    }

    Wrapper(const Wrapper<cl_type>& rhs)
    {
        object_ = rhs.object_;
        if (object_ != NULL) { retain(); }
    }

    Wrapper<cl_type>& operator = (const Wrapper<cl_type>& rhs)
    {
        if (object_ != NULL) { release(); }
        object_ = rhs.object_;
        if (object_ != NULL) { retain(); }
        return *this;
    }

    cl_type operator ()() const { return object_; }

    cl_type& operator ()() { return object_; }

protected:

    cl_int retain() const
    {
        return ReferenceHandler<cl_type>::retain(object_);
    }

    cl_int release() const
    {
        return ReferenceHandler<cl_type>::release(object_);
    }
};

#if defined(__CL_ENABLE_EXCEPTIONS)
static inline cl_int errHandler (
    cl_int err,
    const char * errStr = NULL) throw(Error)
{
    if (err != CL_SUCCESS) {
        throw Error(err, errStr);
    }
    return err;
}
#else
static inline cl_int errHandler (cl_int err, const char * errStr = NULL)
{
    return err;
}
#endif // __CL_ENABLE_EXCEPTIONS

} // namespace detail
//! \endcond

/*! \stuct ImageFormat
 * \brief ImageFormat interface fro cl_image_format.
 */
struct ImageFormat : public cl_image_format
{
    ImageFormat(){}

    ImageFormat(cl_channel_order order, cl_channel_type type)
    {
        image_channel_order = order;
        image_channel_data_type = type;
    }

    ImageFormat& operator = (const ImageFormat& rhs)
    {
        if (this != &rhs) {
            this->image_channel_data_type = rhs.image_channel_data_type;
            this->image_channel_order     = rhs.image_channel_order;
        }
        return *this;
    }
};

/*! \class Device
 * \brief Device interface for cl_device_id.
 */
class Device : public detail::Wrapper<cl_device_id>
{
public:
    Device(cl_device_id device) { object_ = device; }

    Device() : detail::Wrapper<cl_type>() { }

    Device(const Device& device) : detail::Wrapper<cl_type>(device) { }

    Device& operator = (const Device& rhs)
    {
        if (this != &rhs) {
            detail::Wrapper<cl_type>::operator=(rhs);
        }
        return *this;
    }

    template <typename T>
    cl_int getInfo(cl_device_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(&::clGetDeviceInfo, object_, name, param),
            __GET_DEVICE_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_device_info, name>::param_type
    getInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_device_info, name>::param_type param;
        cl_int result = getInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

#if defined(USE_CL_DEVICE_FISSION)
	cl_int createSubDevices(
		const cl_device_partition_property_ext * properties,
		VECTOR_CLASS<Device>* devices)
	{
		typedef CL_API_ENTRY cl_int 
			( CL_API_CALL * PFN_clCreateSubDevicesEXT)(
				cl_device_id /*in_device*/,
                const cl_device_partition_property_ext * /* properties */,
                cl_uint /*num_entries*/,
                cl_device_id * /*out_devices*/,
                cl_uint * /*num_devices*/ ) CL_EXT_SUFFIX__VERSION_1_1;

		static PFN_clCreateSubDevicesEXT pfn_clCreateSubDevicesEXT = NULL;
		__INIT_CL_EXT_FCN_PTR(clCreateSubDevicesEXT);

		cl_uint n = 0;
        cl_int err = pfn_clCreateSubDevicesEXT(object_, properties, 0, NULL, &n);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __CREATE_SUB_DEVICES);
        }

        cl_device_id* ids = (cl_device_id*) alloca(n * sizeof(cl_device_id));
        err = pfn_clCreateSubDevicesEXT(object_, properties, n, ids, NULL);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __CREATE_SUB_DEVICES);
        }

        devices->assign(&ids[0], &ids[n]);
        return CL_SUCCESS;
 	}
#endif
};

/*! \class Platform
 *  \brief Platform interface.
 */
class Platform : public detail::Wrapper<cl_platform_id>
{
public:
    static const Platform null();

    Platform(cl_platform_id platform) { object_ = platform; }

    Platform() : detail::Wrapper<cl_type>()  { }

    Platform(const Platform& platform) : detail::Wrapper<cl_type>(platform) { }

    Platform& operator = (const Platform& rhs)
    {
        if (this != &rhs) {
            detail::Wrapper<cl_type>::operator=(rhs);
        }
        return *this;
    }

    cl_int getInfo(cl_platform_info name, STRING_CLASS* param) const
    {
        return detail::errHandler(
            detail::getInfo(&::clGetPlatformInfo, object_, name, param),
            __GET_PLATFORM_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_platform_info, name>::param_type
    getInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_platform_info, name>::param_type param;
        cl_int result = getInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

    cl_int getDevices(
        cl_device_type type,
        VECTOR_CLASS<Device>* devices) const
    {
        cl_uint n = 0;
        cl_int err = ::clGetDeviceIDs(object_, type, 0, NULL, &n);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __GET_DEVICE_IDS_ERR);
        }

        cl_device_id* ids = (cl_device_id*) alloca(n * sizeof(cl_device_id));
        err = ::clGetDeviceIDs(object_, type, n, ids, NULL);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __GET_DEVICE_IDS_ERR);
        }

        devices->assign(&ids[0], &ids[n]);
        return CL_SUCCESS;
    }

#if defined(USE_DX_INTEROP)
   /*! \brief Get the list of available D3D10 devices.
     *
     *  \param d3d_device_source.
     *
     *  \param d3d_object.
     *
     *  \param d3d_device_set.
     *
     *  \param devices returns a vector of OpenCL D3D10 devices found. The cl::Device
     *  values returned in devices can be used to identify a specific OpenCL
     *  device. If \a devices argument is NULL, this argument is ignored.
     *
     *  \return One of the following values:
     *    - CL_SUCCESS if the function is executed successfully.
     *
     *  The application can query specific capabilities of the OpenCL device(s)
     *  returned by cl::getDevices. This can be used by the application to
     *  determine which device(s) to use.
     *
     * \note In the case that exceptions are enabled and a return value
     * other than CL_SUCCESS is generated, then cl::Error exception is
     * generated.
     */
    cl_int getDevices(
        cl_d3d10_device_source_khr d3d_device_source,
        void *                     d3d_object,
        cl_d3d10_device_set_khr    d3d_device_set,
        VECTOR_CLASS<Device>* devices) const
    {
        typedef CL_API_ENTRY cl_int (CL_API_CALL *PFN_clGetDeviceIDsFromD3D10KHR)(
            cl_platform_id platform, 
            cl_d3d10_device_source_khr d3d_device_source, 
            void * d3d_object,
            cl_d3d10_device_set_khr d3d_device_set,
            cl_uint num_entries,
            cl_device_id * devices,
            cl_uint* num_devices);

        static PFN_clGetDeviceIDsFromD3D10KHR pfn_clGetDeviceIDsFromD3D10KHR = NULL;
        __INIT_CL_EXT_FCN_PTR(clGetDeviceIDsFromD3D10KHR);

        cl_uint n = 0;
        cl_int err = pfn_clGetDeviceIDsFromD3D10KHR(
            object_, 
            d3d_device_source, 
            d3d_object,
            d3d_device_set, 
            0, 
            NULL, 
            &n);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __GET_DEVICE_IDS_ERR);
        }

        cl_device_id* ids = (cl_device_id*) alloca(n * sizeof(cl_device_id));
        err = pfn_clGetDeviceIDsFromD3D10KHR(
            object_, 
            d3d_device_source, 
            d3d_object,
            d3d_device_set,
            n, 
            ids, 
            NULL);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __GET_DEVICE_IDS_ERR);
        }

        devices->assign(&ids[0], &ids[n]);
        return CL_SUCCESS;
    }
#endif

    static cl_int get(
        VECTOR_CLASS<Platform>* platforms)
    {
        cl_uint n = 0;
        cl_int err = ::clGetPlatformIDs(0, NULL, &n);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __GET_PLATFORM_IDS_ERR);
        }

        cl_platform_id* ids = (cl_platform_id*) alloca(
            n * sizeof(cl_platform_id));
        err = ::clGetPlatformIDs(n, ids, NULL);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __GET_PLATFORM_IDS_ERR);
        }

        platforms->assign(&ids[0], &ids[n]);
        return CL_SUCCESS;
    }
};

static inline cl_int
UnloadCompiler()
{
    return ::clUnloadCompiler();
}

class Context : public detail::Wrapper<cl_context>
{
public:
    Context(
        const VECTOR_CLASS<Device>& devices,
        cl_context_properties* properties = NULL,
        void (CL_CALLBACK * notifyFptr)(
            const char *,
            const void *,
            ::size_t,
            void *) = NULL,
        void* data = NULL,
        cl_int* err = NULL)
    {
        cl_int error;
        object_ = ::clCreateContext(
            properties, (cl_uint) devices.size(),
            (cl_device_id*) &devices.front(),
            notifyFptr, data, &error);

        detail::errHandler(error, __CREATE_CONTEXT_FROM_TYPE_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    Context(
        cl_device_type type,
        cl_context_properties* properties = NULL,
        void (CL_CALLBACK * notifyFptr)(
            const char *,
            const void *,
            ::size_t,
            void *) = NULL,
        void* data = NULL,
        cl_int* err = NULL)
    {
        cl_int error;
        object_ = ::clCreateContextFromType(
            properties, type, notifyFptr, data, &error);

        detail::errHandler(error, __CREATE_CONTEXT_FROM_TYPE_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    Context() : detail::Wrapper<cl_type>() { }

    Context(const Context& context) : detail::Wrapper<cl_type>(context) { }

    Context& operator = (const Context& rhs)
    {
        if (this != &rhs) {
            detail::Wrapper<cl_type>::operator=(rhs);
        }
        return *this;
    }

    template <typename T>
    cl_int getInfo(cl_context_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(&::clGetContextInfo, object_, name, param),
            __GET_CONTEXT_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_context_info, name>::param_type
    getInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_context_info, name>::param_type param;
        cl_int result = getInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

    cl_int getSupportedImageFormats(
        cl_mem_flags flags,
        cl_mem_object_type type,
        VECTOR_CLASS<ImageFormat>* formats) const
    {
        cl_uint numEntries;
        cl_int err = ::clGetSupportedImageFormats(
           object_, 
           flags,
           type, 
           0, 
           NULL, 
           &numEntries);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __GET_SUPPORTED_IMAGE_FORMATS_ERR);
        }

        ImageFormat* value = (ImageFormat*)
            alloca(numEntries * sizeof(ImageFormat));
        err = ::clGetSupportedImageFormats(
            object_, 
            flags, 
            type, 
            numEntries,
            (cl_image_format*) value, 
            NULL);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __GET_SUPPORTED_IMAGE_FORMATS_ERR);
        }

        formats->assign(&value[0], &value[numEntries]);
        return CL_SUCCESS;
    }
};

__GET_INFO_HELPER_WITH_RETAIN(cl::Context)

/*! \class Event
 * \brief Event interface for cl_event.
 */
class Event : public detail::Wrapper<cl_event>
{
public:
    Event() : detail::Wrapper<cl_type>() { }

    Event(const Event& event) : detail::Wrapper<cl_type>(event) { }

    Event& operator = (const Event& rhs)
    {
        if (this != &rhs) {
            detail::Wrapper<cl_type>::operator=(rhs);
        }
        return *this;
    }

    template <typename T>
    cl_int getInfo(cl_event_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(&::clGetEventInfo, object_, name, param),
            __GET_EVENT_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_event_info, name>::param_type
    getInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_event_info, name>::param_type param;
        cl_int result = getInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

    template <typename T>
    cl_int getProfilingInfo(cl_profiling_info name, T* param) const
    {
        return detail::errHandler(detail::getInfo(
            &::clGetEventProfilingInfo, object_, name, param),
            __GET_EVENT_PROFILE_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_profiling_info, name>::param_type
    getProfilingInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_profiling_info, name>::param_type param;
        cl_int result = getProfilingInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

    cl_int wait() const
    {
        return detail::errHandler(
            ::clWaitForEvents(1, &object_),
            __WAIT_FOR_EVENTS_ERR);
    }

#if defined(CL_VERSION_1_1)
    cl_int setCallback(
        cl_int type,
        void (CL_CALLBACK * pfn_notify)(cl_event, cl_int, void *),		
        void * user_data = NULL)
    {
        return detail::errHandler(
            ::clSetEventCallback(
                object_,
                type,
                pfn_notify,
                user_data), 
            __SET_EVENT_CALLBACK_ERR);
    }
#endif

    static cl_int
    waitForEvents(const VECTOR_CLASS<Event>& events)
    {
        return detail::errHandler(
            ::clWaitForEvents(
                (cl_uint) events.size(), (cl_event*)&events.front()),
            __WAIT_FOR_EVENTS_ERR);
    }
};

__GET_INFO_HELPER_WITH_RETAIN(cl::Event)

#if defined(CL_VERSION_1_1)
/*! \class UserEvent
 * \brief User event interface for cl_event.
 */
class UserEvent : public Event
{
public:
    UserEvent(
        const Context& context,
        cl_int * err = NULL)
    {
        cl_int error;
        object_ = ::clCreateUserEvent(
            context(),
            &error);

        detail::errHandler(error, __CREATE_USER_EVENT_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    UserEvent() : Event() { }

    UserEvent(const UserEvent& event) : Event(event) { }

    UserEvent& operator = (const UserEvent& rhs)
    {
        if (this != &rhs) {
            Event::operator=(rhs);
        }
        return *this;
    }

    cl_int setStatus(cl_int status)
    {
        return detail::errHandler(
            ::clSetUserEventStatus(object_,status), 
            __SET_USER_EVENT_STATUS_ERR);
    }
};
#endif

inline static cl_int
WaitForEvents(const VECTOR_CLASS<Event>& events)
{
    return detail::errHandler(
        ::clWaitForEvents(
            (cl_uint) events.size(), (cl_event*)&events.front()),
        __WAIT_FOR_EVENTS_ERR);
}

/*! \class Memory
 * \brief Memory interface for cl_mem.
 */
class Memory : public detail::Wrapper<cl_mem>
{
public:
    Memory() : detail::Wrapper<cl_type>() { }

    Memory(const Memory& memory) : detail::Wrapper<cl_type>(memory) { }

    Memory& operator = (const Memory& rhs)
    {
        if (this != &rhs) {
            detail::Wrapper<cl_type>::operator=(rhs);
        }
        return *this;
    }

    template <typename T>
    cl_int getInfo(cl_mem_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(&::clGetMemObjectInfo, object_, name, param),
            __GET_MEM_OBJECT_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_mem_info, name>::param_type
    getInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_mem_info, name>::param_type param;
        cl_int result = getInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

#if defined(CL_VERSION_1_1)
    cl_int setDestructorCallback(
        void (CL_CALLBACK * pfn_notify)(cl_mem, void *),		
        void * user_data = NULL)
    {
        return detail::errHandler(
            ::clSetMemObjectDestructorCallback(
                object_,
                pfn_notify,
                user_data), 
            __SET_MEM_OBJECT_DESTRUCTOR_CALLBACK_ERR);
    }
#endif

};

__GET_INFO_HELPER_WITH_RETAIN(cl::Memory)

/*! \class Buffer
 * \brief Memory buffer interface.
 */
class Buffer : public Memory
{
public:
    Buffer(
        const Context& context,
        cl_mem_flags flags,
        ::size_t size,
        void* host_ptr = NULL,
        cl_int* err = NULL)
    {
        cl_int error;
        object_ = ::clCreateBuffer(context(), flags, size, host_ptr, &error);

        detail::errHandler(error, __CREATE_BUFFER_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    Buffer() : Memory() { }

    Buffer(const Buffer& buffer) : Memory(buffer) { }

    Buffer& operator = (const Buffer& rhs)
    {
        if (this != &rhs) {
            Memory::operator=(rhs);
        }
        return *this;
    }

#if defined(CL_VERSION_1_1)
    Buffer createSubBuffer(
        cl_mem_flags flags,
        cl_buffer_create_type buffer_create_type,
        const void * buffer_create_info,
        cl_int * err = NULL)
    {
        Buffer result;
        cl_int error;
        result.object_ = ::clCreateSubBuffer(
            object_, 
            flags, 
            buffer_create_type, 
            buffer_create_info, 
            &error);

        detail::errHandler(error, __CREATE_SUBBUFFER_ERR);
        if (err != NULL) {
            *err = error;
        }

        return result;
	}		
#endif
};

#if defined (USE_DX_INTEROP)
class BufferD3D10 : public Buffer
{
public:
    typedef CL_API_ENTRY cl_mem (CL_API_CALL *PFN_clCreateFromD3D10BufferKHR)(
    cl_context context, cl_mem_flags flags, ID3D10Buffer*  buffer,
    cl_int* errcode_ret);

    BufferD3D10(
        const Context& context,
        cl_mem_flags flags,
        ID3D10Buffer* bufobj,
        cl_int * err = NULL)
    {
        static PFN_clCreateFromD3D10BufferKHR pfn_clCreateFromD3D10BufferKHR = NULL;
        __INIT_CL_EXT_FCN_PTR(clCreateFromD3D10BufferKHR);

        cl_int error;
        object_ = pfn_clCreateFromD3D10BufferKHR(
            context(),
            flags,
            bufobj,
            &error);

        detail::errHandler(error, __CREATE_GL_BUFFER_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    BufferD3D10() : Buffer() { }

    BufferD3D10(const BufferD3D10& buffer) : Buffer(buffer) { }

    BufferD3D10& operator = (const BufferD3D10& rhs)
    {
        if (this != &rhs) {
            Buffer::operator=(rhs);
        }
        return *this;
    }
};
#endif

/*! \class BufferGL
 * \brief Memory buffer interface for GL interop.
 */
class BufferGL : public Buffer
{
public:
    BufferGL(
        const Context& context,
        cl_mem_flags flags,
        GLuint bufobj,
        cl_int * err = NULL)
    {
        cl_int error;
        object_ = ::clCreateFromGLBuffer(
            context(),
            flags,
            bufobj,
            &error);

        detail::errHandler(error, __CREATE_GL_BUFFER_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    BufferGL() : Buffer() { }

    BufferGL(const BufferGL& buffer) : Buffer(buffer) { }

    BufferGL& operator = (const BufferGL& rhs)
    {
        if (this != &rhs) {
            Buffer::operator=(rhs);
        }
        return *this;
    }

    cl_int getObjectInfo(
        cl_gl_object_type *type,
        GLuint * gl_object_name)
    {
        return detail::errHandler(
            ::clGetGLObjectInfo(object_,type,gl_object_name),
            __GET_GL_OBJECT_INFO_ERR);
    }
};

/*! \class BufferRenderGL
 * \brief Memory buffer interface for GL interop with renderbuffer.
 */
class BufferRenderGL : public Buffer
{
public:
    BufferRenderGL(
        const Context& context,
        cl_mem_flags flags,
        GLuint bufobj,
        cl_int * err = NULL)
    {
        cl_int error;
        object_ = ::clCreateFromGLRenderbuffer(
            context(),
            flags,
            bufobj,
            &error);

        detail::errHandler(error, __CREATE_GL_BUFFER_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    BufferRenderGL() : Buffer() { }

    BufferRenderGL(const BufferGL& buffer) : Buffer(buffer) { }

    BufferRenderGL& operator = (const BufferRenderGL& rhs)
    {
        if (this != &rhs) {
            Buffer::operator=(rhs);
        }
        return *this;
    }

    cl_int getObjectInfo(
        cl_gl_object_type *type,
        GLuint * gl_object_name)
    {
        return detail::errHandler(
            ::clGetGLObjectInfo(object_,type,gl_object_name),
            __GET_GL_OBJECT_INFO_ERR);
    }
};

/*! \class Image
 * \brief Base class  interface for all images.
 */
class Image : public Memory
{
protected:
    Image() : Memory() { }

    Image(const Image& image) : Memory(image) { }

    Image& operator = (const Image& rhs)
    {
        if (this != &rhs) {
            Memory::operator=(rhs);
        }
        return *this;
    }
public:
    template <typename T>
    cl_int getImageInfo(cl_image_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(&::clGetImageInfo, object_, name, param),
            __GET_IMAGE_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_image_info, name>::param_type
    getImageInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_image_info, name>::param_type param;
        cl_int result = getImageInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }
};

/*! \class Image2D
 * \brief Image interface for 2D images.
 */
class Image2D : public Image
{
public:
    Image2D(
        const Context& context,
        cl_mem_flags flags,
        ImageFormat format,
        ::size_t width,
        ::size_t height,
        ::size_t row_pitch = 0,
        void* host_ptr = NULL,
        cl_int* err = NULL)
    {
        cl_int error;
        object_ = ::clCreateImage2D(
            context(), flags,&format, width, height, row_pitch, host_ptr, &error);

        detail::errHandler(error, __CREATE_IMAGE2D_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    Image2D() { }

    Image2D(const Image2D& image2D) : Image(image2D) { }

    Image2D& operator = (const Image2D& rhs)
    {
        if (this != &rhs) {
            Image::operator=(rhs);
        }
        return *this;
    }
};

/*! \class Image2DGL
 * \brief 2D image interface for GL interop.
 */
class Image2DGL : public Image2D
{
public:
    Image2DGL(
        const Context& context,
        cl_mem_flags flags,
        GLenum target,
        GLint  miplevel,
        GLuint texobj,
        cl_int * err = NULL)
    {
        cl_int error;
        object_ = ::clCreateFromGLTexture2D(
            context(),
            flags,
            target,
            miplevel,
            texobj,
            &error);

        detail::errHandler(error, __CREATE_GL_BUFFER_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    Image2DGL() : Image2D() { }

    Image2DGL(const Image2DGL& image) : Image2D(image) { }

    Image2DGL& operator = (const Image2DGL& rhs)
    {
        if (this != &rhs) {
            Image2D::operator=(rhs);
        }
        return *this;
    }
};

/*! \class Image3D
 * \brief Image interface for 3D images.
 */
class Image3D : public Image
{
public:
    Image3D(
        const Context& context,
        cl_mem_flags flags,
        ImageFormat format,
        ::size_t width,
        ::size_t height,
        ::size_t depth,
        ::size_t row_pitch = 0,
        ::size_t slice_pitch = 0,
        void* host_ptr = NULL,
        cl_int* err = NULL)
    {
        cl_int error;
        object_ = ::clCreateImage3D(
            context(), flags, &format, width, height, depth, row_pitch,
            slice_pitch, host_ptr, &error);

        detail::errHandler(error, __CREATE_IMAGE3D_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    Image3D() { }

    Image3D(const Image3D& image3D) : Image(image3D) { }

    Image3D& operator = (const Image3D& rhs)
    {
        if (this != &rhs) {
            Image::operator=(rhs);
        }
        return *this;
    }
};

/*! \class Image2DGL
 * \brief 2D image interface for GL interop.
 */
class Image3DGL : public Image3D
{
public:
    Image3DGL(
        const Context& context,
        cl_mem_flags flags,
        GLenum target,
        GLint  miplevel,
        GLuint texobj,
        cl_int * err = NULL)
    {
        cl_int error;
        object_ = ::clCreateFromGLTexture3D(
            context(),
            flags,
            target,
            miplevel,
            texobj,
            &error);

        detail::errHandler(error, __CREATE_GL_BUFFER_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    Image3DGL() : Image3D() { }

    Image3DGL(const Image3DGL& image) : Image3D(image) { }

    Image3DGL& operator = (const Image3DGL& rhs)
    {
        if (this != &rhs) {
            Image3D::operator=(rhs);
        }
        return *this;
    }
};

/*! \class Sampler
 * \brief Sampler interface for cl_sampler.
 */
class Sampler : public detail::Wrapper<cl_sampler>
{
public:
    Sampler() { }

    Sampler(
        const Context& context,
        cl_bool normalized_coords,
        cl_addressing_mode addressing_mode,
        cl_filter_mode filter_mode,
        cl_int* err = NULL)
    {
        cl_int error;
        object_ = ::clCreateSampler(
            context(), 
            normalized_coords,
            addressing_mode,
            filter_mode,
            &error);

        detail::errHandler(error, __CREATE_SAMPLER_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    Sampler(const Sampler& sampler) : detail::Wrapper<cl_type>(sampler) { }

    Sampler& operator = (const Sampler& rhs)
    {
        if (this != &rhs) {
            detail::Wrapper<cl_type>::operator=(rhs);
        }
        return *this;
    }

    template <typename T>
    cl_int getInfo(cl_sampler_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(&::clGetSamplerInfo, object_, name, param),
            __GET_SAMPLER_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_sampler_info, name>::param_type
    getInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_sampler_info, name>::param_type param;
        cl_int result = getInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }
};

__GET_INFO_HELPER_WITH_RETAIN(cl::Sampler)

class Program;
class CommandQueue;
class Kernel;

/*! \class NDRange
 * \brief NDRange interface
 */
class NDRange
{
private:
    size_t<3> sizes_;
    cl_uint dimensions_;

public:
    NDRange()
        : dimensions_(0)
    { }

    NDRange(::size_t size0)
        : dimensions_(1)
    {
        sizes_.push_back(size0);
    }

    NDRange(::size_t size0, ::size_t size1)
        : dimensions_(2)
    {
        sizes_.push_back(size0);
        sizes_.push_back(size1);
    }

    NDRange(::size_t size0, ::size_t size1, ::size_t size2)
        : dimensions_(3)
    {
        sizes_.push_back(size0);
        sizes_.push_back(size1);
        sizes_.push_back(size2);
    }

    operator const ::size_t*() const { return (const ::size_t*) sizes_; }
    ::size_t dimensions() const { return dimensions_; }
};

static const NDRange NullRange;

/*!
 * \struct LocalSpaceArg
 * \brief Local address raper for use with Kernel::setArg
 */
struct LocalSpaceArg
{
    ::size_t size_;
};

namespace detail {

template <typename T>
struct KernelArgumentHandler
{
    static ::size_t size(const T&) { return sizeof(T); }
    static T* ptr(T& value) { return &value; }
};

template <>
struct KernelArgumentHandler<LocalSpaceArg>
{
    static ::size_t size(const LocalSpaceArg& value) { return value.size_; }
    static void* ptr(LocalSpaceArg&) { return NULL; }
};

} 
//! \endcond

inline LocalSpaceArg
__local(::size_t size)
{
    LocalSpaceArg ret = { size };
    return ret;
}

class KernelFunctor;

/*! \class Kernel
 * \brief Kernel interface that implements cl_kernel
 */
class Kernel : public detail::Wrapper<cl_kernel>
{
public:
    inline Kernel(const Program& program, const char* name, cl_int* err = NULL);

    Kernel() { }

    Kernel(const Kernel& kernel) : detail::Wrapper<cl_type>(kernel) { }

    Kernel& operator = (const Kernel& rhs)
    {
        if (this != &rhs) {
            detail::Wrapper<cl_type>::operator=(rhs);
        }
        return *this;
    }

    template <typename T>
    cl_int getInfo(cl_kernel_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(&::clGetKernelInfo, object_, name, param),
            __GET_KERNEL_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_kernel_info, name>::param_type
    getInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_kernel_info, name>::param_type param;
        cl_int result = getInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

    template <typename T>
    cl_int getWorkGroupInfo(
        const Device& device, cl_kernel_work_group_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(
                &::clGetKernelWorkGroupInfo, object_, device(), name, param),
                __GET_KERNEL_WORK_GROUP_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_kernel_work_group_info, name>::param_type
        getWorkGroupInfo(const Device& device, cl_int* err = NULL) const
    {
        typename detail::param_traits<
        detail::cl_kernel_work_group_info, name>::param_type param;
        cl_int result = getWorkGroupInfo(device, name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

    template <typename T>
    cl_int setArg(cl_uint index, T value)
    {
        return detail::errHandler(
            ::clSetKernelArg(
                object_,
                index,
                detail::KernelArgumentHandler<T>::size(value),
                detail::KernelArgumentHandler<T>::ptr(value)),
            __SET_KERNEL_ARGS_ERR);
    }

    cl_int setArg(cl_uint index, ::size_t size, void* argPtr)
    {
        return detail::errHandler(
            ::clSetKernelArg(object_, index, size, argPtr),
            __SET_KERNEL_ARGS_ERR);
    }

    KernelFunctor bind(
        const CommandQueue& queue,
        const NDRange& offset,
        const NDRange& global,
        const NDRange& local);

    KernelFunctor bind(
        const CommandQueue& queue,
        const NDRange& global,
        const NDRange& local);
};

__GET_INFO_HELPER_WITH_RETAIN(cl::Kernel)

/*! \class Program
 * \brief Program interface that implements cl_program.
 */
class Program : public detail::Wrapper<cl_program>
{
public:
    typedef VECTOR_CLASS<std::pair<const void*, ::size_t> > Binaries;
    typedef VECTOR_CLASS<std::pair<const char*, ::size_t> > Sources;

    Program(
        const Context& context,
        const Sources& sources,
        cl_int* err = NULL)
    {
        cl_int error;

        const ::size_t n = (::size_t)sources.size();
        ::size_t* lengths = (::size_t*) alloca(n * sizeof(::size_t));
        const char** strings = (const char**) alloca(n * sizeof(const char*));

        for (::size_t i = 0; i < n; ++i) {
            strings[i] = sources[(int)i].first;
            lengths[i] = sources[(int)i].second;
        }

        object_ = ::clCreateProgramWithSource(
            context(), (cl_uint)n, strings, lengths, &error);

        detail::errHandler(error, __CREATE_PROGRAM_WITH_SOURCE_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    Program(
        const Context& context,
        const VECTOR_CLASS<Device>& devices,
        const Binaries& binaries,
        VECTOR_CLASS<cl_int>* binaryStatus = NULL,
        cl_int* err = NULL)
    {
        cl_int error;
        const ::size_t n = binaries.size();
        ::size_t* lengths = (::size_t*) alloca(n * sizeof(::size_t));
        const unsigned char** images = (const unsigned char**) alloca(n * sizeof(const void*));

        for (::size_t i = 0; i < n; ++i) {
            images[i] = (const unsigned char*)binaries[(int)i].first;
            lengths[i] = binaries[(int)i].second;
        }

        object_ = ::clCreateProgramWithBinary(
            context(), (cl_uint) devices.size(),
            (cl_device_id*)&devices.front(),
            lengths, images, binaryStatus != NULL
               ? (cl_int*) &binaryStatus->front()
               : NULL, &error);

        detail::errHandler(error, __CREATE_PROGRAM_WITH_BINARY_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    Program() { }

    Program(const Program& program) : detail::Wrapper<cl_type>(program) { }

    Program& operator = (const Program& rhs)
    {
        if (this != &rhs) {
            detail::Wrapper<cl_type>::operator=(rhs);
        }
        return *this;
    }

    cl_int build(
        const VECTOR_CLASS<Device>& devices,
        const char* options = NULL,
        void (CL_CALLBACK * notifyFptr)(cl_program, void *) = NULL,
        void* data = NULL) const
    {
        return detail::errHandler(
            ::clBuildProgram(
                object_,
                (cl_uint)
                devices.size(),
                (cl_device_id*)&devices.front(),
                options,
                notifyFptr,
                data),
                __BUILD_PROGRAM_ERR);
    }

    template <typename T>
    cl_int getInfo(cl_program_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(&::clGetProgramInfo, object_, name, param),
            __GET_PROGRAM_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_program_info, name>::param_type
    getInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_program_info, name>::param_type param;
        cl_int result = getInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

    template <typename T>
    cl_int getBuildInfo(
        const Device& device, cl_program_build_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(
                &::clGetProgramBuildInfo, object_, device(), name, param),
                __GET_PROGRAM_BUILD_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_program_build_info, name>::param_type
    getBuildInfo(const Device& device, cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_program_build_info, name>::param_type param;
        cl_int result = getBuildInfo(device, name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

    cl_int createKernels(VECTOR_CLASS<Kernel>* kernels)
    {
        cl_uint numKernels;
        cl_int err = ::clCreateKernelsInProgram(object_, 0, NULL, &numKernels);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __CREATE_KERNELS_IN_PROGRAM_ERR);
        }

        Kernel* value = (Kernel*) alloca(numKernels * sizeof(Kernel));
        err = ::clCreateKernelsInProgram(
            object_, numKernels, (cl_kernel*) value, NULL);
        if (err != CL_SUCCESS) {
            return detail::errHandler(err, __CREATE_KERNELS_IN_PROGRAM_ERR);
        }

        kernels->assign(&value[0], &value[numKernels]);
        return CL_SUCCESS;
    }
};

__GET_INFO_HELPER_WITH_RETAIN(cl::Program)

inline Kernel::Kernel(const Program& program, const char* name, cl_int* err)
{
    cl_int error;

    object_ = ::clCreateKernel(program(), name, &error);
    detail::errHandler(error, __CREATE_KERNEL_ERR);

    if (err != NULL) {
        *err = error;
    }

}

/*! \class CommandQueue
 * \brief CommandQueue interface for cl_command_queue.
 */
class CommandQueue : public detail::Wrapper<cl_command_queue>
{
public:
    CommandQueue(
        const Context& context,
        const Device& device,
        cl_command_queue_properties properties = 0,
        cl_int* err = NULL)
    {
        cl_int error;
        object_ = ::clCreateCommandQueue(
            context(), device(), properties, &error);

        detail::errHandler(error, __CREATE_COMMAND_QUEUE_ERR);
        if (err != NULL) {
            *err = error;
        }
    }

    CommandQueue() { }

    CommandQueue(const CommandQueue& commandQueue) : detail::Wrapper<cl_type>(commandQueue) { }

    CommandQueue& operator = (const CommandQueue& rhs)
    {
        if (this != &rhs) {
            detail::Wrapper<cl_type>::operator=(rhs);
        }
        return *this;
    }

    template <typename T>
    cl_int getInfo(cl_command_queue_info name, T* param) const
    {
        return detail::errHandler(
            detail::getInfo(
                &::clGetCommandQueueInfo, object_, name, param),
                __GET_COMMAND_QUEUE_INFO_ERR);
    }

    template <cl_int name> typename
    detail::param_traits<detail::cl_command_queue_info, name>::param_type
    getInfo(cl_int* err = NULL) const
    {
        typename detail::param_traits<
            detail::cl_command_queue_info, name>::param_type param;
        cl_int result = getInfo(name, &param);
        if (err != NULL) {
            *err = result;
        }
        return param;
    }

    cl_int enqueueReadBuffer(
        const Buffer& buffer,
        cl_bool blocking,
        ::size_t offset,
        ::size_t size,
        void* ptr,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueReadBuffer(
                object_, buffer(), blocking, offset, size,
                ptr,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_READ_BUFFER_ERR);
    }

    cl_int enqueueWriteBuffer(
        const Buffer& buffer,
        cl_bool blocking,
        ::size_t offset,
        ::size_t size,
        const void* ptr,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueWriteBuffer(
                object_, buffer(), blocking, offset, size,
                ptr,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
                __ENQUEUE_WRITE_BUFFER_ERR);
    }

    cl_int enqueueCopyBuffer(
        const Buffer& src,
        const Buffer& dst,
        ::size_t src_offset,
        ::size_t dst_offset,
        ::size_t size,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueCopyBuffer(
                object_, src(), dst(), src_offset, dst_offset, size,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQEUE_COPY_BUFFER_ERR);
    }

#if defined(CL_VERSION_1_1)
    cl_int enqueueReadBufferRect(
        const Buffer& buffer,
        cl_bool blocking,
        const size_t<3>& buffer_offset,
        const size_t<3>& host_offset,
        const size_t<3>& region,
        ::size_t buffer_row_pitch,
        ::size_t buffer_slice_pitch,
        ::size_t host_row_pitch,
        ::size_t host_slice_pitch,
        void *ptr,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueReadBufferRect(
                object_, 
                buffer(), 
                blocking, 
                (const ::size_t *)buffer_offset,
                (const ::size_t *)host_offset,
                (const ::size_t *)region,
                buffer_row_pitch,
                buffer_slice_pitch,
                host_row_pitch,
                host_slice_pitch,
                ptr,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
                __ENQUEUE_READ_BUFFER_RECT_ERR);
    }


    cl_int enqueueWriteBufferRect(
        const Buffer& buffer,
        cl_bool blocking,
        const size_t<3>& buffer_offset,
        const size_t<3>& host_offset,
        const size_t<3>& region,
        ::size_t buffer_row_pitch,
        ::size_t buffer_slice_pitch,
        ::size_t host_row_pitch,
        ::size_t host_slice_pitch,
        void *ptr,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueWriteBufferRect(
                object_, 
                buffer(), 
                blocking, 
                (const ::size_t *)buffer_offset,
                (const ::size_t *)host_offset,
                (const ::size_t *)region,
                buffer_row_pitch,
                buffer_slice_pitch,
                host_row_pitch,
                host_slice_pitch,
                ptr,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
                __ENQUEUE_WRITE_BUFFER_RECT_ERR);
    }

    cl_int enqueueCopyBufferRect(
        const Buffer& src,
        const Buffer& dst,
        const size_t<3>& src_origin,
        const size_t<3>& dst_origin,
        const size_t<3>& region,
        ::size_t src_row_pitch,
        ::size_t src_slice_pitch,
        ::size_t dst_row_pitch,
        ::size_t dst_slice_pitch,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueCopyBufferRect(
                object_, 
                src(), 
                dst(), 
                (const ::size_t *)src_origin, 
                (const ::size_t *)dst_origin, 
                (const ::size_t *)region,
                src_row_pitch,
                src_slice_pitch,
                dst_row_pitch,
                dst_slice_pitch,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQEUE_COPY_BUFFER_RECT_ERR);
    }
#endif

    cl_int enqueueReadImage(
        const Image& image,
        cl_bool blocking,
        const size_t<3>& origin,
        const size_t<3>& region,
        ::size_t row_pitch,
        ::size_t slice_pitch,
        void* ptr,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueReadImage(
                object_, image(), blocking, (const ::size_t *) origin,
                (const ::size_t *) region, row_pitch, slice_pitch, ptr,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_READ_IMAGE_ERR);
    }

    cl_int enqueueWriteImage(
        const Image& image,
        cl_bool blocking,
        const size_t<3>& origin,
        const size_t<3>& region,
        ::size_t row_pitch,
        ::size_t slice_pitch,
        void* ptr,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueWriteImage(
                object_, image(), blocking, (const ::size_t *) origin,
                (const ::size_t *) region, row_pitch, slice_pitch, ptr,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_WRITE_IMAGE_ERR);
    }

    cl_int enqueueCopyImage(
        const Image& src,
        const Image& dst,
        const size_t<3>& src_origin,
        const size_t<3>& dst_origin,
        const size_t<3>& region,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueCopyImage(
                object_, src(), dst(), (const ::size_t *) src_origin,
                (const ::size_t *)dst_origin, (const ::size_t *) region,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_COPY_IMAGE_ERR);
    }

    cl_int enqueueCopyImageToBuffer(
        const Image& src,
        const Buffer& dst,
        const size_t<3>& src_origin,
        const size_t<3>& region,
        ::size_t dst_offset,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueCopyImageToBuffer(
                object_, src(), dst(), (const ::size_t *) src_origin,
                (const ::size_t *) region, dst_offset,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_COPY_IMAGE_TO_BUFFER_ERR);
    }

    cl_int enqueueCopyBufferToImage(
        const Buffer& src,
        const Image& dst,
        ::size_t src_offset,
        const size_t<3>& dst_origin,
        const size_t<3>& region,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueCopyBufferToImage(
                object_, src(), dst(), src_offset,
                (const ::size_t *) dst_origin, (const ::size_t *) region,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_COPY_BUFFER_TO_IMAGE_ERR);
    }

    void* enqueueMapBuffer(
        const Buffer& buffer,
        cl_bool blocking,
        cl_map_flags flags,
        ::size_t offset,
        ::size_t size,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL,
        cl_int* err = NULL) const
    {
        cl_int error;
        void * result = ::clEnqueueMapBuffer(
            object_, buffer(), blocking, flags, offset, size,
            (events != NULL) ? (cl_uint) events->size() : 0,
            (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
            (cl_event*) event,
            &error);

        detail::errHandler(error, __ENQUEUE_MAP_BUFFER_ERR);
        if (err != NULL) {
            *err = error;
        }
        return result;
    }

    void* enqueueMapImage(
        const Image& buffer,
        cl_bool blocking,
        cl_map_flags flags,
        const size_t<3>& origin,
        const size_t<3>& region,
        ::size_t * row_pitch,
        ::size_t * slice_pitch,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL,
        cl_int* err = NULL) const
    {
        cl_int error;
        void * result = ::clEnqueueMapImage(
            object_, buffer(), blocking, flags,
            (const ::size_t *) origin, (const ::size_t *) region,
            row_pitch, slice_pitch,
            (events != NULL) ? (cl_uint) events->size() : 0,
            (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
            (cl_event*) event,
            &error);

        detail::errHandler(error, __ENQUEUE_MAP_IMAGE_ERR);
        if (err != NULL) {
              *err = error;
        }
        return result;
    }

    cl_int enqueueUnmapMemObject(
        const Memory& memory,
        void* mapped_ptr,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueUnmapMemObject(
                object_, memory(), mapped_ptr,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_UNMAP_MEM_OBJECT_ERR);
    }

    cl_int enqueueNDRangeKernel(
        const Kernel& kernel,
        const NDRange& offset,
        const NDRange& global,
        const NDRange& local,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueNDRangeKernel(
                object_, kernel(), (cl_uint) global.dimensions(),
                offset.dimensions() != 0 ? (const ::size_t*) offset : NULL,
                (const ::size_t*) global,
                local.dimensions() != 0 ? (const ::size_t*) local : NULL,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_NDRANGE_KERNEL_ERR);
    }

    cl_int enqueueTask(
        const Kernel& kernel,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueTask(
                object_, kernel(),
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_TASK_ERR);
    }

    cl_int enqueueNativeKernel(
        void (*userFptr)(void *),
        std::pair<void*, ::size_t> args,
        const VECTOR_CLASS<Memory>* mem_objects = NULL,
        const VECTOR_CLASS<const void*>* mem_locs = NULL,
        const VECTOR_CLASS<Event>* events = NULL,
        Event* event = NULL) const
    {
        cl_mem * mems = (mem_objects != NULL && mem_objects->size() > 0) 
            ? (cl_mem*) alloca(mem_objects->size() * sizeof(cl_mem))
            : NULL;

        if (mems != NULL) {
            for (unsigned int i = 0; i < mem_objects->size(); i++) {
                mems[i] = ((*mem_objects)[i])();
            }
        }

        return detail::errHandler(
            ::clEnqueueNativeKernel(
                object_, userFptr, args.first, args.second,
                (mem_objects != NULL) ? (cl_uint) mem_objects->size() : 0,
                mems,
                (mem_locs != NULL) ? (const void **) &mem_locs->front() : NULL,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_NATIVE_KERNEL);
    }

    cl_int enqueueMarker(Event* event = NULL) const
    {
        return detail::errHandler(
            ::clEnqueueMarker(object_, (cl_event*) event),
            __ENQUEUE_MARKER_ERR);
    }

    cl_int enqueueWaitForEvents(const VECTOR_CLASS<Event>& events) const
    {
        return detail::errHandler(
            ::clEnqueueWaitForEvents(
                object_,
                (cl_uint) events.size(),
                (const cl_event*) &events.front()),
            __ENQUEUE_WAIT_FOR_EVENTS_ERR);
    }

    cl_int enqueueAcquireGLObjects(
         const VECTOR_CLASS<Memory>* mem_objects = NULL,
         const VECTOR_CLASS<Event>* events = NULL,
         Event* event = NULL) const
     {
         return detail::errHandler(
             ::clEnqueueAcquireGLObjects(
                 object_,
                 (mem_objects != NULL) ? (cl_uint) mem_objects->size() : 0,
                 (mem_objects != NULL) ? (const cl_mem *) &mem_objects->front(): NULL,
                 (events != NULL) ? (cl_uint) events->size() : 0,
                 (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                 (cl_event*) event),
             __ENQUEUE_ACQUIRE_GL_ERR);
     }

    cl_int enqueueReleaseGLObjects(
         const VECTOR_CLASS<Memory>* mem_objects = NULL,
         const VECTOR_CLASS<Event>* events = NULL,
         Event* event = NULL) const
     {
         return detail::errHandler(
             ::clEnqueueReleaseGLObjects(
                 object_,
                 (mem_objects != NULL) ? (cl_uint) mem_objects->size() : 0,
                 (mem_objects != NULL) ? (const cl_mem *) &mem_objects->front(): NULL,
                 (events != NULL) ? (cl_uint) events->size() : 0,
                 (events != NULL && events->size() > 0) ? (cl_event*) &events->front() : NULL,
                 (cl_event*) event),
             __ENQUEUE_RELEASE_GL_ERR);
     }

#if defined (USE_DX_INTEROP)
typedef CL_API_ENTRY cl_int (CL_API_CALL *PFN_clEnqueueAcquireD3D10ObjectsKHR)(
    cl_command_queue command_queue, cl_uint num_objects,
    const cl_mem* mem_objects, cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list, cl_event* event);
typedef CL_API_ENTRY cl_int (CL_API_CALL *PFN_clEnqueueReleaseD3D10ObjectsKHR)(
    cl_command_queue command_queue, cl_uint num_objects,
    const cl_mem* mem_objects,  cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list, cl_event* event);

    cl_int enqueueAcquireD3D10Objects(
         const VECTOR_CLASS<Memory>* mem_objects = NULL,
         const VECTOR_CLASS<Event>* events = NULL,
         Event* event = NULL) const
     {
         static PFN_clEnqueueAcquireD3D10ObjectsKHR pfn_clEnqueueAcquireD3D10ObjectsKHR = NULL;
         __INIT_CL_EXT_FCN_PTR(clEnqueueAcquireD3D10ObjectsKHR);
		
         return detail::errHandler(
             pfn_clEnqueueAcquireD3D10ObjectsKHR(
                 object_,
                 (mem_objects != NULL) ? (cl_uint) mem_objects->size() : 0,
                 (mem_objects != NULL) ? (const cl_mem *) &mem_objects->front(): NULL,
                 (events != NULL) ? (cl_uint) events->size() : 0,
                 (events != NULL) ? (cl_event*) &events->front() : NULL,
                 (cl_event*) event),
             __ENQUEUE_ACQUIRE_GL_ERR);
     }

    cl_int enqueueReleaseD3D10Objects(
         const VECTOR_CLASS<Memory>* mem_objects = NULL,
         const VECTOR_CLASS<Event>* events = NULL,
         Event* event = NULL) const
    {
        static PFN_clEnqueueReleaseD3D10ObjectsKHR pfn_clEnqueueReleaseD3D10ObjectsKHR = NULL;
        __INIT_CL_EXT_FCN_PTR(clEnqueueReleaseD3D10ObjectsKHR);

        return detail::errHandler(
            pfn_clEnqueueReleaseD3D10ObjectsKHR(
                object_,
                (mem_objects != NULL) ? (cl_uint) mem_objects->size() : 0,
                (mem_objects != NULL) ? (const cl_mem *) &mem_objects->front(): NULL,
                (events != NULL) ? (cl_uint) events->size() : 0,
                (events != NULL) ? (cl_event*) &events->front() : NULL,
                (cl_event*) event),
            __ENQUEUE_RELEASE_GL_ERR);
    }
#endif

    cl_int enqueueBarrier() const
    {
        return detail::errHandler(
            ::clEnqueueBarrier(object_),
            __ENQUEUE_BARRIER_ERR);
    }

    cl_int flush() const
    {
        return detail::errHandler(::clFlush(object_), __FLUSH_ERR);
    }

    cl_int finish() const
    {
        return detail::errHandler(::clFinish(object_), __FINISH_ERR);
    }
};

__GET_INFO_HELPER_WITH_RETAIN(cl::CommandQueue)

/*! \class KernelFunctor
 * \brief Kernel functor interface
 *
 * \note Currently only functors of zero to ten arguments are supported. It
 * is straightforward to add more and a more general solution, similar to
 * Boost.Lambda could be followed if required in the future.
 */
class KernelFunctor
{
private:
    Kernel kernel_;
    CommandQueue queue_;
    NDRange offset_;
    NDRange global_;
    NDRange local_;

    cl_int err_;
public:
    KernelFunctor() { }

    KernelFunctor(
        const Kernel& kernel,
        const CommandQueue& queue,
        const NDRange& offset,
        const NDRange& global,
        const NDRange& local) :
            kernel_(kernel),
            queue_(queue),
            offset_(offset),
            global_(global),
            local_(local),
            err_(CL_SUCCESS)
    {}

    KernelFunctor& operator=(const KernelFunctor& rhs);

    KernelFunctor(const KernelFunctor& rhs);

    cl_int getError() { return err_; }

    inline Event operator()(const VECTOR_CLASS<Event>* events = NULL);

    template<typename A1>
    inline Event operator()(
        const A1& a1, 
        const VECTOR_CLASS<Event>* events = NULL);

    template<class A1, class A2>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const VECTOR_CLASS<Event>* events = NULL);

    template<class A1, class A2, class A3>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3,
        const VECTOR_CLASS<Event>* events = NULL);

    template<class A1, class A2, class A3, class A4>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4,
        const VECTOR_CLASS<Event>* events = NULL);

    template<class A1, class A2, class A3, class A4, class A5>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5,
        const VECTOR_CLASS<Event>* events = NULL);

    template<class A1, class A2, class A3, class A4, class A5, class A6>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5, 
        const A6& a6,
        const VECTOR_CLASS<Event>* events = NULL);

    template<class A1, class A2, class A3, class A4,
             class A5, class A6, class A7>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5, 
        const A6& a6, 
        const A7& a7,
        const VECTOR_CLASS<Event>* events = NULL);

    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5, 
        const A6& a6, 
        const A7& a7, 
        const A8& a8,
        const VECTOR_CLASS<Event>* events = NULL);

    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8, class A9>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5, 
        const A6& a6, 
        const A7& a7, 
        const A8& a8, 
        const A9& a9,
        const VECTOR_CLASS<Event>* events = NULL);
    
    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8, class A9, class A10>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5, 
        const A6& a6,
        const A7& a7, 
        const A8& a8, 
        const A9& a9, 
        const A10& a10,
        const VECTOR_CLASS<Event>* events = NULL);
    
    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8, class A9, class A10,
             class A11>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5, 
        const A6& a6,
        const A7& a7, 
        const A8& a8, 
        const A9& a9, 
        const A10& a10, 
        const A11& a11,
        const VECTOR_CLASS<Event>* events = NULL);
    
    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8, class A9, class A10,
             class A11, class A12>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5, 
        const A6& a6,
        const A7& a7, 
        const A8& a8, 
        const A9& a9, 
        const A10& a10, 
        const A11& a11, 
        const A12& a12,
        const VECTOR_CLASS<Event>* events = NULL);
    
    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8, class A9, class A10,
             class A11, class A12, class A13>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5, 
        const A6& a6,
        const A7& a7, 
        const A8& a8, 
        const A9& a9, 
        const A10& a10, 
        const A11& a11, 
        const A12& a12, 
        const A13& a13,
        const VECTOR_CLASS<Event>* events = NULL);
    
    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8, class A9, class A10,
             class A11, class A12, class A13, class A14>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5, 
        const A6& a6,
        const A7& a7, 
        const A8& a8, 
        const A9& a9, 
        const A10& a10, 
        const A11& a11,
        const A12& a12, 
        const A13& a13, 
        const A14& a14,
        const VECTOR_CLASS<Event>* events = NULL);
    
    template<class A1, class A2, class A3, class A4, class A5,
             class A6, class A7, class A8, class A9, class A10,
             class A11, class A12, class A13, class A14, class A15>
    inline Event operator()(
        const A1& a1, 
        const A2& a2, 
        const A3& a3, 
        const A4& a4, 
        const A5& a5, 
        const A6& a6,
        const A7& a7, 
        const A8& a8, 
        const A9& a9, 
        const A10& a10, 
        const A11& a11,
        const A12& a12, 
        const A13& a13, 
        const A14& a14, 
        const A15& a15,
        const VECTOR_CLASS<Event>* events = NULL);
};

inline KernelFunctor Kernel::bind(
    const CommandQueue& queue,
    const NDRange& offset,
    const NDRange& global,
    const NDRange& local)
{
    return KernelFunctor(*this,queue,offset,global,local);
}

inline KernelFunctor Kernel::bind(
    const CommandQueue& queue,
    const NDRange& global,
    const NDRange& local)
{
    return KernelFunctor(*this,queue,NullRange,global,local);
}

inline KernelFunctor& KernelFunctor::operator=(const KernelFunctor& rhs)
{
    if (this == &rhs) {
        return *this;
    }
    
    kernel_ = rhs.kernel_;
    queue_  = rhs.queue_;
    offset_ = rhs.offset_;
    global_ = rhs.global_;
    local_  = rhs.local_;
    
    return *this;
}

inline KernelFunctor::KernelFunctor(const KernelFunctor& rhs) :
    kernel_(rhs.kernel_),
    queue_(rhs.queue_),
    offset_(rhs.offset_),
    global_(rhs.global_),
    local_(rhs.local_)
{
}

Event KernelFunctor::operator()(const VECTOR_CLASS<Event>* events)
{
    Event event;

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<typename A1>
Event KernelFunctor::operator()(
    const A1& a1, 
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<typename A1, typename A2>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<typename A1, typename A2, typename A3>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<typename A1, typename A2, typename A3, typename A4>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<typename A1, typename A2, typename A3, typename A4, typename A5>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<typename A1, typename A2, typename A3, typename A4, typename A5,
         typename A6>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5, 
    const A6& a6,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);
    kernel_.setArg(5,a6);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<typename A1, typename A2, typename A3, typename A4,
         typename A5, typename A6, typename A7>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5, 
    const A6& a6, 
    const A7& a7,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);
    kernel_.setArg(5,a6);
    kernel_.setArg(6,a7);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<typename A1, typename A2, typename A3, typename A4, typename A5,
         typename A6, typename A7, typename A8>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5, 
    const A6& a6, 
    const A7& a7, 
    const A8& a8,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);
    kernel_.setArg(5,a6);
    kernel_.setArg(6,a7);
    kernel_.setArg(7,a8);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<typename A1, typename A2, typename A3, typename A4, typename A5,
         typename A6, typename A7, typename A8, typename A9>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5,
    const A6& a6, 
    const A7& a7, 
    const A8& a8, 
    const A9& a9,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);
    kernel_.setArg(5,a6);
    kernel_.setArg(6,a7);
    kernel_.setArg(7,a8);
    kernel_.setArg(8,a9);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<typename A1, typename A2, typename A3, typename A4, typename A5,
         typename A6, typename A7, typename A8, typename A9, typename A10>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5, 
    const A6& a6,
    const A7& a7, 
    const A8& a8, 
    const A9& a9, 
    const A10& a10,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);
    kernel_.setArg(5,a6);
    kernel_.setArg(6,a7);
    kernel_.setArg(7,a8);
    kernel_.setArg(8,a9);
    kernel_.setArg(9,a10);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<class A1, class A2, class A3, class A4, class A5,
         class A6, class A7, class A8, class A9, class A10,
         class A11>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5, 
    const A6& a6,
    const A7& a7, 
    const A8& a8, 
    const A9& a9, 
    const A10& a10, 
    const A11& a11,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);
    kernel_.setArg(5,a6);
    kernel_.setArg(6,a7);
    kernel_.setArg(7,a8);
    kernel_.setArg(8,a9);
    kernel_.setArg(9,a10);
    kernel_.setArg(10,a11);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<class A1, class A2, class A3, class A4, class A5,
         class A6, class A7, class A8, class A9, class A10,
         class A11, class A12>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5, 
    const A6& a6,
    const A7& a7, 
    const A8& a8, 
    const A9& a9, 
    const A10& a10, 
    const A11& a11, 
    const A12& a12,
    const VECTOR_CLASS<Event>* events)
{
    Event event;

    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);
    kernel_.setArg(5,a6);
    kernel_.setArg(6,a7);
    kernel_.setArg(7,a8);
    kernel_.setArg(8,a9);
    kernel_.setArg(9,a10);
    kernel_.setArg(10,a11);
    kernel_.setArg(11,a12);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<class A1, class A2, class A3, class A4, class A5,
         class A6, class A7, class A8, class A9, class A10,
         class A11, class A12, class A13>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5, 
    const A6& a6,
    const A7& a7, 
    const A8& a8, 
    const A9& a9, 
    const A10& a10, 
    const A11& a11, 
    const A12& a12, 
    const A13& a13,
    const VECTOR_CLASS<Event>* events)
{
    Event event;
    
    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);
    kernel_.setArg(5,a6);
    kernel_.setArg(6,a7);
    kernel_.setArg(7,a8);
    kernel_.setArg(8,a9);
    kernel_.setArg(9,a10);
    kernel_.setArg(10,a11);
    kernel_.setArg(11,a12);
    kernel_.setArg(12,a13);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<class A1, class A2, class A3, class A4, class A5,
         class A6, class A7, class A8, class A9, class A10,
         class A11, class A12, class A13, class A14>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5, 
    const A6& a6,
    const A7& a7, 
    const A8& a8, 
    const A9& a9, 
    const A10& a10, 
    const A11& a11,
    const A12& a12, 
    const A13& a13, 
    const A14& a14,
    const VECTOR_CLASS<Event>* events)
{
    Event event;
    
    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);
    kernel_.setArg(5,a6);
    kernel_.setArg(6,a7);
    kernel_.setArg(7,a8);
    kernel_.setArg(8,a9);
    kernel_.setArg(9,a10);
    kernel_.setArg(10,a11);
    kernel_.setArg(11,a12);
    kernel_.setArg(12,a13);
    kernel_.setArg(13,a14);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

template<class A1, class A2, class A3, class A4, class A5,
         class A6, class A7, class A8, class A9, class A10,
         class A11, class A12, class A13, class A14, class A15>
Event KernelFunctor::operator()(
    const A1& a1, 
    const A2& a2, 
    const A3& a3, 
    const A4& a4, 
    const A5& a5,
    const A6& a6, 
    const A7& a7, 
    const A8& a8, 
    const A9& a9, 
    const A10& a10, 
    const A11& a11,
    const A12& a12, 
    const A13& a13, 
    const A14& a14, 
    const A15& a15,
    const VECTOR_CLASS<Event>* events)
{
    Event event;
    
    kernel_.setArg(0,a1);
    kernel_.setArg(1,a2);
    kernel_.setArg(2,a3);
    kernel_.setArg(3,a4);
    kernel_.setArg(4,a5);
    kernel_.setArg(5,a6);
    kernel_.setArg(6,a7);
    kernel_.setArg(7,a8);
    kernel_.setArg(8,a9);
    kernel_.setArg(9,a10);
    kernel_.setArg(10,a11);
    kernel_.setArg(11,a12);
    kernel_.setArg(12,a13);
    kernel_.setArg(13,a14);
    kernel_.setArg(14,a15);

    err_ = queue_.enqueueNDRangeKernel(
        kernel_,
        offset_,
        global_,
        local_,
        NULL,    // bgaster_fixme - do we want to allow wait event lists?
        &event);

    return event;
}

#undef __ERR_STR
#if !defined(__CL_USER_OVERRIDE_ERROR_STRINGS)
#undef __GET_DEVICE_INFO_ERR
#undef __GET_PLATFORM_INFO_ERR
#undef __GET_DEVICE_IDS_ERR
#undef __GET_CONTEXT_INFO_ERR
#undef __GET_EVENT_INFO_ERR
#undef __GET_EVENT_PROFILE_INFO_ERR
#undef __GET_MEM_OBJECT_INFO_ERR
#undef __GET_IMAGE_INFO_ERR
#undef __GET_SAMPLER_INFO_ERR
#undef __GET_KERNEL_INFO_ERR
#undef __GET_KERNEL_WORK_GROUP_INFO_ERR
#undef __GET_PROGRAM_INFO_ERR
#undef __GET_PROGRAM_BUILD_INFO_ERR
#undef __GET_COMMAND_QUEUE_INFO_ERR

#undef __CREATE_CONTEXT_FROM_TYPE_ERR
#undef __GET_SUPPORTED_IMAGE_FORMATS_ERR

#undef __CREATE_BUFFER_ERR
#undef __CREATE_SUBBUFFER_ERR
#undef __CREATE_IMAGE2D_ERR
#undef __CREATE_IMAGE3D_ERR
#undef __CREATE_SAMPLER_ERR
#undef __SET_MEM_OBJECT_DESTRUCTOR_CALLBACK_ERR

#undef __CREATE_USER_EVENT_ERR
#undef __SET_USER_EVENT_STATUS_ERR
#undef __SET_EVENT_CALLBACK_ERR

#undef __WAIT_FOR_EVENTS_ERR

#undef __CREATE_KERNEL_ERR
#undef __SET_KERNEL_ARGS_ERR
#undef __CREATE_PROGRAM_WITH_SOURCE_ERR
#undef __CREATE_PROGRAM_WITH_BINARY_ERR
#undef __BUILD_PROGRAM_ERR
#undef __CREATE_KERNELS_IN_PROGRAM_ERR

#undef __CREATE_COMMAND_QUEUE_ERR
#undef __SET_COMMAND_QUEUE_PROPERTY_ERR
#undef __ENQUEUE_READ_BUFFER_ERR
#undef __ENQUEUE_WRITE_BUFFER_ERR
#undef __ENQUEUE_READ_BUFFER_RECT_ERR
#undef __ENQUEUE_WRITE_BUFFER_RECT_ERR
#undef __ENQEUE_COPY_BUFFER_ERR
#undef __ENQEUE_COPY_BUFFER_RECT_ERR
#undef __ENQUEUE_READ_IMAGE_ERR
#undef __ENQUEUE_WRITE_IMAGE_ERR
#undef __ENQUEUE_COPY_IMAGE_ERR
#undef __ENQUEUE_COPY_IMAGE_TO_BUFFER_ERR
#undef __ENQUEUE_COPY_BUFFER_TO_IMAGE_ERR
#undef __ENQUEUE_MAP_BUFFER_ERR
#undef __ENQUEUE_MAP_IMAGE_ERR
#undef __ENQUEUE_UNMAP_MEM_OBJECT_ERR
#undef __ENQUEUE_NDRANGE_KERNEL_ERR
#undef __ENQUEUE_TASK_ERR
#undef __ENQUEUE_NATIVE_KERNEL

#undef __UNLOAD_COMPILER_ERR
#endif //__CL_USER_OVERRIDE_ERROR_STRINGS

#undef __GET_INFO_HELPER_WITH_RETAIN

// Extensions
#undef __INIT_CL_EXT_FCN_PTR
#undef __CREATE_SUB_DEVICES

#if defined(USE_CL_DEVICE_FISSION)
#undef __PARAM_NAME_DEVICE_FISSION
#endif // USE_CL_DEVICE_FISSION

} // namespace cl

#endif // CL_HPP_
