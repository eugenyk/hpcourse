#define __CL_ENABLE_EXCEPTIONS

#include <OpenCL/cl.h>
#include "cl.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>

size_t const block_size = 256;

std::vector<float> hillis_steele(std::vector<float> &input);

std::vector<float> merge(std::vector<float> a, std::vector<float> b, std::vector<float> const &intermediate_res,
                         cl::Buffer dev_b, cl::Context context, cl::CommandQueue queue, cl::Program program);

void vector_align(std::vector<float> &a, size_t val) {
    while (a.size() % val != 0) {
        a.push_back(float());
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Function should accept the file" << std::endl;
        return 1;
    }
    auto input_file = argv[1];
    std::ifstream ifstr{input_file};

    size_t n;
    ifstr >> n;
    std::vector<float> a(n);
    for (size_t i = 0; i < n; ++i) {
        ifstr >> a[i];
    }
    auto initial_size = a.size();
    auto result = hillis_steele(a);

    for (size_t i = 0; i < initial_size; ++i) {
        std::cout << std::fixed << std::setprecision(3) << result[i] << " ";
    }
    std::cout << std::endl;
    return 0;
}

std::vector<float> hillis_steele(std::vector<float> &input) {
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;
    std::vector<cl::Kernel> kernels;

    try {
        vector_align(input, block_size);

        cl::Platform::get(&platforms);
        platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);

        // create context
        cl::Context context(devices);

        // create command queue
        cl::CommandQueue queue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

        // load opencl source
        std::ifstream cl_file("scan.cl");
        std::string cl_string(std::istreambuf_iterator<char>(cl_file), (std::istreambuf_iterator<char>()));
        cl::Program::Sources source(1, std::make_pair(cl_string.c_str(),
                                                      cl_string.length() + 1));


        // create program
        cl::Program program(context, source);

        // compile opencl source
        program.build(devices);

        std::vector<float> a(input);
        std::vector<float> b(a.size());

        // allocate device buffer to hold message
        cl::Buffer dev_a(context, CL_MEM_READ_ONLY, sizeof(float) * a.size());
        cl::Buffer dev_b(context, CL_MEM_WRITE_ONLY, sizeof(float) * b.size());

        // copy from cpu to gpu
        queue.enqueueWriteBuffer(dev_a, CL_TRUE, 0, sizeof(float) * a.size(), &a[0]);

        queue.finish();

        // load named kernel from opencl source
        cl::Kernel kernel_hillis_steele(program, "scan_hillis_steele");
        cl::KernelFunctor scan_hillis_steele(kernel_hillis_steele, queue, cl::NullRange, cl::NDRange(a.size()),
                                             cl::NDRange(block_size));

        // call kernel function
        cl::Event event = scan_hillis_steele(dev_a, dev_b, cl::__local(sizeof(float) * block_size),
                                             cl::__local(sizeof(float) * block_size));

        event.wait();
        queue.enqueueReadBuffer(dev_b, CL_TRUE, 0, sizeof(float) * b.size(), &b[0]);

        if (a.size() <= block_size) {
            return b;
        }

        std::vector<float> group_sums;
        for (size_t i = 0; i < a.size() / block_size; ++i) {
            group_sums.push_back(b[(i + 1) * block_size - 1]);
        }

        auto intermediate_res = hillis_steele(group_sums);
        return merge(a, b, intermediate_res, dev_b, context, queue, program);

    } catch (cl::Error e) {
        throw std::runtime_error(std::string(e.what()) + " : " + std::to_string(e.err()));
    }
}

std::vector<float> merge(std::vector<float> a, std::vector<float> b, std::vector<float> const &intermediate_res,
                         cl::Buffer dev_b, cl::Context context, cl::CommandQueue queue, cl::Program program) {


    cl::Buffer dev_a(context, CL_MEM_READ_ONLY, sizeof(float) * a.size());
    cl::Buffer dev_gs(context, CL_MEM_READ_ONLY, sizeof(float) * intermediate_res.size());

    queue.enqueueWriteBuffer(dev_a, CL_TRUE, 0, sizeof(float) * a.size(), &b[0]);
    queue.enqueueWriteBuffer(dev_gs, CL_TRUE, 0, sizeof(float) * intermediate_res.size(),
                             &intermediate_res[0]);
    queue.finish();

    cl::Kernel kernel_merge(program, "scan_merge");
    cl::KernelFunctor scan_merge(kernel_merge, queue, cl::NullRange, cl::NDRange(a.size()),
                                 cl::NDRange(block_size));

    auto event = scan_merge(dev_a, dev_gs, dev_b);
    event.wait();

    queue.enqueueReadBuffer(dev_b, CL_TRUE, 0, sizeof(float) * a.size(), &b[0]);

    return b;
}