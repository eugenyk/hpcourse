__kernel void convolution(__global double * A, __global double * B, __global double * C, int N, int M)
{
   int id = get_global_id(0);
   if (id >= N * N) {
       return;
   }
   int HM = (M - 1) / 2;
   int i = id / N, j = id % N;
   C[id] = 0;
   for (int k = -HM; k <= HM; k++) {
       for (int l = -HM; l <= HM; l++) {
           if (i + k < 0 || j + l < 0 || i + k >= N || j + l >= N) {
               continue;
           }
           C[i * N + j] += A[(i + k) * N + j + l] * B[(k + HM) * M + l + HM];
       }
   }
}