__kernel void matrix_conv(__global float *A, __global float *B, __global float *C, const int N, const int M, const int HM) {
    
    // Get the index of the current element
    int i = get_global_id(0);
    int j = get_global_id(1);
	
	float result = 0.0f;
	
	if ( i >= N  || j >= N)
        return;
	


    for (int k = -HM; k <= HM; ++k) {
      for (int l = -HM; l <= HM; ++l) {
         if (!(i + k  < 0) && !(j + l < 0) && !(j + l >= N) && !(i + k >= N)) {
            result += A[(i + k) * N + (j + l)] * B[(k + HM) * M + (l + HM)];
			
         } 
      }
    }
	

    // Do the operation

	C[i * N + j] = result;
}
