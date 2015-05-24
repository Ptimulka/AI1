inline void atomic_add_float(volatile __global float* source, const float valuetoadd)
{
	float before_add = 0.0;
	float new_val = 0.0;
    do {
        before_add = *source;
        new_val = before_add + valuetoadd;
    } while (atomic_cmpxchg((volatile __global unsigned int*)source, *(unsigned int*)&before_add, *(unsigned int*)&new_val) != *(unsigned int*)&before_add);
}

__kernel void run(
        __global float* outs,
        __global float* w,
        int layers,
        int n
){
    int myneuron = get_global_id(0);
    int prevneuron = get_global_id(1);

    for (int lid=1; lid<layers; ++lid)
    {
        //calculate x as sum of previous outs
        float tmp = outs[(lid-1)*n + prevneuron] * w[(lid*n + myneuron)*n + prevneuron];
        atomic_add_float(&outs[lid*n + myneuron], tmp);

        barrier(CLK_LOCAL_MEM_FENCE);

        //apply activation func
        if (prevneuron == 0)
            outs[lid*n + myneuron] = 1.0/exp(-outs[lid*n + myneuron]);

        barrier(CLK_GLOBAL_MEM_FENCE);
    }
}
