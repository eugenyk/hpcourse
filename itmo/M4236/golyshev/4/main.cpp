#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.h>
#include "cl.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <assert.h>
#include <string>

using num_t = float;

static size_t const block_size(256);

std::vector<num_t> read_input(char const* input_file) {
    std::ifstream input{input_file};

    size_t N;
    input >> N;
    std::vector<num_t> input_data(N);

    for (size_t i = 0; i < N; ++i) {
        input >> input_data[i];
    }

    return input_data;
}

void extend_to_match_size(std::vector<num_t>& input, size_t size_to_match) {
    while (input.size() % size_to_match != 0) {
        input.push_back(num_t());
    }
}

std::vector<num_t> do_work(std::vector<num_t> const& input);

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: scan <input.txt>" << std::endl;
        return 1;
    }

    auto input_file = argv[1];
    auto input_data = read_input(input_file);
    auto initial_size = input_data.size();

    extend_to_match_size(input_data, block_size);

    auto result = do_work(input_data);
    for (size_t i = 0; i < initial_size; ++i) {
        std::cout << std::fixed << std::setprecision(3) << result[i] << " ";
    }

    std::cout << std::endl;

    return 0;
}

std::vector<num_t> do_work(std::vector<num_t> const& input_data) {
    std::vector <cl::Platform> platforms;
    std::vector <cl::Device> devices;
    std::vector <cl::Kernel> kernels;

    try {

        // create platform
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

        size_t const input_size = input_data.size();
        size_t const output_size = input_size;

        std::vector<num_t> input = input_data;
        std::vector<num_t> output(output_size);

        // allocate device buffer to hold message
        cl::Buffer dev_input(context, CL_MEM_READ_ONLY, sizeof(num_t) * input_size);
        cl::Buffer dev_output(context, CL_MEM_WRITE_ONLY, sizeof(num_t) * output_size);

        // copy from cpu to gpu
        queue.enqueueWriteBuffer(dev_input, CL_TRUE, 0, sizeof(num_t) * input_size, input.data());

        queue.finish();

        // load named kernel from opencl source
        cl::Kernel kernel_hs(program, "scan_hillis_steele");
        cl::KernelFunctor scan_hs(kernel_hs, queue, cl::NullRange, cl::NDRange(input_size),
                                  cl::NDRange(block_size));

        // call kernel function
        cl::Event event = scan_hs(
            dev_input,
            dev_output,
            cl::__local(sizeof(num_t) * block_size),
            cl::__local(sizeof(num_t) * block_size)
        );

        event.wait();
        queue.enqueueReadBuffer(dev_output, CL_TRUE, 0, sizeof(num_t) * output_size, output.data());

        if (input_size <= block_size) {
            return output;
        }

        // get sum for every group
        auto group_num = input_size / block_size;
        std::vector<num_t> group_sums;
        for (size_t i = 0; i < group_num; ++i) {
            group_sums.push_back(output[(i + 1) * block_size - 1]);
        }

        // aggregate groups sums recursively
        extend_to_match_size(group_sums, block_size);
        auto aggregated_sums = do_work(group_sums);
        auto aggregated_sums_size = aggregated_sums.size();

        cl::Buffer dev_group_sum(context, CL_MEM_READ_ONLY, sizeof(num_t) * aggregated_sums_size);

        queue.enqueueWriteBuffer(dev_input, CL_TRUE, 0, sizeof(num_t) * input_size, output.data());
        queue.enqueueWriteBuffer(dev_group_sum, CL_TRUE, 0, sizeof(num_t) * aggregated_sums_size, aggregated_sums.data());
        queue.finish();

        cl::Kernel kernel_aggregate(program, "aggregate_sums");
        cl::KernelFunctor aggregate(
            kernel_aggregate,
            queue,
            cl::NullRange,
            cl::NDRange(input_size),
            cl::NDRange(block_size)
        );

        auto aggr_event = aggregate(dev_input, dev_group_sum, dev_output);
        aggr_event.wait();

        queue.enqueueReadBuffer(dev_output, CL_TRUE, 0, sizeof(num_t) * input_size, output.data());

        return output;
    } catch (cl::Error e) {
        throw std::runtime_error(std::string(e.what()) + " : " + std::to_string(e.err()));
    }
}
