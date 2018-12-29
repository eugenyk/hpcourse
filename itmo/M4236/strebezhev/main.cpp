#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include <CL/cl.h>
#include <CL/cl2.hpp>

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>

#include <cassert>
#include <cmath>


int main(int argc, char** argv) {
  std::ifstream file(argc == 2 ? argv[1] : "input.txt", std::ios_base::in);
  assert(file.good());

  // read sizes
  size_t N, N_MAX = 1024, M, M_MAX = 9;
  file >> N >> M;
  assert(1 <= N && N <= N_MAX);
  assert(1 <= M && M <= M_MAX && M & 1);
  size_t N_SIZE = M + N + M, N_MAX_SIZE = M_MAX + N_MAX + M_MAX;

  // read matrices
  std::vector<float> A(N_SIZE * N_SIZE), B(M * M);

  for (size_t row = 0; row < N; ++row)
    for (size_t col = 0; col < N; ++col)
      file >> A[(M + row) * N_SIZE + (M + col)];  // extra padding to remove branching on GPU

  for (float& idx : B) file >> idx;


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
    std::ifstream cl_file("convolution.c");
    std::string cl_string(std::istreambuf_iterator<char>{cl_file}, std::istreambuf_iterator<char>{});
    cl::Program::Sources source = {cl_string};

    // create program
    cl::Program program(context, source);

    // compile opencl source
    try {
      program.build(devices);
    } catch (cl::Error const& e) {
      std::string log_str = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
      std::cout << std::endl << e.what() << " : " << e.err() << std::endl;
      std::cout << log_str;
      return 0;
    }

    // allocate device buffer to hold message
    cl::Buffer A_DEV(context, CL_MEM_READ_ONLY, sizeof(float) * N_MAX_SIZE * N_MAX_SIZE);
    cl::Buffer B_DEV(context, CL_MEM_READ_ONLY, sizeof(float) * M_MAX * M_MAX);
    cl::Buffer C_DEV(context, CL_MEM_WRITE_ONLY, sizeof(float) * N_MAX * N_MAX);

    // copy from cpu to gpu
    queue.enqueueWriteBuffer(A_DEV, CL_TRUE, 0, sizeof(float) * A.size(), &A[0]);
    queue.enqueueWriteBuffer(B_DEV, CL_TRUE, 0, sizeof(float) * B.size(), &B[0]);

    // load named kernel from opencl source
    cl::Kernel func(program, "main");
    func.setArg(0, A_DEV);
    func.setArg(1, B_DEV);
    func.setArg(2, C_DEV);
    func.setArg(3, (int) N);
    func.setArg(4, (int) M);
    func.setArg(5, ((int) ((M - 1) / 2)));

    size_t const BLOCK = 16;
    size_t THREADS = (N / BLOCK + 1) * BLOCK;
    queue.enqueueNDRangeKernel(func, cl::NullRange, cl::NDRange(THREADS, THREADS), cl::NDRange(BLOCK, BLOCK));

    std::vector<float> C(N_MAX * N_MAX);
    queue.enqueueReadBuffer(C_DEV, CL_TRUE, 0, sizeof(float) * C.size(), &C[0]);

    for (size_t row = 0; row < N; ++row) {
      for (size_t col = 0; col < N; ++col)
        std::cout << C[row * N + col] << " ";
      std::cout << "\n";
    }
  } catch (cl::Error const& e) {
    std::cout << std::endl << e.what() << " : " << e.err();
  }

  std::cout << std::endl;
  return 0;
}
