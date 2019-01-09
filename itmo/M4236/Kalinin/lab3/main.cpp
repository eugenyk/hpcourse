#define __CL_ENABLE_EXCEPTIONS

#include <OpenCL/cl.h>
#include "cl.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>

void read_data(size_t &n, size_t &m, std::vector<float> &a, std::vector<float> &b, const char *file) {
    std::ifstream ifile(file);
    ifile >> n >> m;
    size_t size_a = n * n;
    size_t size_b = m * m;

    for (int i = 0; i < size_a; ++i) {
        float tmp;
        ifile >> tmp;
        a.push_back(tmp);
    }

    for (int i = 0; i < size_b; ++i) {
        float tmp;
        ifile >> tmp;
        b.push_back(tmp);
    }

    ifile.close();
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Function should accept the file" << std::endl;
        return 1;
    }

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
        cl::CommandQueue queue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

        // load opencl source
        std::ifstream cl_file("convolution.cl");
        std::string cl_string(std::istreambuf_iterator<char>(cl_file), (std::istreambuf_iterator<char>()));
        cl::Program::Sources source(1, std::make_pair(cl_string.c_str(),
                                                      cl_string.length() + 1));

        // create program
        cl::Program program(context, source);

        // compile opencl source
        try {
            program.build(devices);
        }
        catch (cl::Error const &e) {
            std::string log_str = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
            std::cout << std::endl << e.what() << " : " << e.err() << std::endl;
            std::cout << log_str;
            return 0;
        }

        // create a message to send to kernel
        size_t const block_size = 16;
        size_t const max_n = 1024;
        size_t const max_m = 9;

        size_t n, m;
        std::vector<float> a;
        std::vector<float> b;
        read_data(n, m, a, b, argv[1]);
        std::vector<float> c(a.size());


        // allocate device buffer to hold message
        cl::Buffer dev_a(context, CL_MEM_READ_ONLY, sizeof(float) * max_n * max_n);
        cl::Buffer dev_b(context, CL_MEM_READ_ONLY, sizeof(float) * max_m * max_m);
        cl::Buffer dev_c(context, CL_MEM_WRITE_ONLY, sizeof(float) * max_n * max_n);

        // copy from cpu to gpu
        queue.enqueueWriteBuffer(dev_a, CL_TRUE, 0, sizeof(float) * a.size(), &a[0]);
        queue.enqueueWriteBuffer(dev_b, CL_TRUE, 0, sizeof(float) * b.size(), &b[0]);

        auto const workers_count = (size_t) ((n / block_size) + 1) * block_size;
        auto const hm = (size_t) ((m - 1) / 2);

        // load named kernel from opencl source
        cl::Kernel kernel_gmem(program, "convolution");
        kernel_gmem.setArg(0, dev_a);
        kernel_gmem.setArg(1, dev_b);
        kernel_gmem.setArg(2, dev_c);
        kernel_gmem.setArg(3, static_cast<int>(n));
        kernel_gmem.setArg(4, static_cast<int>(m));
        kernel_gmem.setArg(5, static_cast<int>(hm));

        queue.enqueueNDRangeKernel(kernel_gmem, cl::NullRange, cl::NDRange(workers_count, workers_count),
                                   cl::NDRange(block_size, block_size));

        queue.enqueueReadBuffer(dev_c, CL_TRUE, 0, sizeof(int) * c.size(), &c[0]);

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                std::cout << c[i * n + j] << " ";
            }
            std::cout << std::endl;
        }
    } catch (cl::Error const &e) {
        std::cout << std::endl << e.what() << " : " << e.err() << std::endl;
    }

    return 0;
}