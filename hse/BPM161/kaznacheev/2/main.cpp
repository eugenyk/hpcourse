#define CL_HPP_TARGET_OPENCL_VERSION 120
#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#include <CL/cl.h>
#include "cl.hpp"

#include <algorithm>
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <stdio.h>

int main()
{
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;
    std::vector<cl::Kernel> kernels;

    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    size_t n, m;
    std::cin >> n >> m;
    std::cerr << n << " " << m << std::endl;

    std::vector<double> a(n * n);
    std::vector<double> b(m * m);
    std::vector<double> c(n * n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            std::cin >> a[i * n + j];
        }
    }

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            std::cin >> b[i * m + j];
        }
    }

    try {
        cl::Platform::get(&platforms);

        platforms[0].getDevices(CL_DEVICE_TYPE_CPU, &devices);
        cl::Context context(devices);
        cl::CommandQueue queue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

        std::ifstream cl_file("convolution.cl");
        std::string cl_string(std::istreambuf_iterator<char>(cl_file), (std::istreambuf_iterator<char>()));
        cl::Program::Sources source(1, std::make_pair(cl_string.c_str(),
                                                      cl_string.length() + 1));
        cl::Program program(context, source);

        try {
            program.build(devices);
        }
        catch (cl::Error& e) {
            std::cerr << e.what() << std::endl;
            return 0;
        }

        size_t const block_size = 16;
        size_t const base_matrix_size = n * n;
        size_t const kernel_matrix_size = m * m;

        cl::Buffer dev_a(context, CL_MEM_READ_ONLY, sizeof(double) * base_matrix_size);
        cl::Buffer dev_b(context, CL_MEM_READ_ONLY, sizeof(double) * kernel_matrix_size);
        cl::Buffer dev_c(context, CL_MEM_WRITE_ONLY, sizeof(double) * base_matrix_size);

        queue.enqueueWriteBuffer(dev_a, CL_TRUE, 0, sizeof(double) * base_matrix_size, &a[0]);
        queue.enqueueWriteBuffer(dev_b, CL_TRUE, 0, sizeof(double) * kernel_matrix_size, &b[0]);

        cl::Kernel kernel_gmem(program, "convolve");
        cl::KernelFunctor convolve(kernel_gmem, queue, cl::NullRange, cl::NDRange(n, n), cl::NDRange(block_size, block_size));
        cl::Event event = convolve(dev_a, dev_b, dev_c, static_cast<int>(m), static_cast<int>(n));

        event.wait();

        queue.enqueueReadBuffer(dev_c, CL_TRUE, 0, sizeof(double) * base_matrix_size, &c[0]);

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                std::cout << c[i * n + j] << " ";
            }
            std::cout << std::endl;
        }
    }
    catch (cl::Error const & e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}