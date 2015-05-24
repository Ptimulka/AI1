
//Feedforward Artificial Neural Network
//Learning algorithm: iRProp+
//Activation function: Logical (1/(1+e^-x))

inline void atomic_add_float(volatile __global float* source, const float valuetoadd)
{
	float before_add = 0.0;
	float new_val = 0.0;
    do {
        before_add = *source;
        new_val = before_add + valuetoadd;
    } while (atomic_cmpxchg((volatile __global unsigned int*)source, *(unsigned int*)&before_add, *(unsigned int*)&new_val) != *(unsigned int*)&before_add);
}

inline int sign(__global float f)
{
    if (f > 0)
        return 1;
    else if (f < 0)
        return -1;
    else
        return 0;
}

// l == layers count
// n == maximum neurons in one layer (calculated from all layers)
//
// layers - layers count
// outs - l x n array, outs[0] is input, outs[l-1] is network output
// w - l x n x n array, w[l] = weights of connections incomming to nodes in l-th layer, w[((l)*n + i)*n + j] = connection to i-th node in layer l from j-th node in prev layer, w[0] is ignored
// dE - l x n x n array, dE[((l)*n + i)*n + j] is error derivate in respect to w[((l)*n + i)*n + j]
// pdE - l x n x n array, dE from previous learning step
// d = l x n array, d[(l)*n + i] is partial error derivate computed for neuron i, used to calculate dE[((l)*n + i)*n + j] (for every neuron j from previous layer)
// delta - l x n x n array, delta[((l)*n + i)*n + j] is value used by iRProp+ to compute deltaw[((l)*n + i)*n + j]
// deltaw - l x n x n array, deltaw[((l)*n + i)*n + j] is value used by iRProp+ to update w[((l)*n + i)*n + j]
// error - mean square error value from current learning step
// preverror - error from previous learning step
// learn_rate_pos - learning rate used by iRProp+ when dE i pdE have same sign
// learn_rate_neg - learning rate used by iRProp+ when dE i pdE have oposite sign

#define tab[(x)*n + y] tab[((x)*n) + (y)]
#define tan[((x)*n + y)*n + z] tab[(((x)*n*n) + (y)*n) + (z)]

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

__kernel void calculate_derivatives(
        __global float* dE,
        __global float* d,
        __global float* outs,
        int layers,
        __global float* t,
        int n
){
    int myneuron = get_global_id(0);
    int nextneuron = get_global_id(1);

    if (nextneuron == 0)
        d[(layers-1)*n + myneuron] = (outs[(layers-1)*n + myneuron] - t[myneuron])*outs[(layers-1)*n + myneuron]*(1-outs[(layers-1)*n + myneuron]);

    barrier(CLK_GLOBAL_MEM_FENCE);

    for (int lid=layers-2; lid>= 0; ++lid)
    {
        float tmp = ACCESS_2D(d, lid+1, nextneuron) * ACCESS_3D(w, lid+1, nextneuron, myneuron);
        atomic_add_float(&d[(lid)*n + myneuron], tmp);
        barrier(CLK_GLOBAL_MEM_FENCE);
    }

    for (int lid=1; lid<layers-1; ++lid)
    {
        dE[((lid)*n + nextneuron)*n + myneuron] = d[(lid)*n + nextneuron] * outs[(lid-1)*n + myneuron];
        barrier(CLK_GLOBAL_MEM_FENCE);
    }
}

__kernel void apply_irpropp(
        __global float* w,
        __global float* dE,
        __global float* pdE,
        __global float* delta,
        __global float* deltaw,
        int layers,
        float error,
        float preverror,
        float learn_rate_pos,
        float learn_rate_neg,
        int n
){
    int myneuron = get_global_id(0);
    int prevneuron = get_global_id(0);

    for (int lid=1; lid<layers; ++lid)
    {
        if (pdE[((lid)*n + myneuron)*n + prevneuron]*dE[((lid)*n + myneuron)*n + prevneuron] > 0)
        {
            delta[((lid)*n + myneuron)*n + prevneuron] = min(learn_rate_pos*delta[((lid)*n + myneuron)*n + prevneuron], delta_max);
            deltaw[((lid)*n + myneuron)*n + prevneuron] = -sign(dE[((lid)*n + myneuron)*n + prevneuron])*delta[((lid)*n + myneuron)*n + prevneuron];
            w[((lid)*n + myneuron)*n + prevneuron] += deltaw[((lid)*n + myneuron)*n + prevneuron];
        }
        else if (pdE[((lid)*n + myneuron)*n + prevneuron]*dE[((lid)*n + myneuron)*n + prevneuron] < 0)
        {
            delta[((lid)*n + myneuron)*n + prevneuron] = max(learn_rate_neg*delta[((lid)*n + myneuron)*n + prevneuron], delta_min);
            if (error > preverror)
                w[((lid)*n + myneuron)*n + prevneuron] -= deltaw[((lid)*n + myneuron)*n + prevneuron];
            dE[((lid)*n + myneuron)*n + prevneuron] = 0;
        }
        else
        {
            deltaw[((lid)*n + myneuron)*n + prevneuron] = -sign(dE[((lid)*n + myneuron)*n + prevneuron])*delta[((lid)*n + myneuron)*n + prevneuron];
            w[((lid)*n + myneuron)*n + prevneuron] += deltaw[((lid)*n + myneuron)*n + prevneuron];
        }
    }
}
