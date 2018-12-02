float get_with_default(global float* matrix, int row, int col, int N) {
    if (0 <= row && row < N && 0 <= col && col < N) {
        return matrix[row * N + col];
    }

    return 0;
}


kernel void matrix_conv(global float* input, global float* mask, global float* output, int N, int M) {
    int row = get_global_id(0);
    int col = get_global_id(1);

    if (row >= N || col >= N)
        return;

    float res = 0;

    for (int mask_i = 0; mask_i < M; ++mask_i) {
        for (int mask_j = 0; mask_j < M; ++mask_j) {
            int input_row = (row + mask_i - M / 2);
            int input_col = (col + mask_j - M / 2);
            int input_idx = input_row * N + input_col;
            res += get_with_default(input, input_row, input_col, N) * mask[mask_i * M + mask_j];
        }
    }

    output[row * N + col] = res;
}