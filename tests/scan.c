//
// File:       scan.c
//
// Abstract:   This example shows how to perform an efficient parallel prefix sum (aka Scan)
//             using OpenCL.  Scan is a common data parallel primitive which can be used for 
//             variety of different operations -- this example uses local memory for storing
//             partial sums and avoids memory bank conflicts on architectures which serialize
//             memory operations that are serviced on the same memory bank by offsetting the
//             loads and stores based on the size of the local group and the number of
//             memory banks (see appropriate macro definition).  As a result, this example
//             requires that the local group size > 1.
//
// Version:    <1.0>
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright ( C ) 2008 Apple Inc. All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef __EMSCRIPTEN__
  #include <emscripten/emscripten.h>
  #include <CL/opencl.h>
#else
  #include <libc.h>
  #include <OpenCL/opencl.h>
#endif

#ifdef __APPLE__
  #include <mach/mach_time.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_INFO      (0)
int		GROUP_SIZE      = 256;
#define NUM_BANKS       (16)
#define MAX_ERROR       (1e-7)
#define SEPARATOR       ("----------------------------------------------------------------------\n")

#define min(A,B) ((A) < (B) ? (A) : (B))

static int iterations = 1000;
static int count      = 1024 * 1024;    

////////////////////////////////////////////////////////////////////////////////////////////////////

cl_device_id            ComputeDeviceId;
cl_command_queue        ComputeCommands;
cl_context              ComputeContext;
cl_program              ComputeProgram;
cl_kernel*              ComputeKernels;
cl_mem*                 ScanPartialSums = 0;
unsigned int            ElementsAllocated = 0;
unsigned int            LevelsAllocated = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////

enum KernelMethods
{
    PRESCAN                             = 0,
    PRESCAN_STORE_SUM                   = 1,
    PRESCAN_STORE_SUM_NON_POWER_OF_TWO  = 2,
    PRESCAN_NON_POWER_OF_TWO            = 3,
    UNIFORM_ADD                         = 4
};

static const char* KernelNames[] =
{
    "PreScanKernel",
    "PreScanStoreSumKernel",
    "PreScanStoreSumNonPowerOfTwoKernel",
    "PreScanNonPowerOfTwoKernel",
    "UniformAddKernel"
};

static const unsigned int KernelCount = sizeof(KernelNames) / sizeof(char *);

////////////////////////////////////////////////////////////////////////////////////////////////////

uint64_t
GetCurrentTime()
{
#ifdef __EMSCRIPTEN__
  return emscripten_get_now();
#else
  return mach_absolute_time();
#endif

}
	
double 
SubtractTimeInSec( uint64_t endtime, uint64_t starttime )
{    
#ifdef __EMSCRIPTEN__
  return (endtime - starttime) / 1000.0;
#else
	static double conversion = 0.0;
	uint64_t difference = endtime - starttime;
	if( 0 == conversion )
	{
		mach_timebase_info_data_t timebase;
		kern_return_t kError = mach_timebase_info( &timebase );
		if( kError == 0  )
			conversion = 1e-9 * (double) timebase.numer / (double) timebase.denom;
    }
		
	return conversion * (double) difference; 
#endif
}

static char *
LoadProgramSourceFromFile(const char *filename)
{
    struct stat statbuf;
    FILE        *fh;
    char        *source;

    fh = fopen(filename, "r");
    if (fh == 0)
        return 0;

    stat(filename, &statbuf);
    source = (char *) malloc(statbuf.st_size + 1);
    fread(source, statbuf.st_size, 1, fh);
    source[statbuf.st_size] = '\0';

    return source;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsPowerOfTwo(int n)
{
    return ((n&(n-1))==0) ;
}

int floorPow2(int n)
{
    int exp;
    frexp((float)n, &exp);
    return 1 << (exp - 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int 
CreatePartialSumBuffers(unsigned int count)
{
    ElementsAllocated = count;

    unsigned int group_size = GROUP_SIZE;
    unsigned int element_count = count;

    int level = 0;

    do
    {       
        unsigned int group_count = (int)fmax(1, (int)ceil((float)element_count / (2.0f * group_size)));
        if (group_count > 1)
        {
            level++;
        }
        element_count = group_count;
        
    } while (element_count > 1);

    ScanPartialSums = (cl_mem*) malloc(level * sizeof(cl_mem));
    LevelsAllocated = level;
    memset(ScanPartialSums, 0, sizeof(cl_mem) * level);
    
    element_count = count;
    level = 0;
    
    do
    {       
        unsigned int group_count = (int)fmax(1, (int)ceil((float)element_count / (2.0f * group_size)));
        if (group_count > 1) 
        {
            size_t buffer_size = group_count * sizeof(float);
            ScanPartialSums[level++] = clCreateBuffer(ComputeContext, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);
        }

        element_count = group_count;

    } while (element_count > 1);

    return CL_SUCCESS;
}

void 
ReleasePartialSums(void)
{
    unsigned int i;
    for (i = 0; i < LevelsAllocated; i++)
    {
        clReleaseMemObject(ScanPartialSums[i]);
    }    
    
    free(ScanPartialSums);
    ScanPartialSums = 0;
    ElementsAllocated = 0;
    LevelsAllocated = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
PreScan(
    size_t *global, 
    size_t *local, 
    size_t shared, 
    cl_mem output_data, 
    cl_mem input_data, 
    unsigned int n,
    int group_index, 
    int base_index)
{
#if DEBUG_INFO
    printf("PreScan: Global[%4d] Local[%4d] Shared[%4d] BlockIndex[%4d] BaseIndex[%4d] Entries[%d]\n", 
        (int)global[0], (int)local[0], (int)shared, group_index, base_index, n);
#endif

    unsigned int k = PRESCAN;
    unsigned int a = 0;

    int err = CL_SUCCESS;
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &output_data);  
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &input_data);
    err |= clSetKernelArg(ComputeKernels[k],  a++, shared,         0);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &group_index);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &base_index);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &n);
    if (err != CL_SUCCESS)
    {
        printf("Error: %s: Failed to set kernel arguments!\n", KernelNames[k]);
        return EXIT_FAILURE;
    }

    err = CL_SUCCESS;
    err |= clEnqueueNDRangeKernel(ComputeCommands, ComputeKernels[k], 1, NULL, global, local, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: %s: Failed to execute kernel!\n", KernelNames[k]);
        return EXIT_FAILURE;
    }

    return CL_SUCCESS;
}

int
PreScanStoreSum(
    size_t *global, 
    size_t *local, 
    size_t shared, 
    cl_mem output_data, 
    cl_mem input_data, 
    cl_mem partial_sums,
    unsigned int n,
    int group_index, 
    int base_index)
{
#if DEBUG_INFO
    printf("PreScan: Global[%4d] Local[%4d] Shared[%4d] BlockIndex[%4d] BaseIndex[%4d] Entries[%d]\n", 
        (int)global[0], (int)local[0], (int)shared, group_index, base_index, n);
#endif

    unsigned int k = PRESCAN_STORE_SUM;
    unsigned int a = 0;

    int err = CL_SUCCESS;
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &output_data);  
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &input_data);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &partial_sums);
    err |= clSetKernelArg(ComputeKernels[k],  a++, shared,         0);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &group_index);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &base_index);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &n);
    if (err != CL_SUCCESS)
    {
        printf("Error: %s: Failed to set kernel arguments!\n", KernelNames[k]);
        return EXIT_FAILURE;
    }

    err = CL_SUCCESS;
    err |= clEnqueueNDRangeKernel(ComputeCommands, ComputeKernels[k], 1, NULL, global, local, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: %s: Failed to execute kernel!\n", KernelNames[k]);
        return EXIT_FAILURE;
    }
    
    return CL_SUCCESS;
}

int
PreScanStoreSumNonPowerOfTwo(
    size_t *global, 
    size_t *local, 
    size_t shared, 
    cl_mem output_data, 
    cl_mem input_data, 
    cl_mem partial_sums,
    unsigned int n,
    int group_index, 
    int base_index)
{
#if DEBUG_INFO
    printf("PreScanStoreSumNonPowerOfTwo: Global[%4d] Local[%4d] BlockIndex[%4d] BaseIndex[%4d] Entries[%d]\n", 
        (int)global[0], (int)local[0], group_index, base_index, n);
#endif

    unsigned int k = PRESCAN_STORE_SUM_NON_POWER_OF_TWO;
    unsigned int a = 0;

    int err = CL_SUCCESS;
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &output_data);  
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &input_data);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &partial_sums);
    err |= clSetKernelArg(ComputeKernels[k],  a++, shared,         0);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &group_index);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &base_index);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &n);
    if (err != CL_SUCCESS)
    {
        printf("Error: %s: Failed to set kernel arguments!\n", KernelNames[k]);
        return EXIT_FAILURE;
    }

    err = CL_SUCCESS;
    err |= clEnqueueNDRangeKernel(ComputeCommands, ComputeKernels[k], 1, NULL, global, local, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: %s: Failed to execute kernel!\n", KernelNames[k]);
        return EXIT_FAILURE;
    }

    return CL_SUCCESS;
}

int
PreScanNonPowerOfTwo(
    size_t *global, 
    size_t *local, 
    size_t shared, 
    cl_mem output_data, 
    cl_mem input_data, 
    unsigned int n,
    int group_index, 
    int base_index)
{
#if DEBUG_INFO
    printf("PreScanNonPowerOfTwo: Global[%4d] Local[%4d] BlockIndex[%4d] BaseIndex[%4d] Entries[%d]\n", 
        (int)global[0], (int)local[0], group_index, base_index, n);
#endif

    unsigned int k = PRESCAN_NON_POWER_OF_TWO;
    unsigned int a = 0;

    int err = CL_SUCCESS;
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &output_data);  
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &input_data);
    err |= clSetKernelArg(ComputeKernels[k],  a++, shared,         0);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &group_index);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &base_index);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &n);
    if (err != CL_SUCCESS)
    {
        printf("Error: %s: Failed to set kernel arguments!\n", KernelNames[k]);
        return EXIT_FAILURE;
    }

    err = CL_SUCCESS;
    err |= clEnqueueNDRangeKernel(ComputeCommands, ComputeKernels[k], 1, NULL, global, local, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: %s: Failed to execute kernel!\n", KernelNames[k]);
        return EXIT_FAILURE;
    }
    return CL_SUCCESS;
}

int
UniformAdd(
    size_t *global, 
    size_t *local, 
    cl_mem output_data, 
    cl_mem partial_sums, 
    unsigned int n, 
    unsigned int group_offset, 
    unsigned int base_index)
{
#if DEBUG_INFO
    printf("UniformAdd: Global[%4d] Local[%4d] BlockOffset[%4d] BaseIndex[%4d] Entries[%d]\n", 
        (int)global[0], (int)local[0], group_offset, base_index, n);
#endif

    unsigned int k = UNIFORM_ADD;
    unsigned int a = 0;

    int err = CL_SUCCESS;
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &output_data);  
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_mem), &partial_sums);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(float),  0);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &group_offset);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &base_index);
    err |= clSetKernelArg(ComputeKernels[k],  a++, sizeof(cl_int), &n);
    if (err != CL_SUCCESS)
    {
        printf("Error: %s: Failed to set kernel arguments!\n", KernelNames[k]);
        return EXIT_FAILURE;
    }

    err = CL_SUCCESS;
    err |= clEnqueueNDRangeKernel(ComputeCommands, ComputeKernels[k], 1, NULL, global, local, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: %s: Failed to execute kernel!\n", KernelNames[k]);
        return EXIT_FAILURE;
    }

    return CL_SUCCESS;
}

int 
PreScanBufferRecursive(
    cl_mem output_data, 
    cl_mem input_data, 
    int max_group_size,
    int max_work_item_count,
    int element_count, 
    int level)
{
    unsigned int group_size = max_group_size; 
    unsigned int group_count = (int)fmax(1.0f, (int)ceil((float)element_count / (2.0f * group_size)));
    unsigned int work_item_count = 0;

    if (group_count > 1)
        work_item_count = group_size;
    else if (IsPowerOfTwo(element_count))
        work_item_count = element_count / 2;
    else
        work_item_count = floorPow2(element_count);
        
    work_item_count = (work_item_count > max_work_item_count) ? max_work_item_count : work_item_count;

    unsigned int element_count_per_group = work_item_count * 2;
    unsigned int last_group_element_count = element_count - (group_count-1) * element_count_per_group;
    unsigned int remaining_work_item_count = (int)fmax(1.0f, last_group_element_count / 2);
    remaining_work_item_count = (remaining_work_item_count > max_work_item_count) ? max_work_item_count : remaining_work_item_count;
    unsigned int remainder = 0;
    size_t last_shared = 0;

    
    if (last_group_element_count != element_count_per_group)
    {
        remainder = 1;

        if(!IsPowerOfTwo(last_group_element_count))
            remaining_work_item_count = floorPow2(last_group_element_count);    
        
        remaining_work_item_count = (remaining_work_item_count > max_work_item_count) ? max_work_item_count : remaining_work_item_count;
        unsigned int padding = (2 * remaining_work_item_count) / NUM_BANKS;
        last_shared = sizeof(float) * (2 * remaining_work_item_count + padding);
    }

    remaining_work_item_count = (remaining_work_item_count > max_work_item_count) ? max_work_item_count : remaining_work_item_count;
    size_t global[] = { (int)fmax(1, group_count - remainder) * work_item_count, 1 };
    size_t local[]  = { work_item_count, 1 };  

    unsigned int padding = element_count_per_group / NUM_BANKS;
    size_t shared = sizeof(float) * (element_count_per_group + padding);
    
    cl_mem partial_sums = ScanPartialSums[level];
    int err = CL_SUCCESS;
    
    if (group_count > 1)
    {
        err = PreScanStoreSum(global, local, shared, output_data, input_data, partial_sums, work_item_count * 2, 0, 0);
        if(err != CL_SUCCESS)
            return err;
            
        if (remainder)
        {
            size_t last_global[] = { 1 * remaining_work_item_count, 1 };
            size_t last_local[]  = { remaining_work_item_count, 1 };  

            err = PreScanStoreSumNonPowerOfTwo(
                    last_global, last_local, last_shared, 
                    output_data, input_data, partial_sums,
                    last_group_element_count, 
                    group_count - 1, 
                    element_count - last_group_element_count);    
        
            if(err != CL_SUCCESS)
                return err;			
			
        }

        err = PreScanBufferRecursive(partial_sums, partial_sums, max_group_size, max_work_item_count, group_count, level + 1);
        if(err != CL_SUCCESS)
            return err;
            
        err = UniformAdd(global, local, output_data, partial_sums,  element_count - last_group_element_count, 0, 0);
        if(err != CL_SUCCESS)
            return err;
        
        if (remainder)
        {
            size_t last_global[] = { 1 * remaining_work_item_count, 1 };
            size_t last_local[]  = { remaining_work_item_count, 1 };  

            err = UniformAdd(
                    last_global, last_local, 
                    output_data, partial_sums,
                    last_group_element_count, 
                    group_count - 1, 
                    element_count - last_group_element_count);
                
            if(err != CL_SUCCESS)
                return err;
        }
    }
    else if (IsPowerOfTwo(element_count))
    {
        err = PreScan(global, local, shared, output_data, input_data, work_item_count * 2, 0, 0);
        if(err != CL_SUCCESS)
            return err;
    }
    else
    {
        err = PreScanNonPowerOfTwo(global, local, shared, output_data, input_data, element_count, 0, 0);
        if(err != CL_SUCCESS)
            return err;
    }

    return CL_SUCCESS;
}

void 
PreScanBuffer(
    cl_mem output_data, 
    cl_mem input_data, 
    unsigned int max_group_size,
    unsigned int max_work_item_count,
    unsigned int element_count)
{
    PreScanBufferRecursive(output_data, input_data, max_group_size, max_work_item_count, element_count, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ScanReference( float* reference, float* input, const unsigned int count) 
{
    reference[0] = 0;
    double total_sum = 0;
    
    unsigned int i = 1;
    for( i = 1; i < count; ++i) 
    {
        total_sum += input[i-1];
        reference[i] = input[i-1] + reference[i-1];
    }
    if (total_sum != reference[count-1])
        printf("Warning: Exceeding single-precision accuracy.  Scan will be inaccurate.\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    int i;
    uint64_t         t0 = 0;
    uint64_t         t1 = 0;
    uint64_t         t2 = 0;
    int              err = 0;
    cl_mem			 output_buffer;
    cl_mem           input_buffer;
    
    int use_gpu = 1;
    for( i = 0; i < argc && argv; i++)
    {
        if(!argv[i])
            continue;
            
        if(strstr(argv[i], "cpu"))
            use_gpu = 0;        

        else if(strstr(argv[i], "gpu"))
            use_gpu = 1;
    }
    
    // Create some random input data on the host 
    //
    float *float_data = (float*)malloc(count * sizeof(float));
    for (i = 0; i < count; i++)
    {
        float_data[i] = (int)(10 * ((float) rand() / (float) RAND_MAX));
    }

    // Connect to a GPU compute device
    //
    err = clGetDeviceIDs(NULL, use_gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &ComputeDeviceId, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to locate a compute device!\n");
        return EXIT_FAILURE;
    }

    size_t returned_size = 0;
    size_t max_workgroup_size = 0;
    err = clGetDeviceInfo(ComputeDeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_workgroup_size, &returned_size);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve device info!\n");
        return EXIT_FAILURE;
    }
	
	GROUP_SIZE = min( GROUP_SIZE, max_workgroup_size );

    cl_char vendor_name[1024] = {0};
    cl_char device_name[1024] = {0};
    err = clGetDeviceInfo(ComputeDeviceId, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
    err|= clGetDeviceInfo(ComputeDeviceId, CL_DEVICE_NAME, sizeof(device_name), device_name, &returned_size);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve device info!\n");
        return EXIT_FAILURE;
    }

    printf(SEPARATOR);
    printf("Connecting to %s %s...\n", vendor_name, device_name);

    // Load the compute program from disk into a cstring buffer
    //
    printf(SEPARATOR);
    const char* filename = "scan_kernel.cl";
    printf("Loading program '%s'...\n", filename);
    printf(SEPARATOR);

    char *source = LoadProgramSourceFromFile(filename);
    if(!source)
    {
        printf("Error: Failed to load compute program from file!\n");
        return EXIT_FAILURE;    
    }
    
    // Create a compute ComputeContext 
    //
    ComputeContext = clCreateContext(0, 1, &ComputeDeviceId, NULL, NULL, &err);
    if (!ComputeContext)
    {
        printf("Error: Failed to create a compute ComputeContext!\n");
        return EXIT_FAILURE;
    }

    // Create a command queue
    //
    ComputeCommands = clCreateCommandQueue(ComputeContext, ComputeDeviceId, 0, &err);
    if (!ComputeCommands)
    {
        printf("Error: Failed to create a command ComputeCommands!\n");
        return EXIT_FAILURE;
    }

    // Create the compute program from the source buffer
    //
    ComputeProgram = clCreateProgramWithSource(ComputeContext, 1, (const char **) & source, NULL, &err);
    if (!ComputeProgram || err != CL_SUCCESS)
    {
        printf("%s\n", source);
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
    }
    
    // Build the program executable
    //
    err = clBuildProgram(ComputeProgram, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t length;
        char build_log[2048];
        printf("%s\n", source);
        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(ComputeProgram, ComputeDeviceId, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, &length);
        printf("%s\n", build_log);
        return EXIT_FAILURE;
    }

    ComputeKernels = (cl_kernel*) malloc(KernelCount * sizeof(cl_kernel));
    for(i = 0; i < KernelCount; i++)
    {    
        // Create each compute kernel from within the program
        //
        ComputeKernels[i] = clCreateKernel(ComputeProgram, KernelNames[i], &err);
        if (!ComputeKernels[i] || err != CL_SUCCESS)
        {
            printf("Error: Failed to create compute kernel!\n");
            return EXIT_FAILURE;
        }
		
		size_t wgSize;
		err = clGetKernelWorkGroupInfo(ComputeKernels[i], ComputeDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &wgSize, NULL); 
		if(err)
		{
			printf("Error: Failed to get kernel work group size\n");
			return EXIT_FAILURE;
		}
		GROUP_SIZE = min( GROUP_SIZE, wgSize );
		
    }

    free(source);

    // Create the input buffer on the device
    //
    size_t buffer_size = sizeof(float) * count;
    input_buffer = clCreateBuffer(ComputeContext, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);
    if (!input_buffer)
    {
        printf("Error: Failed to allocate input buffer on device!\n");
        return EXIT_FAILURE;
    }

    // Fill the input buffer with the host allocated random data
    //
    err = clEnqueueWriteBuffer(ComputeCommands, input_buffer, CL_TRUE, 0, buffer_size, float_data, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        return EXIT_FAILURE;
    }

    // Create the output buffer on the device
    //
    output_buffer = clCreateBuffer(ComputeContext, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);
    if (!output_buffer)
    {
        printf("Error: Failed to allocate result buffer on device!\n");
        return EXIT_FAILURE;
    }

    float* result = (float*)malloc(buffer_size);
    memset(result, 0, buffer_size);
	
    err = clEnqueueWriteBuffer(ComputeCommands, output_buffer, CL_TRUE, 0, buffer_size, result, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        return EXIT_FAILURE;
    }
		
    CreatePartialSumBuffers(count);
    PreScanBuffer(output_buffer, input_buffer, GROUP_SIZE, GROUP_SIZE, count);

    printf("Starting timing run of '%d' iterations...\n", iterations);

    t0 = t1 = GetCurrentTime();
    for (i = 0; i < iterations; i++)
    {
        PreScanBuffer(output_buffer, input_buffer, GROUP_SIZE, GROUP_SIZE, count);
    }

    err = clFinish(ComputeCommands);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to wait for command queue to finish! %d\n", err);
        return EXIT_FAILURE;
    }
    t2 = GetCurrentTime();

    
    // Calculate the statistics for execution time and throughput
    //
    double t = SubtractTimeInSec(t2, t1);
    printf("Exec Time:  %.2f ms\n", 1000.0 * t / (double)(iterations));
    printf("Throughput: %.2f GB/sec\n", 1e-9 * buffer_size * iterations / t);
    printf(SEPARATOR);

    // Read back the results that were computed on the device
    //
    err = clEnqueueReadBuffer(ComputeCommands, output_buffer, CL_TRUE, 0, buffer_size, result, 0, NULL, NULL);
    if (err)
    {
        printf("Error: Failed to read back results from the device!\n");
        return EXIT_FAILURE;
    }

    // Verify the results are correct
    //
    float* reference = (float*) malloc( buffer_size); 
    ScanReference(reference, float_data, count);

    float error = 0.0f;
    float diff = 0.0f;
    for(i = 0; i < count; i++)
    {
        diff = fabs(reference[i] - result[i]);
        error = diff > error ? diff : error;
    }

    if (error > MAX_ERROR)
    {
        printf("Error:   Incorrect results obtained! Max error = %f\n", error);
        return EXIT_FAILURE;
    }
    else
    {
        printf("Results Validated!\n");
        printf(SEPARATOR);
    }
    
    // Shutdown and cleanup
    //
    ReleasePartialSums();    
    for(i = 0; i < KernelCount; i++)
        clReleaseKernel(ComputeKernels[i]);
    clReleaseProgram(ComputeProgram);
    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
    clReleaseCommandQueue(ComputeCommands);
    clReleaseContext(ComputeContext);
    
    free(ComputeKernels);
    free(float_data);
    free(reference);
    free(result);
    
        
    return 0;
}

