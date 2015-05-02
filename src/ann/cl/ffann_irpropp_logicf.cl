
//Feedforward Artificial Neural Network
//Learning algorithm: iRProp+
//Activation function: Logical (1/(1+e^-x))

void atomic_add_float(volatile global float* source, const float valuetoadd)
{
    do {
        local float before_add = *source;
        local float new_val = before_add + valuetoadd;
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
// w - l x n x n array, w[l] = weights of connections incomming to nodes in l-th layer, w[l][i][j] = connection to i-th node in layer l from j-th node in prev layer, w[0] is ignored
// dE - l x n x n array, dE[l][i][j] is error derivate in respect to w[l][i][j]
// pdE - l x n x n array, dE from previous learning step
// d = l x n array, d[l][i] is partial error derivate computed for neuron i, used to calculate dE[l][i][j] (for every neuron j from previous layer)
// delta - l x n x n array, delta[l][i][j] is value used by iRProp+ to compute deltaw[l][i][j]
// deltaw - l x n x n array, deltaw[l][i][j] is value used by iRProp+ to update w[l][i][j]
// error - mean square error value from current learning step
// preverror - error from previous learning step
// learn_rate_pos - learning rate used by iRProp+ when dE i pdE have same sign
// learn_rate_neg - learning rate used by iRProp+ when dE i pdE have oposite sign

kernel void run(global float** outs, global float*** w, global int layers, global int* layers_sizes)
{
    local int myneuron = get_global_id(0);
    local int prevneuron = get_global_id(1);

    for (int lid=1; i<layers; ++i)
    {
        //calculate x as sum of previous outs
        local float tmp = outs[lid-1][prevneuron] * w[lid][myneuron][prevneuron];
        atomic_add_float(&outs[lid][myneuron], tmp);

        barrier(CLK_GLOBAL_FENCE);

        //apply activation func
        if (myconnection == 0)
            outs[lid][myneuron] = 1.0/exp(-outs[lid][myneuron]);

        barrier(CLK_GLOBAL_MEM_FENCE);
    }
}

kernel void calculate_derivatives(global float*** dE, float** d, global float** outs, global int layers, float* t)
{
    local int myneuron = get_global_id(0);
    local int nextneuron = get_global_id(1);

    if (nextneuron == 0)
        d[layers-1][myneuron] = (outs[layers-1][myneuron] - t[myneuron])*outs[layers-1][myneuron]*(1-outs[layers-1][myneuron]);

    barrier(CLK_GLOBAL_MEM_FENCE);

    for (int lid=layers-2; lid>= 0; ++lid)
    {
        float tmp = d[lid+1][nextneuron] * w[lid+1][nextneuron][myneuron];
        atomic_add_float(&d[lid][myneuron], tmp);
        barrier(CLK_GLOBAL_MEM_FENCE);
    }

    for (int lid=1; lid<layers-1; ++lid)
    {
        dE[lid][nextneuron][myneuron] = d[lid][nextneuron] * outs[lid-1][myneuron];
        barrier(CLK_GLOBAL_MEM_FENCE);
    }
}

kernel void apply_irpropp(global float*** w, global float*** dE, float*** pdE, global float*** delta, float*** deltaw, global int layers, global float error, global float preverror, global float learn_rate_pos, global float learn_rate_neg)
{
    local int myneuron = get_global_id(0);
    local int prevneuron = get_global_id(0);

    for (int lid=1; lid<layers; ++lid)
    {
        if (pdE[lid][myneuron][prevneuron]*dE[lid][myneuron][prevneuron] > 0)
        {
            delta[lid][myneuron][prevneuron] = min(learn_rate_pos*delta[lid][myneuron][prevneuron], delta_max);
            deltaw[lid][myneuron][prevneuron] = -sign(dE[lid][myneuron][prevneuron])*delta[lid][myneuron][prevneuron];
            w[lid][myneuron][prevneuron] += deltaw[lid][myneuron][prevneuron];
        }
        else if (pdE[lid][myneuron][prevneuron]*dE[lid][myneuron][prevneuron] < 0)
        {
            delta[lid][myneuron][prevneuron] = max(learn_rate_neg*delta[lid][myneuron][prevneuron], delta_min);
            if (error > preverror)
                w[lid][myneuron][prevneuron] -= deltaw[lid][myneuron][prevneuron];
            dE[lid][myneuron][prevneuron] = 0;
        }
        else
        {
            deltaw[lid][myneuron][prevneuron] = -sign(dE[lid][myneuron][prevneuron])*delta[lid][myneuron][prevneuron];
            w[lid][myneuron][prevneuron] += deltaw[lid][myneuron][prevneuron];
        }
    }
}
