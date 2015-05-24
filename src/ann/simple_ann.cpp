#include "simple_ann.h"
#include "fann.h"
using namespace std;

struct SimpleAnn::Data
{
    fann* ann;
};

SimpleAnn::SimpleAnn(const Config * anncfg) : SimpleAnn(shared_ptr<const SimpleAnn::Config>(new Config(*anncfg)))
{
}

SimpleAnn::SimpleAnn(SimpleAnn const & other) : SimpleAnn(other._myconf)
{
}

SimpleAnn::SimpleAnn(shared_ptr<const SimpleAnn::Config> const& _ptr) : _myconf(_ptr)
{
    _create();
}

SimpleAnn::~SimpleAnn()
{
    _destroy();
}

void SimpleAnn::train(std::vector<float> data, std::vector<float> output)
{

}

void SimpleAnn::_create()
{
    _mydata = new Data;

    vector<uint> layers;
    layers.push_back(_myconf->inputs);
    for (uint i = 0; i < _myconf->layers - 2; ++i)
        layers.push_back(_myconf->hidden_neurons);
    layers.push_back(_myconf->outputs);
    //_mydata->ann = fann_create_standard(_myconf->layers, *layers.data());
}

void SimpleAnn::_destroy()
{
    //fann_destroy(_mydata->ann);
    _mydata->ann = nullptr;
    delete _mydata;
    _mydata = nullptr;
}

void SimpleAnn::Config::save(ostream & out)
{
    auto len = sizeof(*this);
    out.write((const char*)&len, sizeof(len));
    out.write((const char*)this, len);
}

bool SimpleAnn::Config::load(istream & in)
{
    auto len = sizeof(this);
    in.read((char*)len, sizeof(len));
    if (len != sizeof(this))
        return false;
    in.read((char*)this, sizeof(this));
    return true;
}
