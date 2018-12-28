#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.h>
#include "cl.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>

int main(int argc, char** argv)
{

   if (argc < 3) {
      std::cout << "You forget files\n";
   }

   using data_type = float;

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
      program.build(devices);

      // some consts
      size_t const max_n = 1024;
      size_t const max_m = 9;
      size_t const block_size = 16;


      // read input
      std::ifstream ifile(argv[1]);
      
      // read mtx and mask sizes
      size_t mtx_w_h;
      size_t mask_w_h;
      ifile >> mtx_w_h >> mask_w_h;
      
      // compute other vars and create containers
      size_t const mtx_size = mtx_w_h * mtx_w_h;
      size_t const mask_size = mask_w_h * mask_w_h;
      std::vector<data_type> mtx;
      std::vector<data_type> mask;
      std::vector<data_type> output(mtx_size);

      // continue to read input
      for (int i = 0; i < mtx_size; ++i) {
         data_type tmp;
         ifile >> tmp;
         mtx.push_back(tmp);
      }

      for (int i = 0; i < mask_size; ++i) {
         data_type tmp;
         ifile >> tmp;
         mask.push_back(tmp);
      }
      ifile.close();

      // compute other consts
      size_t const HM = (size_t) ((mask_w_h - 1) / 2);
      size_t const num_workers = (size_t)((mtx_w_h / block_size) + 1) * block_size;

      // allocate device buffer to hold message
      cl::Buffer dev_input (context, CL_MEM_READ_ONLY,  sizeof(data_type) * max_n * max_n);
      cl::Buffer dev_output(context, CL_MEM_WRITE_ONLY, sizeof(data_type) * max_n * max_n);
      cl::Buffer dev_mask  (context, CL_MEM_READ_ONLY,  sizeof(data_type) * max_m * max_m);

      // copy from cpu to gpu
      queue.enqueueWriteBuffer(dev_input, CL_TRUE, 0, sizeof(data_type) * mtx_size,  &mtx[0]);
      queue.enqueueWriteBuffer(dev_mask,  CL_TRUE, 0, sizeof(data_type) * mask_size, &mask[0]);

      // load named kernel from opencl source
      queue.finish();
      cl::Kernel kernel_gmem(program, "gpu_convolution_gmem");
      cl::KernelFunctor convolution_gmem(kernel_gmem, queue, cl::NullRange,
                                         cl::NDRange(num_workers, num_workers), cl::NDRange(block_size, block_size));
      cl::Event event = convolution_gmem(dev_input, dev_mask, dev_output,
                                         cl_int(mask_w_h), cl_int(mtx_w_h), cl_int(HM));
 
      event.wait();
      cl_ulong start_time = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
      cl_ulong end_time   = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();
      cl_ulong elapsed_time = end_time - start_time;

      queue.enqueueReadBuffer(dev_output, CL_TRUE, 0, sizeof(data_type) * mtx_size, &output[0]);
    

      // write output
      std::ofstream ofile(argv[2]);

      for (size_t i = 0; i < mtx_w_h; ++i) {
         for (size_t j = 0; j < mtx_w_h; ++j) {
            ofile << output[i * mtx_w_h + j] << " ";
            //std::cout << output[i * mtx_w_h + j] << " ";
         }
         ofile << std::endl;
      }
      
      std::cout << std::setprecision(2) << "Total time: " << elapsed_time / 1000000.0 << " ms" << std::endl;

   }
   catch (cl::Error e)
   {
      std::cout << std::endl << e.what() << " : " << e.err() << std::endl;
   }

   return 0;
}