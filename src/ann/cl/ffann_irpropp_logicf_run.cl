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
		__global int* layer_sizes,
        int n
){
    int myneuron = get_global_id(0)*n + get_global_id(1);
	
	__global float* curr_outs = outs+layer_sizes[0];
	__global float* prev_outs = outs;

    for (int lid=1; lid<layers; ++lid)
    {
		if (myneuron < layer_sizes[lid])
		{
			__global float* myws = w+myneuron*layer_sizes[lid-1];
			
			//calculate x as sum of previous outs
			for (int prevneuron = 0; prevneuron < layer_sizes[lid-1]; ++prevneuron)
				curr_outs[myneuron] += prev_outs[prevneuron] * myws[prevneuron];

			curr_outs[myneuron] = 1.0/exp(-curr_outs[myneuron]);
		}
		
		prev_outs = curr_outs;
		curr_outs += layer_sizes[lid];
		w += layer_sizes[lid]*layer_sizes[lid-1];
		
        barrier(CLK_GLOBAL_MEM_FENCE);
    }
}
