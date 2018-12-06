#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include <CL/cl.h>
#include <CL/cl2.hpp> // using cl2 because cl was missing on my docker image

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>

#include <cassert>
#include <cmath>

using num_t = float;

size_t const block_size = 16;
size_t const MAX_N = 1024;
size_t const MAX_M = 9;

size_t closest_work_size(size_t x) {
    return std::ceil(x / static_cast<double>(max_block_size)) * max_block_size;
}

struct InputData {
    size_t N;
    size_t M;
    std::vector<num_t> A;
    std::vector<num_t> B;
};

InputData read_input(std::string const& filename) {
    InputData result;
    std::ifstream file(filename);
    file >> result.N;
    file >> result.M;

    assert(result.N > 0 && result.N <= MAX_N);
    assert(result.M > 0 && result.M <= MAX_M);

    for (size_t i = 0; i < result.N * result.N; ++i) {
        num_t read;
        file >> read;
        result.A.push_back(read);
    }

    for (size_t i = 0; i < result.M * result.M; ++i) {
        num_t read;
        file >> read;
        result.B.push_back(read);
    }

    return result;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "First argument should be a file with input matrices." << std::endl;
        return 1;
    }

    auto input_file_name = argv[1];

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
        std::ifstream cl_file("matrix_conv.cl");
        std::string cl_string(std::istreambuf_iterator < char > {cl_file}, std::istreambuf_iterator < char > {});
        cl::Program::Sources source = {cl_string};

        // create program
        cl::Program program(context, source);

        // compile opencl source
        try {
            program.build(devices);
        }
        catch (cl::Error const& e) {
            std::string log_str = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
            std::cout << std::endl << e.what() << " : " << e.err() << std::endl;
            std::cout << log_str;
            return 0;
        }

        // create a message to send to kernel
        auto input = read_input(input_file_name);

        // allocate device buffer to hold message
        cl::Buffer dev_a(context, CL_MEM_READ_ONLY, sizeof(num_t) * MAX_N * MAX_N);
        cl::Buffer dev_b(context, CL_MEM_READ_ONLY, sizeof(num_t) * MAX_M * MAX_M);
        cl::Buffer dev_c(context, CL_MEM_WRITE_ONLY, sizeof(num_t) * MAX_N * MAX_N);

        std::vector<num_t> C(MAX_N * MAX_N);

//      // copy from cpu to gpu
        queue.enqueueWriteBuffer(dev_a, CL_TRUE, 0, sizeof(num_t) * input.A.size(), input.A.data());
        queue.enqueueWriteBuffer(dev_b, CL_TRUE, 0, sizeof(num_t) * input.B.size(), input.B.data());

        // load named kernel from opencl source
        cl::Kernel kernel_gmem(program, "matrix_conv");
        kernel_gmem.setArg(0, dev_a);
        kernel_gmem.setArg(1, dev_b);
        kernel_gmem.setArg(2, dev_c);
        kernel_gmem.setArg(3, static_cast<int>(input.N));
        kernel_gmem.setArg(4, static_cast<int>(input.M));

        auto work_size = closest_work_size(input.N);

        queue.enqueueNDRangeKernel(
            kernel_gmem,
            cl::NullRange,
            cl::NDRange(work_size, work_size),
            cl::NDRange(max_block_size, max_block_size)
        );

        queue.enqueueReadBuffer(dev_c, CL_TRUE, 0, sizeof(num_t) * C.size(), C.data());

        for (size_t i = 0; i < input.N; ++i) {
            for (size_t j = 0; j < input.N; ++j) {
                std::cout << C[i * input.N + j] << " ";
            }

            std::cout << std::endl;
        }
    } catch (cl::Error const& e) {
        std::cout << std::endl << e.what() << " : " << e.err() << std::endl;
    }

    return 0;
}
