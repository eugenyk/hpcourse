#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.h>
#include "cl.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <iterator>

void read_matrix(std::ifstream& input_stream, double* matrix, int N)
{
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            int idx = i * N + j;
            input_stream >> matrix[idx];
        }
    }
}

size_t round_n(int n, int block_size)
{
    return ((size_t)(n + block_size - 1) / block_size) * block_size;
}

int main()
{
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
        size_t const block_size = 16;
        try
        {
            program.build(devices, "-DBLOCK_SIZE=16");
        }
        catch (cl::Error const & e)
        {
            std::string log_str = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
            std::cout << std::endl << e.what() << " : " << e.err() << std::endl;
            std::cout << log_str;
            return 0;
        }

        // create a message to send to kernel
        std::ifstream input_stream("input.txt");
        int N, M;
        input_stream >> N >> M;

        double a[N * N];
        double b[M * M];
        double c[N * N];
        int a_size = N * N;
        int b_size = M * M;
        int c_size = N * N;
        read_matrix(input_stream, a, N);
        read_matrix(input_stream, b, M);

        // allocate device buffer to hold message
        cl::Buffer dev_a(context, CL_MEM_READ_ONLY,  sizeof(double) * a_size);
        cl::Buffer dev_b(context, CL_MEM_READ_ONLY,  sizeof(double) * b_size);
        cl::Buffer dev_c(context, CL_MEM_WRITE_ONLY, sizeof(double) * c_size);

        // copy from cpu to gpu
        queue.enqueueWriteBuffer(dev_a, CL_TRUE, 0, sizeof(double) * a_size, a);
        queue.enqueueWriteBuffer(dev_b, CL_TRUE, 0, sizeof(double) * b_size, b);

        // load named kernel from opencl source
        size_t range = round_n(N, block_size);
        cl::Kernel kernel(program, "convolution");
        cl::KernelFunctor convolution(kernel, queue, cl::NullRange,
                cl::NDRange(range, range), cl::NDRange(block_size, block_size));
        convolution(dev_a, dev_b, dev_c, N, M);

        queue.enqueueReadBuffer(dev_c, CL_TRUE, 0, sizeof(double) * c_size, c);

        std::ofstream output_stream("output.txt");
        output_stream << std::fixed << std::setprecision(3);
        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                size_t idx = i * N + j;
                output_stream << c[idx] << " ";
            }
            output_stream << std::endl;
        }
        output_stream.close();
    }
    catch (cl::Error& e)
    {
        std::cout << std::endl << e.what() << " : " << e.err() << std::endl;
    }

    return 0;
}
