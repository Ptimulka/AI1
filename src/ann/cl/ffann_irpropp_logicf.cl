
//Feedforward Artificial Neural Network
//Learning algorithm: iRProp+
//Activation function: Logical (1/(1+e^-x))

void atomic_add_float(volatile global float* source, const float valuetoadd)
{
    do {
        float before_add = *source;
        float new_val = before_add + valuetoadd;
    } while (atomic_cmpxchg((volatile global unsigned int*)source, *(unsigned int*)&before_add, *(unsigned int*)&new_val) != *(unsigned int*)&before_add);
}

int sign(global float f)
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
// w - l x n x n array, w[l] = weights of connections incomming to nodes in l-th layer, ACCESS_3D(w, l, i, j) = connection to i-th node in layer l from j-th node in prev layer, w[0] is ignored
// dE - l x n x n array, ACCESS_3D(dE, l, i, j) is error derivate in respect to ACCESS_3D(w, l, i, j)
// pdE - l x n x n array, dE from previous learning step
// d = l x n array, ACCESS_2D(d, l, i) is partial error derivate computed for neuron i, used to calculate ACCESS_3D(dE, l, i, j) (for every neuron j from previous layer)
// delta - l x n x n array, ACCESS_3D(delta, l, i, j) is value used by iRProp+ to compute ACCESS_3D(deltaw, l, i, j)
// deltaw - l x n x n array, ACCESS_3D(deltaw, l, i, j) is value used by iRProp+ to update ACCESS_3D(w, l, i, j)
// error - mean square error value from current learning step
// preverror - error from previous learning step
// learn_rate_pos - learning rate used by iRProp+ when dE i pdE have same sign
// learn_rate_neg - learning rate used by iRProp+ when dE i pdE have oposite sign

#define ACCESS_2D(tab, x, y) tab[((x)*n) + (y)]
#define ACCESS_3D(tan, x, y, z) tab[(((x)*n*n) + (y)*n) + (z)]

kernel void run(global float* outs, global float* w, global int layers, int n)
{
    int myneuron = get_global_id(0);
    int prevneuron = get_global_id(1);

    for (int lid=1; lid<layers; ++lid)
    {
        //calculate x as sum of previous outs
        float tmp = ACCESS_2D(outs, lid-1, prevneuron) * ACCESS_3D(w, lid, myneuron, prevneuron);
        atomic_add_float(&ACCESS_2D(outs, lid, myneuron), tmp);

        barrier(CLK_GLOBAL_MEM_FENCE);

        //apply activation func
        if (prevneuron == 0)
            ACCESS_2D(outs, lid, myneuron) = 1.0/exp(-ACCESS_2D(outs, lid, myneuron));

        barrier(CLK_GLOBAL_MEM_FENCE);
    }
}

kernel void calculate_derivatives(global float* dE, float* d, global float* outs, global int layers, global float* t, int n)
{
    int myneuron = get_global_id(0);
    int nextneuron = get_global_id(1);

    if (nextneuron == 0)
        ACCESS_2D(d, layers-1, myneuron) = (ACCESS_2D(outs, layers-1, myneuron) - t[myneuron])*ACCESS_2D(outs, layers-1, myneuron)*(1-ACCESS_2D(outs, layers-1, myneuron));

    barrier(CLK_GLOBAL_MEM_FENCE);

    for (int lid=layers-2; lid>= 0; ++lid)
    {
        float tmp = ACCESS_2D(d, lid+1, nextneuron) * ACCESS_3D(w, lid+1, nextneuron, myneuron);
        atomic_add_float(&ACCESS_2D(d, lid, myneuron), tmp);
        barrier(CLK_GLOBAL_MEM_FENCE);
    }

    for (int lid=1; lid<layers-1; ++lid)
    {
        ACCESS_3D(dE, lid, nextneuron, myneuron) = ACCESS_2D(d, lid, nextneuron) * ACCESS_2D(outs, lid-1, myneuron);
        barrier(CLK_GLOBAL_MEM_FENCE);
    }
}

kernel void apply_irpropp(global float* w, global float* dE, float* pdE, global float* delta, float* deltaw, global int layers, global float error, global float preverror, global float learn_rate_pos, global float learn_rate_neg, int n)
{
    int myneuron = get_global_id(0);
    int prevneuron = get_global_id(0);

    for (int lid=1; lid<layers; ++lid)
    {
        if (ACCESS_3D(pdE, lid, myneuron, prevneuron)*ACCESS_3D(dE, lid, myneuron, prevneuron) > 0)
        {
            ACCESS_3D(delta, lid, myneuron, prevneuron) = min(learn_rate_pos*ACCESS_3D(delta, lid, myneuron, prevneuron), delta_max);
            ACCESS_3D(deltaw, lid, myneuron, prevneuron) = -sign(ACCESS_3D(dE, lid, myneuron, prevneuron))*ACCESS_3D(delta, lid, myneuron, prevneuron);
            ACCESS_3D(w, lid, myneuron, prevneuron) += ACCESS_3D(deltaw, lid, myneuron, prevneuron);
        }
        else if (ACCESS_3D(pdE, lid, myneuron, prevneuron)*ACCESS_3D(dE, lid, myneuron, prevneuron) < 0)
        {
            ACCESS_3D(delta, lid, myneuron, prevneuron) = max(learn_rate_neg*ACCESS_3D(delta, lid, myneuron, prevneuron), delta_min);
            if (error > preverror)
                ACCESS_3D(w, lid, myneuron, prevneuron) -= ACCESS_3D(deltaw, lid, myneuron, prevneuron);
            ACCESS_3D(dE, lid, myneuron, prevneuron) = 0;
        }
        else
        {
            ACCESS_3D(deltaw, lid, myneuron, prevneuron) = -sign(ACCESS_3D(dE, lid, myneuron, prevneuron))*ACCESS_3D(delta, lid, myneuron, prevneuron);
            ACCESS_3D(w, lid, myneuron, prevneuron) += ACCESS_3D(deltaw, lid, myneuron, prevneuron);
        }
    }
}
