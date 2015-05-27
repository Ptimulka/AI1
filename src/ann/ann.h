#pragma once

#include "typedefs.h"
#include "utils/vectorn.h"
#include <iostream>

class ArtificialNeuralNetwork
{
public:
    enum AnnDriver
    {
        FANN_DRIVER,
        OCL_DRIVER
    };

    struct FannDriver;
    struct OclDriver;

public:
    ArtificialNeuralNetwork(std::vector<uint> nodes_in_layers);
    ArtificialNeuralNetwork(std::istream& load, bool native = false, uint trains = 0, char* configuration_file = "default.ann");
    ~ArtificialNeuralNetwork();

    template <typename DRIVER, typename... DONOTPASS>
    void init(DONOTPASS... a) { static_assert(_Always_false<DRIVER>::value, "Invalid driver type."); }

    template <>
    void init<FannDriver>(uint trains)
    {
        _initFann(trains);
    }

    template <>
    void init<OclDriver>()
    {
        _initOcl();
    }

    void learn(std::vector<float> tests, std::vector<float> results);
    std::vector<float> run(std::vector<float> in);

    inline unsigned int getConnectionsCount() const
    {
        uint connections = 0;
        uint prev = 0;
        for (auto const& l : layers)
            connections += prev*l, prev = l;

        return connections;
    }

    void save(std::ostream& out, bool native = false, char* configuration_file = "default.ann") const;

private:
    void* driver;

    std::vector<uint> layers;
    std::vector<float> weights;

    void _initFann(uint trains);
    void _initOcl();
};
