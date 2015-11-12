/*
 *  Simple OpenCL demo program
 *
 *  Copyright (C) 2009  Clifford Wolf <clifford@clifford.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  gcc -o cldemo -std=gnu99 -Wall -I/usr/include/nvidia-current cldemo.c -lOpenCL
 *
 */

#ifdef __APPLE__ //ifdef added from web suggestions
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
//#include <cl.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cl_errors.h"

#define CL_DEVICE_TYPE_TO_QUERY (CL_DEVICE_TYPE_ALL)
//#define CL_DEVICE_TYPE_TO_QUERY (CL_DEVICE_TYPE_CPU)
//#define CL_DEVICE_TYPE_TO_QUERY (CL_DEVICE_TYPE_GPU)
//#define CL_DEVICE_TYPE_TO_QUERY (CL_DEVICE_TYPE_DEFAULT)

//#define TU_BERLIN_WORKAROUND

#define NUM_DATA 100

#define CL_CHECK(_expr)                                                         \
    do {                                                                         \
        cl_int _err = _expr;                                                       \
        if (_err == CL_SUCCESS)                                                    \
            break;                                                                   \
        fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err);   \
        fprintf(stderr, "Error: %s\n", print_cl_errstring(_err));                  \
        abort();                                                                   \
    } while (0)

#define CL_CHECK_ERR(_expr)                                                     \
    ({                                                                           \
        cl_int _err = CL_INVALID_VALUE;                                            \
        typeof(_expr) _ret = _expr;                                                \
        if (_err != CL_SUCCESS) {                                                  \
            fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err); \
            fprintf(stderr, "Error: %s\n", print_cl_errstring(_err));                \
            abort();                                                                 \
        }                                                                          \
        _ret;                                                                      \
    })

void pfn_notify(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
	fprintf(stderr, "OpenCL Error (via pfn_notify): %s\n", errinfo);
}

int main(int argc, char **argv)
{
    // FIND OUT HOW MANY PLATFORMS
    cl_platform_id *platforms = NULL;
	cl_uint platforms_n = 0;
    // find out how many platforms available
    CL_CHECK(clGetPlatformIDs(0, NULL, &platforms_n));
    //printf("DEBUG: platforms_n = %d\n", platforms_n);
    platforms = malloc(platforms_n * sizeof(cl_platform_id));
    // get the platform IDs
	CL_CHECK(clGetPlatformIDs(platforms_n, platforms, &platforms_n));

    // QUERY INDIVIDUAL PLATFORM INFO
	printf("=== %d OpenCL platform(s) found: ===\n", platforms_n);
    const int attributeCount = 5;
    const char* attributeNames[attributeCount] = { "Name", "Vendor", "Version", "Profile", "Extensions" };
    const cl_platform_info attributeTypes[attributeCount] = {
        CL_PLATFORM_NAME,
        CL_PLATFORM_VENDOR,
        CL_PLATFORM_VERSION,
        CL_PLATFORM_PROFILE,
        CL_PLATFORM_EXTENSIONS };
    char *info;
    size_t infoSize = 0;
	for (int i=0; i<platforms_n; i++)
	{
        printf("  -- %d --\n", i);
        
        for (int j = 0; j < attributeCount; j++) {
            // get platform attribute value size
            clGetPlatformInfo(platforms[i], attributeTypes[j], 0, NULL, &infoSize);
            info = (char*) malloc(infoSize);
            
            // get platform attribute value
            clGetPlatformInfo(platforms[i], attributeTypes[j], infoSize, info, NULL);
            printf("  %d.%d %-11s = %s\n", i+1, j+1, attributeNames[j], info);
        } 
        printf("\n\n"); 
    //}

	if (platforms_n == 0)
		return 1;
        
    //Hack for TU Berlin setup
#ifdef TU_BERLIN_WORKAROUND
    if (i == 0)
        continue;
#endif
        
    // FIND OUT HOW MANY DEVICES
	cl_device_id *devices = NULL;
	cl_uint devices_n = 0;
    printf("Querying device numbers,");
    // find out how many platforms available
    CL_CHECK(clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_TO_QUERY, 0, NULL, &devices_n));
    printf(" device numbers queried\n");
    printf("There are %d devices on this platform\n", (int)devices_n);
    // collect device IDs for this platform
    devices = malloc(devices_n * sizeof(cl_device_id));
    CL_CHECK(clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_TO_QUERY, devices_n, devices, NULL));
	
    // QUERY INDIVIDUAL DEVICES ON EACH PLATFORM
	printf("=== %d OpenCL device(s) found on %d platform:\n", devices_n, platforms_n);
    //printf("devices_n = %d \n", devices_n);
	for (int i=0; i<devices_n; i++)
	{
		char buffer[10240];
		size_t buf_sizet;
        size_t *buf_sizet_array = NULL;
        
        cl_uint buf_uint;
		cl_ulong buf_ulong;
        cl_bool buf_bool;
        cl_device_type buf_type;
        cl_platform_id buf_platform;
        
        
        printf("  -- %d --\n", i);
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(buffer), buffer, NULL));
        printf("  DEVICE_NAME = %s\n", buffer);
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL));
        printf("  DEVICE_VENDOR = %s\n", buffer);
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL));
        printf("  DEVICE_VERSION = %s\n", buffer);
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL));
        printf("  DRIVER_VERSION = %s\n", buffer);
        
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(buf_type), &buf_type, NULL));
        printf("  DEVICE_TYPE = %d\n", (int)buf_type);
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_PLATFORM, sizeof(buf_platform), &buf_platform, NULL));
        printf("  DEVICE_PLATFORM = %d\n", (int)buf_platform);
     
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(buf_uint), &buf_uint, NULL));
        printf("  DEVICE_MAX_CLOCK_FREQUENCY = %u\n", (unsigned int)buf_uint);
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(buf_uint), &buf_uint, NULL));
        printf("  DEVICE_MAX_CONSTANT_ARGS = %u\n", (unsigned int)buf_uint);
        
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_ulong), &buf_ulong, NULL));
        printf("  DEVICE_GLOBAL_MEM_SIZE = %llu\n", (unsigned long long)buf_ulong);
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(buf_ulong), &buf_ulong, NULL));
        printf("  DEVICE_MAX_MEM_ALLOC_SIZE = %llu\n", (unsigned long long)buf_ulong);
        
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buf_uint), &buf_uint, NULL));
        printf("  DEVICE_MAX_COMPUTE_UNITS = %u\n", (unsigned int)buf_uint);
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &buf_sizet, NULL));
        printf("  DEVICE_MAX_WORK_GROUP_SIZE = %u\n", (unsigned int)buf_sizet);
        
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(buf_uint), &buf_uint, NULL));
        printf("  DEVICE_MAX_WORK_ITEM_DIMENSIONS = %u\n", (unsigned int)buf_uint);
        buf_sizet_array = malloc(buf_uint * sizeof(size_t));
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, buf_uint * sizeof(size_t), buf_sizet_array, NULL));
        for (int j = 0; j<buf_uint; j++)
            printf("  DEVICE_MAX_WORK_ITEM_SIZES[%d] = %u\n", j, (unsigned int)buf_sizet_array[j]);
        
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_AVAILABLE, sizeof(buf_bool), &buf_bool, NULL));
        printf("  DEVICE_AVAILABLE = %d\n", (int)buf_bool);
        
        CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_EXTENSIONS, sizeof(buffer), buffer, NULL));
        printf("  DEVICE_EXTENSIONS = %s\n", buffer);

	//}

        printf("DEBUG: finished querying device capabilities\n\n\n");
        
	if (devices_n == 0)
		return 1;

	cl_context context;
	context = CL_CHECK_ERR(clCreateContext(NULL, 1, devices, &pfn_notify, NULL, &_err));

	const char *program_source[] = {
		"__kernel void simple_demo(__global int *src, __global int *dst, int factor)\n",
		"{\n",
		"	int i = get_global_id(0);\n",
		"	dst[i] = src[i] * factor;\n",
		"}\n"
	};

	cl_program program;
	program = CL_CHECK_ERR(clCreateProgramWithSource(context, sizeof(program_source)/sizeof(*program_source), program_source, NULL, &_err));
	if (clBuildProgram(program, 1, devices, "", NULL, NULL) != CL_SUCCESS) {
		char buffer[10240];
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
		fprintf(stderr, "CL Compilation failed:\n%s", buffer);
		abort();
	}
	//CL_CHECK(clUnloadCompiler());

	cl_mem input_buffer;
	input_buffer = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)*NUM_DATA, NULL, &_err));

	cl_mem output_buffer;
	output_buffer = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int)*NUM_DATA, NULL, &_err));

	int factor = 2;

	cl_kernel kernel;
	kernel = CL_CHECK_ERR(clCreateKernel(program, "simple_demo", &_err));
	CL_CHECK(clSetKernelArg(kernel, 0, sizeof(input_buffer), &input_buffer));
	CL_CHECK(clSetKernelArg(kernel, 1, sizeof(output_buffer), &output_buffer));
	CL_CHECK(clSetKernelArg(kernel, 2, sizeof(factor), &factor));

//insert
    size_t local;
    CL_CHECK(clGetKernelWorkGroupInfo(kernel, devices[i], CL_KERNEL_WORK_GROUP_SIZE, sizeof((local)), &local, NULL));
    printf(" CL_KERNEL_WORK_GROUP_SIZE: %d \n", (int)local);
    CL_CHECK(clGetKernelWorkGroupInfo(kernel, devices[i], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof((local)), &local, NULL) );
    printf(" CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: %d \n", (int)local);
//end insert
        
	cl_command_queue queue;
	queue = CL_CHECK_ERR(clCreateCommandQueue(context, devices[0], 0, &_err));

	for (int i=0; i<NUM_DATA; i++) {
		CL_CHECK(clEnqueueWriteBuffer(queue, input_buffer, CL_TRUE, i*sizeof(int), sizeof(int), &i, 0, NULL, NULL));
	}

	cl_event kernel_completion;
	size_t global_work_size[1] = { NUM_DATA };
	CL_CHECK(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, &kernel_completion));
	CL_CHECK(clWaitForEvents(1, &kernel_completion));
	CL_CHECK(clReleaseEvent(kernel_completion));

	printf("Result:");
	for (int i=0; i<NUM_DATA; i++) {
		int data;
		CL_CHECK(clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, i*sizeof(int), sizeof(int), &data, 0, NULL, NULL));
		printf(" %d", data);
	}
	printf("\n");

	CL_CHECK(clReleaseMemObject(input_buffer));
	CL_CHECK(clReleaseMemObject(output_buffer));

	CL_CHECK(clReleaseKernel(kernel));
	CL_CHECK(clReleaseProgram(program));
	CL_CHECK(clReleaseContext(context));

    } //end devices per platform loop
    } //end platforms loop
    
	return 0;
}

