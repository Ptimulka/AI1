#include "ann.h"
#include "utils/log.h"

#define NOMINMAX
#include "fann.h"
#include "cl/oclkernel.h"
#include "cl/oclcontext.h"
#include "cl/oclmgr.h"

#include <map>
#include <algorithm>
#include <CL/cl.h>
using namespace std;

template <typename T>
T& get_weight(vector<T>& v, uint l, uint curr, uint prev, vector<uint> const& s)
{
    assert(l > 0);

    uint offset = 0;
    for (uint i = 1; i < l; ++i)
        offset += s[i]*s[i-1];
    offset += s[l - 1] * curr;
    return v[prev + offset];
}

template <typename T>
T const& get_weight(vector<T> const& v, uint l, uint curr, uint prev, vector<uint> const& s)
{
    assert(l > 0);

    uint offset = 0;
    for (uint i = 1; i < l; ++i)
        offset += s[i] * s[i - 1];
    offset += s[l - 1] * curr;
    return v[prev + offset];
}

struct ArtificialNeuralNetwork::FannDriver
{
    AnnDriver driver = ArtificialNeuralNetwork::FANN_DRIVER;

    fann* ann = nullptr;
    fann_train_data* data = nullptr;

    struct FannNodeCoords
    {
        uint layer;
        uint neuron;
    };

    map<void*, FannNodeCoords> nodesmap;

    FannDriver(vector<uint> const& layers, vector<float> const& weights, uint training_data, fann *fann = nullptr)
    {
        if (fann!=nullptr)
            ann = fann;     
        else
        {
            ann = fann_create_standard_array(layers.size(), layers.data());
            fann_set_activation_function_hidden(ann, FANN_SIGMOID);
            fann_set_activation_function_output(ann, FANN_SIGMOID);
            fann_set_activation_steepness_hidden(ann, 0.5f);
            fann_set_activation_steepness_output(ann, 0.5f);
            fann_set_train_stop_function(ann, FANN_STOPFUNC_MSE);

            if (training_data)
                data = fann_create_train(training_data, layers.front(), layers.back());
            else
                data = nullptr;

            { //copied from fann's fann_set_weight

                unsigned inttype source_index = 0;

                auto first_neuron = ann->first_layer->first_neuron;

                /* Find the connection, simple brute force search through the network
                for one or more connections that match to minimize datastructure dependencies.
                Nothing is done if the connection does not already exist in the network. */

                uint layer_idx = 0;
                for (auto layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++, ++layer_idx)
                {
                    uint neuron_idx = 0;
                    for (auto neuron_it = layer_it->first_neuron; neuron_it != layer_it->last_neuron - 1; neuron_it++, ++neuron_idx)
                    {
                        nodesmap.insert(make_pair(neuron_it, FannNodeCoords{ layer_idx, neuron_idx }));
                    }
                }

                /* for each layer */
                for (auto layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++)
                {
                    /* for each neuron */
                    for (auto neuron_it = layer_it->first_neuron; neuron_it < layer_it->last_neuron - 1; neuron_it++)
                    {
                        /* for each connection */
                        for (auto idx = neuron_it->first_con; neuron_it->last_con != 0 && idx < neuron_it->last_con - 1; idx++)
                        {
                            auto& from = nodesmap[ann->connections[idx]];
                            auto& to = nodesmap[neuron_it];
                            assert(from.layer + 1 == to.layer);

                            ann->weights[idx] = get_weight(weights, to.layer, to.neuron, from.neuron, layers);
                        }
                    }
                }
            }
        }
        
    }

	FannDriver(const char* filename, uint trains)
	{
		ann = fann_create_from_file(filename);
		uint lcount = fann_get_num_layers(ann);
		uint *l = new uint[lcount];
		fann_get_layer_array(ann, l);

		if (trains)
			data = fann_create_train(trains, l[0], l[lcount-1]);
		else
			data = nullptr;

		delete[] l;

		uint layer_idx = 0;
		for (auto layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++, ++layer_idx)
		{
			uint neuron_idx = 0;
			for (auto neuron_it = layer_it->first_neuron; neuron_it != layer_it->last_neuron - 1; neuron_it++, ++neuron_idx)
			{
				nodesmap.insert(make_pair(neuron_it, FannNodeCoords{ layer_idx, neuron_idx }));
			}
		}

	}

    ~FannDriver()
    {
        fann_destroy_train(data);
        fann_destroy(ann);

        data = nullptr;
        ann = nullptr;
    }

    void fetchWeights(vector<float>& weights, vector<uint> const& layers)
    {
        unsigned inttype source_index = 0;

        /* for each layer */
        for (auto layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++)
        {
            /* for each neuron */
            for (auto neuron_it = layer_it->first_neuron; neuron_it != layer_it->last_neuron - 1; neuron_it++)
            {
                /* for each connection */
                for (auto idx = neuron_it->first_con; neuron_it->last_con != 0 && idx < neuron_it->last_con - 1; idx++)
                {
                    auto& from = nodesmap[ann->connections[idx]];
                    auto& to = nodesmap[neuron_it];
                    assert(from.layer + 1 == to.layer);

                    get_weight(weights, to.layer, to.neuron, from.neuron, layers) = ann->weights[idx];
                }
            }
        }
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

ArtificialNeuralNetwork::ArtificialNeuralNetwork() : layers(), weights()
{

}

ArtificialNeuralNetwork::ArtificialNeuralNetwork(std::vector<uint> nodes_in_layers) : layers(nodes_in_layers), weights(getConnectionsCount())
{
    for (auto& w : weights)
        w = (decay<decltype(w)>::type)rand() / RAND_MAX;
}

ArtificialNeuralNetwork::ArtificialNeuralNetwork(istream& load)
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
	uint old_data = d->data->num_data;
	d->data->num_data = tests.size() / d->data->num_input;

	assert(d->data->num_data <= old_data);
    assert(tests.size() == d->data->num_data*d->data->num_input);
    assert(results.size() == d->data->num_data*d->data->num_output);

    for (uint i = 0; i < d->data->num_data; ++i)
    {
        memcpy(d->data->input[i], tests.data() + d->data->num_input*i, d->data->num_input*sizeof(float));
		memcpy(d->data->output[i], results.data() + d->data->num_output*i, d->data->num_output*sizeof(float));
    }

    fann_train_on_data(d->ann, d->data, 5000, 500, 0.0001f);
    d->fetchWeights(weights, layers);
	d->data->num_data = old_data;
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

void ArtificialNeuralNetwork::save(ostream & out) const
{
    uint lcount = layers.size();
    out.write((char*)&lcount, sizeof(lcount));
    for (auto& l : layers)
        out.write((char*)&l, sizeof(l));

    for (auto& w : weights)
        out.write((char*)&w, sizeof(w));

}

void ArtificialNeuralNetwork::saveNative(string const& filename) const
{
	uint type = *reinterpret_cast<uint*>(driver);
	if (type == FANN_DRIVER)
	{
		FannDriver* d = reinterpret_cast<FannDriver*>(driver);
		fann_save(d->ann, filename.c_str());
	}
}

void ArtificialNeuralNetwork::_initFann(uint trains)
{
    driver = new FannDriver(layers, weights, trains);
}

void ArtificialNeuralNetwork::_initFann(std::string const& loadfromfile, uint trains)
{
	auto _driver = new FannDriver(loadfromfile.c_str(), trains);
	uint lcount = fann_get_num_layers(_driver->ann);
	uint *l = new uint[lcount];
	fann_get_layer_array(_driver->ann, l);
	layers.assign(l, l + lcount);
	weights.resize(getConnectionsCount());
	_driver->fetchWeights(weights, layers);

	driver = _driver;
}

void ArtificialNeuralNetwork::_initOcl()
{
    driver = new OclDriver(layers, weights);
}
