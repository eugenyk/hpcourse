__kernel void matrix_mult(__global int * a, __global int * b, __global int * c, int n, int m)
{
   int row = get_global_id(0);
   int col = get_global_id(1);

   if (row >= n || col >= n)
      return;

   int res = 0;
   for (int k = 0; k < m; ++k)
   {
      for (int l = 0; l < m; ++l)
      {
         int row_a = row + k - m / 2;
         int col_a = col + l - m / 2;

         if (row_a >= 0 && row_a < n && col_a >= 0 && col_a < n)
             res += a[row_a * n + col_a] * b[k * m + l];
      }
   }

   c[row * n + col] = res;
}