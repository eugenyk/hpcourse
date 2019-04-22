__kernel void convolution(__global double * a, __global double * b, __global double * c, int n, int m)
{
    int hm = (m - 1) / 2;
    int row = get_global_id(0);
    int col = get_global_id(1);

    if (row >= n || col >= n)
        return;

    double sum = 0;

    for (int k = -hm; k <= hm; ++k)
        for (int l = -hm; l <= hm; ++l) {
            int i = row + k;
            int j = col + l;
            if (i < 0 || i >= n || j < 0 || j >= n)
                continue;
            sum += a[i * n + j] * b[(k + hm) * m + l + hm];
        }


    c[row * n + col] = sum;
}

