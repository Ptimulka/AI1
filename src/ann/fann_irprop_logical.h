#pragma once

#include "typedefs.h"
#include "utils/vectorn.h"
#include <list>
#include <vector>
#include <atomic>
#include <thread>
#include <iostream>
#include <initializer_list>


class FanniRPROP
{
public:
    FanniRPROP();
    FanniRPROP(std::initializer_list<uint> const& layers);
    ~FanniRPROP();

    inline void* createSession() const { auto* ret = initSession(); _initRunningWorkers(ret); return ret; }
    inline void* createLearningSession() { auto* ret = initSession(); _initLearningWorkers(ret); return ret; }

    void learn(void* session, std::initializer_list<double> input, std::initializer_list<double> output, double target_error = 0.01);

    void run(void* session, std::initializer_list<double> input) const;
    std::list<double> fetchResult(void* session) const;

    //run && fetchResult
    std::list<double> calc(void* session, std::initializer_list<double> input) const;

    void save(std::ostream& out);
    void load(std::istream& in);

    static void deleteSession(void* session)
    {
        delete reinterpret_cast<Session*>(session);
    }

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
        Session(uint layers, uint nodes);
        ~Session();

        vectorn<std::atomic_ullong*, 2> inputs;
        vectorn<double, 2> outputs;
        vectorn<std::atomic_ullong*, 2> d;
        vectorn<double, 3> dE; //pochodne bledu po wadze, uzywane przy nauce
        vectorn<double, 3> pdE; //pochodne bledu po wadze, uzywane przy nauce
        vectorn<double, 3> delta;
        vectorn<double, 3> deltaw;

        std::vector<std::thread*> workers;

        std::atomic<bool> exists;
        std::atomic<bool> run;
        std::atomic<bool> end;
        bool learning;
        bool can_learn;
        std::atomic_uint working;

        std::atomic_uint barrier1;
        std::atomic_uint barrier2;
        std::atomic_uint barrier3;
        std::atomic_uint barrier4;

        double learn_rate_pos;
        double learn_rate_neg;
        double delta_max;
        double delta_min;
        double error;
        double preverror;
        vectorn<double, 1> expected;
    };

    Session* initSession() const;

    void _initLearningWorkers(Session* s);
    void _initRunningWorkers(Session* s) const;

    void _run(Session* s) const;
};
