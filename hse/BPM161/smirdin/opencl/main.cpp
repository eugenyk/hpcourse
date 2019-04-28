#define __CL_ENABLE_EXCEPTIONS

#ifdef __APPLE__

#include <OpenCL/cl_platform.h>

#else
#include <CL/cl_platform.h>
#endif

#include "cl.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <cmath>


float* read_vector_data(int size) {
    auto a = new float[size];
    for (int i = 0; i < size; i++) {
        float value;
        std::cin >> value;
        a[i] = value;
    }
    return a;
}

void write_output(float* a, int size, int extended_size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            std::cout << a[extended_size * i + j] << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;
    std::vector<cl::Kernel> kernels;

    try {

        // create platform
        cl::Platform::get(&platforms);
        platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);

        // create context
        cl::Context context(devices);

        // create command queue
        cl::CommandQueue queue(context, devices[0]);

        // load opencl source
        std::ifstream cl_file("convolution.cl");
        std::string cl_string(std::istreambuf_iterator<char>(cl_file),
                              (std::istreambuf_iterator<char>()));
        cl::Program::Sources source(1, std::make_pair(cl_string.c_str(),
                                                      cl_string.length() + 1));

        // create program
        cl::Program program(context, source);

        // compile opencl source
        size_t const block_size = 16;
        program.build(devices, "-D BLOCK_SIZE=16");

        freopen("input.txt", "r", stdin);
        freopen("output.txt", "w", stdout);

        int N, M;
        std::cin >> N >> M;

        int extended_N = ceil(1. * N / block_size) * block_size;

        const size_t size_a = extended_N * extended_N;
        const size_t size_b = M * M;

        float* real_a = read_vector_data(N * N);
        auto* a = new float[size_a];
        for (int i = 0; i < extended_N; i++) {
            for (int j = 0; j < extended_N; j++) {
                a[i * extended_N + j] = (i >= N || j >= N) ? 0 : real_a[i * N + j];
            }
        }

        float* b = read_vector_data(size_b);
        auto* c = new float[size_a];

        // allocate device buffer to hold message
        cl::Buffer dev_a(context, CL_MEM_READ_ONLY, sizeof(float) * size_a);
        cl::Buffer dev_b(context, CL_MEM_READ_ONLY, sizeof(float) * size_b);
        cl::Buffer dev_c(context, CL_MEM_WRITE_ONLY, sizeof(float) * size_a);

        // copy from cpu to gpu
        queue.enqueueWriteBuffer(dev_a, CL_TRUE, 0, sizeof(float) * size_a, a);
        queue.enqueueWriteBuffer(dev_b, CL_TRUE, 0, sizeof(float) * size_b, b);

        // load named kernel from opencl source
        cl::Kernel kernel(program, "matrix_mult");
        cl::KernelFunctor matrix_mult(kernel, queue, cl::NullRange,
                                      cl::NDRange(extended_N, extended_N),
                                      cl::NDRange(block_size, block_size));

        matrix_mult(dev_a, dev_b, dev_c, (int) extended_N, (int) M);

        queue.enqueueReadBuffer(dev_c, CL_TRUE, 0, sizeof(float) * size_a, c);

        write_output(c, N, extended_N);

        delete[] a;
        delete[] real_a;
        delete[] b;
        delete[] c;
    }
    catch (cl::Error& e) {
        std::cout << std::endl << e.what() << " : " << e.err() << std::endl;
    }

    return 0;
}


