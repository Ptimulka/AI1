#include "ann.h"
#include "utils/log.h"

#define NOMINMAX
#include "fann.h"
#include "cl/oclkernel.h"
#include "cl/oclcontext.h"
#include "cl/oclmgr.h"

#include <algorithm>
#include <CL/cl.h>
using namespace std;

struct ArtificialNeuralNetwork::FannDriver
{
    AnnDriver driver = ArtificialNeuralNetwork::FANN_DRIVER;

    fann* ann = nullptr;
    fann_train_data* data = nullptr;

    FannDriver(vector<uint> const& layers, uint training_data)
    {
        ann = fann_create_standard(layers.size(), layers.front());
        fann_set_activation_function_hidden(ann, FANN_SIGMOID);
        fann_set_activation_function_output(ann, FANN_SIGMOID);
        fann_set_activation_steepness_hidden(ann, 0.5f);
        fann_set_activation_steepness_output(ann, 0.5f);

        if (training_data)
            data = fann_create_train(training_data, layers.front(), layers.back());
        else
            data = nullptr;
    }

    ~FannDriver()
    {
        fann_destroy_train(data);
        fann_destroy(ann);

        data = nullptr;
        ann = nullptr;
    }
};

struct ArtificialNeuralNetwork::OclDriver
{
    AnnDriver driver = ArtificialNeuralNetwork::OCL_DRIVER;

    cl_int layers_count;
    cl_int workers;
    void* weights;
    void* outputs;
    void* layers;

    OclKernel* kernel;

    OclDriver(vector<uint> nodes, vector<float> const& connections)
    {
        vector<float> _outputs([&nodes]() { size_t ret = 0; for (auto const& l : nodes) ret += l; return ret; }(), 0.0f);

        layers_count = nodes.size();
        outputs = OclMgr::singleton().getDefaultContext()->createBuffer(OclContext::READ_WRITE, _outputs.size()*sizeof(_outputs.front()), _outputs.data());
        weights = OclMgr::singleton().getDefaultContext()->createBuffer(OclContext::READ, connections.size()*sizeof(connections.front()), (void*)connections.data());
        layers = OclMgr::singleton().getDefaultContext()->createBuffer(OclContext::READ, nodes.size()*sizeof(nodes.front()), nodes.data());
        
        sort(nodes.begin(), nodes.end());
        workers = nodes[1];

        kernel = new OclKernel("kernels/ffann_run_logical.cl");
    }
};

ArtificialNeuralNetwork::ArtificialNeuralNetwork(std::vector<uint> nodes_in_layers) : layers(nodes_in_layers), weights(getConnectionsCount())
{
    for (auto& w : weights)
        w = (decay<decltype(w)>::type)rand() / RAND_MAX;
}

ArtificialNeuralNetwork::ArtificialNeuralNetwork(istream & load)
{
    uint lcount;
    load.read((char*)&lcount, sizeof(lcount));
    layers.resize(lcount);
    for (auto& l : layers)
        load.read((char*)&l, sizeof(l));

    weights.resize(getConnectionsCount());
    for (auto& w : weights)
        load.read((char*)&w, sizeof(w));
}

ArtificialNeuralNetwork::~ArtificialNeuralNetwork()
{
}

void ArtificialNeuralNetwork::learn(vector<float> tests, vector<float> results)
{
    uint type = *reinterpret_cast<uint*>(driver);
    if (type == OCL_DRIVER)
    {
        sLog.log("OCL driver is not capable of learning!");
        return;
    }

    FannDriver* d = reinterpret_cast<FannDriver*>(driver);
    assert(tests.size() == d->data->num_data*d->data->num_input);
    assert(results.size() == d->data->num_data*d->data->num_output);

    for (uint i = 0; i < d->data->num_data; ++i)
    {
        memcpy(d->data->input[i], tests.data() + d->data->num_input*i, d->data->num_input);
        memcpy(d->data->output[i], results.data() + d->data->num_output*i, d->data->num_output);
    }

    fann_train_on_data(d->ann, d->data, 5000, 500, 0.05);
    fann_connection a;
}

vector<float> ArtificialNeuralNetwork::run(vector<float> in)
{
    assert(in.size() == layers.front());

    uint type = *reinterpret_cast<uint*>(driver);
    if (type == FANN_DRIVER)
    {
        FannDriver* d = reinterpret_cast<FannDriver*>(driver);
        fann_type* result = fann_run(d->ann, in.data());
        vector<float> ret;
        for (uint i = 0; i < layers.back(); ++i)
            ret.push_back(result[i]);

        return ret;
    }

    OclDriver* ocl = reinterpret_cast<OclDriver*>(driver);
    
    cl_command_queue q = (cl_command_queue)OclMgr::singleton().getDefaultContext()->getCommandQueue();

    cl_event writing_done;
    clEnqueueWriteBuffer(q, (cl_mem)ocl->outputs, CL_TRUE, 0, in.size()*sizeof(in.front()), in.data(), 0, nullptr, &writing_done);
    clWaitForEvents(1, &writing_done);
    OclKernel::Instance* runable = ocl->kernel->getCallableInstance("run", ocl->outputs, ocl->weights, ocl->layers_count, ocl->layers, (cl_int)1024);
    ocl->kernel->run(runable, ocl->workers / 1024, ocl->workers % 1024);

    vector<float> ret(layers.back(), 0.0);
    clEnqueueReadBuffer(q, (cl_mem)ocl->outputs, CL_TRUE, [](vector<uint> const& v) { size_t ret = 0; for (uint i = 0; i < v.size() - 1; ++i) ret += i; return ret; }(layers), layers.back()*sizeof(ret.front()), ret.data(), 0, nullptr, &writing_done);
    clWaitForEvents(1, &writing_done);
    return ret;
}

void ArtificialNeuralNetwork::_initFann(uint trains)
{
    driver = new FannDriver(layers, trains);

}

void ArtificialNeuralNetwork::_initOcl()
{
    driver = new OclDriver(layers, weights);
}
