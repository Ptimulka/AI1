#pragma once

#include "typedefs.h"
#include <vector>
#include <istream>
#include <ostream>
#include <memory>

class SimpleAnn
{
public:
    struct Config
    {
        uint inputs = 0;
        uint outputs = 0;
        uint layers = 0;
        uint hidden_neurons = 0;

        float training_eps = 1e-6f;


        inline SimpleAnn produceAnn() const
        {
            return SimpleAnn(this);
        }

        void save(std::ostream& out);
        bool load(std::istream& in);
    };

private:
    explicit SimpleAnn(std::shared_ptr<const Config> const& _ptr);

public:
    SimpleAnn(const Config* anncfg);
    SimpleAnn(SimpleAnn const& other);
    ~SimpleAnn();

    const Config* getConfig() const { return _myconf.get(); }

    void train(std::vector<float> data, std::vector<float> output);

private:
    std::shared_ptr<const Config> _myconf;

    struct Data;
    Data* _mydata = nullptr;

    void _create();
    void _destroy();
};
