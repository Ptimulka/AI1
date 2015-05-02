kernel void worker(global int* a, global int* b, global int* res)
{
	local int myid = get_global_id(0);
	res[myid] = a[myid] + b[myid];
}
