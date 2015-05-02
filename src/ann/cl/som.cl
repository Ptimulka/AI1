
kernel void calc_diff_vectors(global float*** weight_vectors, global float* input, volatile global float** diffs)
{
    size_t x = get_global_id(0);
    size_t y = get_global_id(1);
    size_t i = get_global_id(2);

    float diff = weight_vectors[x][y][i] - input[i];
    uint trunc_diff = (uint)(diff * 10000);

    atomic_add((volatile __global uint*)&diffs[x][y], trunc_diff);
    barrier(CLK_GLOBAL_MEM_FENCE);
    if (i == 0)
        diffs[x][y] = sqrt(diffs[x][y])/100.0;       
}

kernel void find_minimum(global float** diffs, global uint2** results)
{
    size_t x_dim = get_global_size(0);
    size_t y_dim = get_global_size(1);

    size_t x = get_global_id(0);
    size_t y = get_global_id(1);
    size_t idx_1d = x*x_dim + y;

    while (idx_1d < x_dim*y_dim)
    {
        size_t x2 = (idx_1d+1)/x_dim;
        size_t y2 = (idx_1d+1) - x2*x_dim;

        float min = diffs[x][y];
        if (idx_1d+1 < x_dim*y_dim)
        {
            if (diffs[x][y] < diffs[x2][y2])
            {
                results[x/2][y/2] = uint2(x, y);
                min = diffs[x][y];
            }
            else
            {
                results[x/2][y/2] = uint2(x2, y2);
                min = diffs[x2][y2];
            }
        }

        barrier(CLK_GLOBAL_MEM_FENCE);
        diffs[x/2][y/2] = min;

        if (x_dim & 0x01 == 0)
            x_dim /= 2;
        else
            x_dim = x_dim/2 + 1;

        if (y_dim & 0x01 == 0)
            y_dim /= 2;
        else
            y_dim = y_dim/2 + 1;
    }
}
