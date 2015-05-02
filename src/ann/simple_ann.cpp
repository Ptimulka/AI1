#include "simple_ann.h"
#include "fann.h"
using namespace std;

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

void SimpleAnn::_create()
{
}

void SimpleAnn::_destroy()
{
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
