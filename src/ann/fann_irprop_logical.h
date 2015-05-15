#pragma once

#include "typedefs.h"
#include <list>
#include <vector>
#include <atomic>
#include <thread>
#include <initializer_list>

template <typename T, uint DIM>
struct vector_n : public std::vector<vector_n<T, DIM-1>>
{
};

template <typename T>
struct vector_n<T, 1> : public std::vector<T>
{
};

template <typename T>
struct vector_n<T, 0>
{
};

class FanniRPROP
{
public:
    FanniRPROP(std::initializer_list<uint> const& layers);
    ~FanniRPROP();

    void* createSession();

    void learn(void* session, std::initializer_list<double> input, std::initializer_list<double> output);

private:
    struct Node
    {
        uint id; //idx in layer
        uint layer; //layer idx

        std::vector<double> in_weights;
    };
    struct Layer
    {
        uint id; //layer id
        std::vector<Node> nodes;
    };
    std::vector<Layer> net;
    uint max_nodes = 0;

    void createNewNet(std::initializer_list<uint> const& layers);


    struct Session
    {
        vector_n<std::atomic_ullong, 2> outputs;
        vector_n<std::atomic_ullong, 2> d;
        vector_n<double, 3> dE; //pochodne bledu po wadze, uzywane przy nauce
        vector_n<double, 3> pdE; //pochodne bledu po wadze, uzywane przy nauce
        vector_n<double, 3> delta;
        vector_n<double, 3> deltaw;

        std::vector<std::thread*> workers;

        bool exists;
        bool run;
        bool learning;
        std::atomic_int working;
        std::atomic_uint waiting_counter;

        double learn_rate_pos;
        double learn_rate_neg;
        double delta_max;
        double delta_min;
        double error;
        double preverror;
        vector_n<double, 1> expected;
    };

    Session* initSession();
};
