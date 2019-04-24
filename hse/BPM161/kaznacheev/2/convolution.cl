__kernel void convolve(__global double * a, __global double * b, __global double * res, int m, int n)
{
   int i = get_global_id(0);
   int col = get_global_id(1);
   int hm = (m - 1) / 2;
   if (i < n && col < n) {
      double ans = 0;
      for (int j = -hm; j <= hm; j++) {
         for (int k = -hm; k <= hm; k++) {
            if (i + j >= 0 && 
                i + j < n && 
                col + k >= 0 && 
                col + k < n) {
               ans += a[(i + j) * n + col + k] * b[(hm + j) * m + hm + k];
            }
         }
      }
      res[i * n + col] = ans;
   }
}