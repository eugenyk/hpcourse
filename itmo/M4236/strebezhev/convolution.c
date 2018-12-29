kernel void main(global float* A, global float* B, global float* output, int N, int M, int HM) {
  int row = get_global_id(0);
  int col = get_global_id(1);
  if (row >= N || col >= N) return;

  row += HM;
  col += HM;

  float sum = 0;
  for (int i = 0; i < M; ++i)
    for (int j = 0; j < M; ++j)
      sum += A[(row + i) * (N + M + M) + (col + j)] * B[i * M + j];

  output[(row - HM) * N + (col - HM)] = sum;
}
