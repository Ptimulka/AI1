#pragma once

#include "typedefs.h"
#include <list>
#include <vector>
#include <atomic>
#include <thread>
#include <iostream>
#include <initializer_list>

template <typename T, uint DIM>
struct vector_n : public std::vector<vector_n<T, DIM - 1>>
{
    vector_n() : std::vector<vector_n<T, DIM - 1>>()
    {}

    template <typename... INTS>
    vector_n(T val, uint mysize, INTS... rest) : std::vector<vector_n<T, DIM-1>>(mysize, vector_n<T, DIM-1>(val, rest...))
    {
        static_assert(_all_same<uint, INTS...>::value, "");
        static_assert(sizeof...(rest) == DIM-1, "");
    }

    template <typename... INTS>
    vector_n(uint mysize, INTS... rest) : std::vector<vector_n<T, DIM - 1>>(mysize, vector_n<T, DIM - 1>(rest...))
    {
        static_assert(_all_same<uint, INTS...>::value, "");
        static_assert(sizeof...(rest) == DIM - 1, "");
    }

private:
    template <typename TEST, typename T1, typename... REST> struct _all_same { enum { value = _all_same<TEST, T1>::value && _all_same<TEST, REST...>::value }; };
    template <typename TEST, typename A> struct _all_same<TEST,A>{ enum { value = false }; };
    template <typename TEST> struct _all_same<TEST,TEST>{ enum { value = true }; };
};

template <typename T>
struct vector_n<T, 1> : public std::vector<T>
{
    vector_n() : std::vector<T>()
    {}

    vector_n(T val, uint mysize) : std::vector<T>(mysize, val)
    {}

    vector_n(uint mysize) : std::vector<T>(mysize)
    {}
};

template <typename T>
struct vector_n<T, 0>
{
};

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

        vector_n<std::atomic_ullong*, 2> inputs;
        vector_n<double, 2> outputs;
        vector_n<std::atomic_ullong*, 2> d;
        vector_n<double, 3> dE; //pochodne bledu po wadze, uzywane przy nauce
        vector_n<double, 3> pdE; //pochodne bledu po wadze, uzywane przy nauce
        vector_n<double, 3> delta;
        vector_n<double, 3> deltaw;

        std::vector<std::thread*> workers;

        bool exists;
        bool run;
        bool end;
        bool learning;
        bool can_learn;
        std::atomic_uint working;
        std::atomic_uint waiting_counter;

        double learn_rate_pos;
        double learn_rate_neg;
        double delta_max;
        double delta_min;
        double error;
        double preverror;
        vector_n<double, 1> expected;
    };

    Session* initSession() const;

    void _initLearningWorkers(Session* s);
    void _initRunningWorkers(Session* s) const;

    void _run(Session* s) const;
};
