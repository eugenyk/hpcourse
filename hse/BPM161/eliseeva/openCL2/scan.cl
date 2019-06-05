#define SWAP(a,b) {__local double * tmp=a; a=b; b=tmp;}

__kernel void scan_hillis_steele(__global double * input, __global double * output, __local double * a, __local double * b, int N)
{
    uint gid = get_global_id(0);
    uint lid = get_local_id(0);
    uint block_size = get_local_size(0);
 
    if (gid < N)
    {
        a[lid] = b[lid] = input[gid];
    }
    barrier(CLK_LOCAL_MEM_FENCE);
 
    for(uint s = 1; s < block_size; s <<= 1)
    {
        if(lid > (s-1))
        {
            b[lid] = a[lid] + a[lid-s];
        }
        else
        {
            b[lid] = a[lid];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        SWAP(a,b);
    }
    if (gid < N)
    {
        output[gid] = a[lid];
    }
}

__kernel void copy_sums(__global double *input, __global double *output, int N, int M) {
    uint gid = get_global_id(0);
    uint block_size = get_local_size(0);
    uint index = gid / block_size + 1;

    if (gid < N && index < M && gid == index * block_size - 1) {
        output[index] = input[gid];
    }
}

__kernel void add_sums(__global double *copy_sums, __global double *input, __global double *output, int N) {
    uint gid = get_global_id(0);
    uint block_size = get_local_size(0);
    uint index = gid / block_size;

    if (gid < N) {
        output[gid] = input[gid] + copy_sums[index];
    }
}
