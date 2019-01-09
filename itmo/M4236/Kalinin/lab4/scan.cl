//реализацию scan_hillis_steele подсмотрел здесь: https://tmramalho.github.io/blog/2014/06/23/parallel-programming-with-opencl-and-python-parallel-scan/

#define SWAP(a, b) {__local float * tmp=a; a=b; b=tmp;}
__kernel void scan_hillis_steele(__global float* a, __global float* r, __local float* b, __local float* c) {
    uint gid = get_global_id(0);
    uint lid = get_local_id(0);
    uint gs = get_local_size(0);

    c[lid] = b[lid] = a[gid];
    barrier(CLK_LOCAL_MEM_FENCE);

    for (uint s = 1; s < gs; s <<= 1) {
        if (lid > (s - 1)) {
            c[lid] = b[lid] + b[lid - s];
        } else {
            c[lid] = b[lid];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        SWAP(b, c);
    }
    r[gid] = b[lid];
}

__kernel void scan_merge(__global float* a, __global float* b, __global float* c) {
    uint lid = get_local_id(0);
    uint gid = get_global_id(0);
    uint block_size = get_local_size(0);
    uint bid = gid / block_size;
    c[lid + bid * block_size] = a[lid + bid * block_size] + b[bid - 1];
}