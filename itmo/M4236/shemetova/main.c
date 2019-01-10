#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)

int closest_size(int size, int block) {
    return ceil(size/(float) block)*block;
}

int main(int argc, char* argv[]) {
	
	if (argc < 2) {
		printf("Error: Please, specify input file!");
        exit(1);
	}
	
	int const MAX_N = 1024;
	int const MAX_M = 9;
	size_t const MAX_BLOCK = 16;
    // Initialize the two input matrices
    FILE* file = fopen(argv[1], "r"); 

    if (file == NULL){
        printf("Error Reading File\n");
        exit (0);
    }
	
	int N, M;
	int sc;
	int scanf_res;
	
	scanf_res = fscanf(file, "%d,", &N);
	scanf_res = fscanf(file, "%d,", &M);
	
	
	float* A = (float *)malloc(N * N * sizeof(float));
	float* B = (float *)malloc(M * M * sizeof(float));
	float* C = (float *)malloc(N * N * sizeof(float));
	
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
            sc = fscanf(file, "%f", &A[i*N + j]);
       }
	}
	
	for (int i = 0; i < M; ++i) {
		for (int j = 0; j < M; ++j) {
            sc = fscanf(file, "%f", &B[i*M + j]);
       }
	}

	
    fclose(file);
	
	int HM = (M - 1)/2;


    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen("matrix_conv.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;   
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, 
            &device_id, &ret_num_devices);
	if (ret != CL_SUCCESS)
    {
        printf("Error: Failed to get deviceID! %d\n", ret);
        exit(1);
    }

    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);

    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    // Create memory buffers on the device for each vector 
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, 
            MAX_N * MAX_N * sizeof(float), NULL, &ret);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
            MAX_M * MAX_M * sizeof(float), NULL, &ret);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
            MAX_N * MAX_N * sizeof(float), NULL, &ret);

    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
            N * N * sizeof(float), A, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, 
            M * M * sizeof(float), B, 0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf("Error: Failed to copy buffers! %d\n", ret);
        exit(1);
    }


    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, 
            (const char **)&source_str, (const size_t *)&source_size, &ret);

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	if (ret != CL_SUCCESS)
    {
        printf("Error: Failed to build program! %d\n", ret);
        exit(1);
    }

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "matrix_conv", &ret);


    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
    ret = clSetKernelArg(kernel, 3, sizeof(int), &N);
	if (ret != CL_SUCCESS)
    {
        printf("Error: Failed to set args! %d\n", ret);
        exit(1);
    }
    ret = clSetKernelArg(kernel, 4, sizeof(int), &M);
	ret = clSetKernelArg(kernel, 5, sizeof(int), &HM);
	if (ret != CL_SUCCESS)
    {
        printf("Error: Failed to set args! %d\n", ret);
        exit(1);
    }
    
    // Execute the OpenCL kernel on the list
	int grid = closest_size(N, MAX_BLOCK);
	const size_t local[2] = { MAX_BLOCK, MAX_BLOCK };
    const size_t global[2] = { grid, grid };
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, 
            global, local, 0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve kernel work group info! %d\n", ret);
        exit(1);
    }


    // Read the memory buffer C on the device to the local variable C
    //int *C = (int*)malloc(sizeof(int)*LIST_SIZE);
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, 
            N * N * sizeof(float), C, 0, NULL, NULL);

    // Display the result on the screen
    for(int i = 0; i < N; ++i) {
		printf("\n");
       for(int j = 0; j < N; ++j) {
        printf("%.3f ", C[i*N + j]);
	   }
	}
	printf("\n");

    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
    free(A);
    free(B);
    free(C);
    return 0;
}

