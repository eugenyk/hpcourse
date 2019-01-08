__kernel void gpu_convolution_gmem(__global float * input, __global float * mask, 
                                   __global float * output, int m, int n, int HM)
{
   int i = get_global_id(0);
   int j = get_global_id(1);

   if (i >= n || j >= n || i < 0 || j < 0) {
      return;
   } 

   float res = 0;
   
   for (int k = -HM; k <= HM; ++k) {
      for (int l = -HM; l <= HM; ++l) {
         if (!(i + k  < 0) && !(i + k >= n) && !(j + l < 0) && !(j + l >= n)) {
            res += input[(i + k) * n + (j + l)] * mask[(k + HM) * m + (l + HM)];
         } 
      }
   }

   output[i * n + j] = res;
}