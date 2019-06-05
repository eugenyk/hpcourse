#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.h>
#include "cl.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <assert.h>

void copy_sums(std::vector<double> &input, std::vector<double> &output, size_t N, size_t block_size, size_t global_size,
   cl::Context &context, cl::Program &program, cl::CommandQueue &queue) {
      // allocate device buffer to hold message
      cl::Buffer dev_input (context, CL_MEM_READ_ONLY, sizeof(double) * N);
      cl::Buffer dev_output(context, CL_MEM_WRITE_ONLY, sizeof(double) * global_size / block_size);

      // copy from cpu to gpu
      queue.enqueueWriteBuffer(dev_input, CL_TRUE, 0, sizeof(double) * N, &input[0]);

      // load named kernel from opencl source
      cl::Kernel kernel_hs(program, "copy_sums");
      cl::KernelFunctor scan_hs(kernel_hs, queue, cl::NullRange, cl::NDRange(global_size), cl::NDRange(block_size));
      scan_hs(dev_input, dev_output, (int)N, (int)(global_size / block_size));

      queue.enqueueReadBuffer(dev_output, CL_TRUE, 0, sizeof(double) * global_size/ block_size, &output[0]);
      output[0] = 0;
}

void add_sums(std::vector<double> &p_input, std::vector<double> &input, std::vector<double> &output, size_t N, size_t block_size, size_t global_size,
   cl::Context &context, cl::Program &program, cl::CommandQueue &queue) {
      // allocate device buffer to hold message
      cl::Buffer dev_p_input (context, CL_MEM_READ_ONLY, sizeof(double) * global_size / block_size);
      cl::Buffer dev_input (context, CL_MEM_READ_ONLY, sizeof(double) * N);
      cl::Buffer dev_output(context, CL_MEM_WRITE_ONLY, sizeof(double) * N);

      // copy from cpu to gpu
      queue.enqueueWriteBuffer(dev_p_input, CL_TRUE, 0, sizeof(double) * global_size / block_size, &p_input[0]);
      queue.enqueueWriteBuffer(dev_input, CL_TRUE, 0, sizeof(double) * N, &input[0]);

      // load named kernel from opencl source
      cl::Kernel kernel_hs(program, "add_sums");
      cl::KernelFunctor scan_hs(kernel_hs, queue, cl::NullRange, cl::NDRange(global_size), cl::NDRange(block_size));
      scan_hs(dev_p_input, dev_input, dev_output, (int)N);

      queue.enqueueReadBuffer(dev_output, CL_TRUE, 0, sizeof(double) * N, &output[0]);
}


void scan_hillis_steele(std::vector<double> &input, std::vector<double> &output, size_t N, size_t block_size, size_t global_size,
   cl::Context &context, cl::Program &program, cl::CommandQueue &queue) {
      // allocate device buffer to hold message
      cl::Buffer dev_input (context, CL_MEM_READ_ONLY, sizeof(double) * N);
      cl::Buffer dev_output(context, CL_MEM_WRITE_ONLY, sizeof(double) * N);

      // copy from cpu to gpu
      queue.enqueueWriteBuffer(dev_input, CL_TRUE, 0, sizeof(double) * N, &input[0]);

      // load named kernel from opencl source
      cl::Kernel kernel_hs(program, "scan_hillis_steele");
      cl::KernelFunctor scan_hs(kernel_hs, queue, cl::NullRange, cl::NDRange(global_size), cl::NDRange(block_size));
      scan_hs(dev_input, dev_output, cl::__local(sizeof(double) * block_size), cl::__local(sizeof(double) * block_size), (int)N);

      queue.enqueueReadBuffer(dev_output, CL_TRUE, 0, sizeof(double) * N, &output[0]);
}


int main()
{
   std::vector<cl::Platform> platforms;
   std::vector<cl::Device> devices;
   std::vector<cl::Kernel> kernels;

    std::ifstream fin("input.txt");
    std::ofstream fout("output.txt");

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

      // create a message to send to kernel
      size_t const block_size = 256;

      size_t N;
      fin >> N;

      std::vector<double> input(N);
      std::vector<double> output(N, 0);

      for (size_t i = 0; i < N; i++) {
        fin >> input[i];
      }
      size_t global_size = N;
      if (global_size % block_size != 0) {
        global_size = (N + block_size - 1) / block_size * block_size;
      }
      
      scan_hillis_steele(input, output, N, block_size, global_size, context, program, queue);
      

      if (N > block_size) {
        std::vector<double> _input = input;
        input = output;
        output.clear();
        output.resize(global_size / block_size, 0);

        copy_sums(input, output, N, block_size, global_size, context, program, queue);
        
        input = output;
        output.clear();
        output.resize(global_size / block_size, 0);
        
        scan_hillis_steele(input, output, N, block_size, global_size, context, program, queue);
        
        input = output;
        output.clear();
        output.resize(N, 0);

        add_sums(input, _input, output, N, block_size, global_size, context, program, queue);
      }

      fout << std::setprecision(3) << std::fixed;
      for (size_t i = 0; i < N; i++) {
         fout << output[i] << ' ';
      }
   }
   catch (cl::Error e)
   {
      fout << std::endl << e.what() << " : " << e.err() << std::endl;
   }

   return 0;
}