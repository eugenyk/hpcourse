#define num_t float
#define SWAP(a, b) {__local num_t * tmp=a; a=b; b=tmp;}

__kernel void scan_hillis_steele(__global num_t* input, __global num_t* output, __local num_t* a, __local num_t* b) {
    uint gid = get_global_id(0);
    uint lid = get_local_id(0);
    uint block_size = get_local_size(0);

    a[lid] = b[lid] = input[gid];
    barrier(CLK_LOCAL_MEM_FENCE);

    for (uint s = 1; s < block_size; s <<= 1) {
        if (lid > (s - 1)) {
            b[lid] = a[lid] + a[lid - s];
        } else {
            b[lid] = a[lid];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        SWAP(a, b);
    }
    output[gid] = a[lid];
}

__kernel void aggregate_sums(__global num_t* input, __global num_t* group_sums, __global num_t* output) {
    uint lid = get_local_id(0);
    uint grid = get_group_id(0);

    uint block_size = get_local_size(0);

    if (grid > 0) {
        output[lid + grid * block_size] = input[lid + grid * block_size] + group_sums[grid - 1];
    }
}