#include "fann_irprop_logical.h"
#include <cassert>
#include <algorithm>
using namespace std;

const double& as_double(unsigned long long const& mem)
{
    return *(const double* const)&mem;
}

double& as_double(unsigned long long& mem)
{
    return *(double* const)&mem;
}

const unsigned long long& as_ull(double const& val)
{
    return *(const unsigned long long*)&val;
}

unsigned long long& as_ull(double &val)
{
    return *(unsigned long long*)&val;
}

void atomic_double_add(atomic_ullong& mem, double value_to_add)
{
    double expected;
    double sum;

    do
    {
        expected = as_double(mem.load());
        sum = expected + value_to_add;
    } while (mem.compare_exchange_strong(as_ull(expected), as_ull(sum)) == false);
}

void wait_for_count(atomic_uint& counter, unsigned count, uint prev_count)
{
    while (counter.load() >= min(prev_count + 1, count))
        ;// this_thread::sleep_for(chrono::milliseconds(1));

    auto waiting = counter.fetch_add(1) + 1;
    if (waiting < count)
        while (counter < count)
            ;// this_thread::sleep_for(chrono::milliseconds(1));

    waiting = counter.fetch_add(1) + 1;
    if (waiting == count * 2)
        counter = 0;
}

int sign(double f)
{
    if (f > 0)
        return 1;
    else if (f < 0)
        return -1;
    else
        return 0;
}



FanniRPROP::FanniRPROP()
{
}

FanniRPROP::FanniRPROP(std::initializer_list<uint> const & layers)
{
    createNewNet(layers);
}

FanniRPROP::~FanniRPROP()
{
}

void FanniRPROP::learn(void * session, std::initializer_list<double> input, std::initializer_list<double> output, double target_error)
{
    assert(output.size() == net.back().nodes.size());

    Session* s = reinterpret_cast<Session*>(session);
    assert(s->can_learn);

    uint i = 0;
    for (auto& out : output)
        s->expected[i++] = out;

    i = 0;
    for (auto& in : input)
        s->outputs[0][i++] = in;

    s->learning = true;

    const uint iter_max = 5000;
    uint iter = 0;
    while (iter++ < iter_max)
    {
        _run(s);

        s->error = 0.0;
        auto itr = output.begin();
        for (uint i = 0; i < net.back().nodes.size(); ++i)
        {
            auto new_error = *itr++ - s->outputs.back()[i];
            if (abs(s->error) < abs(new_error))
                s->error = new_error;
        }

        if (abs(s->error) < target_error)
            break;

        s->dE.swap(s->pdE);
        swap(s->error, s->preverror);
    }

    s->learning = false;
}

void FanniRPROP::run(void * session, std::initializer_list<double> input) const
{
    assert(input.size() == net.front().nodes.size());

    Session* s = reinterpret_cast<Session*>(session);
    uint i = 0;
    for (auto& in : input)
        s->outputs[0][i++] = in;

    _run(s);
}

void FanniRPROP::_run(Session * s) const
{
    while (s->waiting_counter != 0)
        ;//this_thread::sleep_for(chrono::milliseconds(1));

    s->run = true;
    s->end = false;
    while (!s->end /*|| s->waiting_counter.load() < s->workers.size()*/)
        ;//this_thread::sleep_for(chrono::milliseconds(1));

    s->run = false;
    wait_for_count(s->waiting_counter, s->workers.size() + 1, s->workers.size());
}

std::list<double> FanniRPROP::fetchResult(void* session) const
{
    list<double> ret;

    const auto& result = reinterpret_cast<Session*>(session)->outputs.back();
    for (uint i = 0; i < net.back().nodes.size(); ++i)
        ret.push_back(result[i]);

    return ret;
}

std::list<double> FanniRPROP::calc(void * session, std::initializer_list<double> input) const
{
    run(session, input);
    return fetchResult(session);
}

void FanniRPROP::save(std::ostream & out)
{
    auto layers = net.size();
    out.write((const char*)&layers, sizeof(layers));

    for (Layer& l : net)
    {
        auto size = l.nodes.size();
        out.write((const char*)&l.id, sizeof(l.id));
        out.write((const char*)&size, sizeof(size));
        for (Node& n : l.nodes)
        {
            out.write((const char*)&n.id, sizeof(n.id));
            out.write((const char*)&n.layer, sizeof(n.layer));
            
            auto weights = n.in_weights.size();
            out.write((const char*)&weights, sizeof(weights));
            out.write((const char*)n.in_weights.data(), sizeof(n.in_weights.front())*weights);
        }
    }
}

void FanniRPROP::load(std::istream & in)
{
    decltype(net.size()) layers;
    in.read((char*)&layers, sizeof(layers));

    net.resize(layers);
    for (Layer& l : net)
    {
        decltype(l.nodes.size()) size;
        in.read((char*)&l.id, sizeof(l.id));
        in.read((char*)&size, sizeof(size));

        l.nodes.resize(size);
        for (Node& n : l.nodes)
        {
            in.read((char*)&n.id, sizeof(n.id));
            in.read((char*)&n.layer, sizeof(n.layer));

            decltype(n.in_weights.size()) weights;
            in.read((char*)&weights, sizeof(weights));
            n.in_weights.resize(weights);
            in.read((char*)n.in_weights.data(), sizeof(n.in_weights.front())*weights);
        }
    }
}

void FanniRPROP::createNewNet(initializer_list<uint> const& layers)
{
    assert(layers.size() >= 2);
    net.clear();
    net.resize(layers.size());

    max_nodes = max(layers);

    uint lidx = 0;
    auto itr = layers.begin();
    for (Layer& l : net)
    {
        l.id = lidx++;
        l.nodes.resize(*itr++);

        uint nidx = 0;
        for (Node& n : l.nodes)
        {
            n.id = nidx++;
            n.layer = l.id;
            if (l.id != 0)
            {
                Layer& prev = net[l.id - 1];
                n.in_weights.resize(prev.nodes.size());
                for (auto& w : n.in_weights)
                    w = rand() / (double)RAND_MAX;
            }
        }
    }
}

FanniRPROP::Session* FanniRPROP::initSession() const
{
    Session* new_session = new Session(net.size(), max_nodes);
    new_session->exists = true;
    new_session->run = false;
    new_session->end = false;
    new_session->learning = false;
    new_session->can_learn = false;
    atomic_init(&new_session->working, 0);
    atomic_init(&new_session->waiting_counter, 0);

    new_session->learn_rate_pos = 0.8;
    new_session->learn_rate_neg = 1.2;
    new_session->delta_max = 50;
    new_session->delta_min = 0.001;
    new_session->error = 0.0;
    new_session->preverror = 0.0;

    for (uint i = 0; i < net.size(); ++i)
    {
        for (uint j = 0; j < max_nodes; ++j)
        {
            new_session->inputs[i][j] = new atomic_ullong();
            new_session->d[i][j] = new atomic_ullong();
            atomic_init(new_session->inputs[i][j], as_ull(0.0));
            atomic_init(new_session->d[i][j], 0);
        }
    }

    new_session->learn_rate_pos = 1.2;
    new_session->learn_rate_neg = 0.5;
    new_session->delta_max = 50;
    new_session->delta_min = 0;

    uint wid = 0;
    new_session->workers.resize(max_nodes*max_nodes);

    return new_session;
}

void FanniRPROP::_initLearningWorkers(Session* new_session)
{
    new_session->can_learn = true;
    uint wid = 0;
    for (auto* tptr : new_session->workers)
    {
        tptr = new thread([new_session, this](const uint myneuron, const uint prevneuron) {
            while (new_session->exists)
            {
                while ((!new_session->run || new_session->end || new_session->working > new_session->workers.size()) && new_session->exists)
                    ;/*this_thread::sleep_for(chrono::milliseconds(1));*/

                if (!new_session->exists)
                    break;

                new_session->working.fetch_add(1);
                for (uint i = 1; i < net.size(); ++i)
                {
                    if (prevneuron == 0)
                        new_session->inputs[i][myneuron]->store(0);

                    wait_for_count(new_session->waiting_counter, new_session->workers.size(), new_session->workers.size() + 1);

                    if (net[i].nodes.size() > myneuron && net[i - 1].nodes.size() > prevneuron)
                    {
                        double tmp = new_session->outputs[i - 1][prevneuron] * net[i].nodes[myneuron].in_weights[prevneuron];
                        atomic_double_add(*new_session->inputs[i][myneuron], tmp);
                    }

                    wait_for_count(new_session->waiting_counter, new_session->workers.size(), new_session->workers.size());

                    if (prevneuron == 0)
                    {
                        auto mem = new_session->inputs[i][myneuron]->load();
                        auto val = 1.0 / (1+exp(-as_double(mem)));
                        new_session->outputs[i][myneuron] = val;
                    }

                    wait_for_count(new_session->waiting_counter, new_session->workers.size(), new_session->workers.size());
                }

                if (new_session->learning)
                {
                    auto nextneuron = prevneuron;

                    if (nextneuron == 0)
                        new_session->d.back()[myneuron]->store(as_ull((new_session->outputs.back()[myneuron] - new_session->expected[myneuron]) * new_session->outputs.back()[myneuron] * (1 - new_session->outputs.back()[myneuron])));

                    wait_for_count(new_session->waiting_counter, new_session->workers.size(), new_session->workers.size());

                    for (int lid = net.size() - 2; lid > 0; --lid)
                    {
                        if (net[lid + 1].nodes.size() > nextneuron && net[lid].nodes.size() > myneuron)
                        {
                            double tmp = as_double(new_session->d[lid + 1][nextneuron]->load()) * net[lid + 1].nodes[nextneuron].in_weights[myneuron]*new_session->outputs[lid][myneuron] * (1 - new_session->outputs[lid][myneuron]);
                            atomic_double_add(*new_session->d[lid][myneuron], tmp);
                        }
                        wait_for_count(new_session->waiting_counter, new_session->workers.size(), new_session->workers.size());
                    }

                    for (uint lid = 1; lid<net.size(); ++lid)
                    {
                        if (net[lid].nodes.size() > myneuron && net[lid - 1].nodes.size() > prevneuron)
                        {
                            auto& d = as_double(new_session->d[lid][nextneuron]->load());
                            auto& pdE = new_session->pdE[lid][myneuron][prevneuron];
                            auto& dE = new_session->dE[lid][myneuron][prevneuron];
                            auto& delta = new_session->delta[lid][myneuron][prevneuron];
                            auto& deltaw = new_session->deltaw[lid][myneuron][prevneuron];
                            auto& w = net[lid].nodes[myneuron].in_weights[prevneuron];
                            auto& x = new_session->outputs[lid-1][prevneuron];

                            dE = d * x;

                            if (pdE*dE > 0)
                            {
                                delta = min(new_session->learn_rate_pos * delta, new_session->delta_max);
                                deltaw = -sign(dE)*delta;
                                w += deltaw;
                            }
                            else if (pdE*dE < 0)
                            {
                                delta = max(new_session->learn_rate_neg*delta, new_session->delta_min);
                                if (new_session->error > new_session->preverror)
                                    w -= deltaw;
                                dE = 0;
                            }
                            else
                            {
                                deltaw = -sign(dE)*delta;
                                w += deltaw;
                            }
                        }

                        wait_for_count(new_session->waiting_counter, new_session->workers.size(), new_session->workers.size());
                    }

                }

                if (new_session->working.fetch_add(1) +1 == new_session->workers.size() * 2)
                {
                    new_session->end = true;
                    new_session->working = 0;
                }
                wait_for_count(new_session->waiting_counter, new_session->workers.size() + 1, new_session->workers.size());

            }
        }, wid / max_nodes, wid%max_nodes);

        ++wid;
    }
}

void FanniRPROP::_initRunningWorkers(Session* new_session) const
{
    uint wid = 0;
    for (auto* tptr : new_session->workers)
    {
        tptr = new thread([new_session, this](uint myneuron, uint prevneuron) {
            while (new_session->exists)
            {
                while ((!new_session->run || new_session->end || new_session->working > new_session->workers.size()) && new_session->exists)
                    this_thread::sleep_for(chrono::milliseconds(1));

                if (!new_session->exists)
                    break;

                new_session->working.fetch_add(1);
                for (uint i = 1; i < net.size(); ++i)
                {
                    double tmp = new_session->outputs[i - 1][prevneuron];
                    atomic_double_add(*new_session->inputs[i][myneuron], tmp);

                    wait_for_count(new_session->waiting_counter, new_session->workers.size(), new_session->workers.size() + 1);

                    if (prevneuron == 0)
                    {
                        auto mem = new_session->inputs[i][myneuron]->load();
                        auto val = 1.0 / (1 + exp(-as_double(mem)));
                        new_session->outputs[i][myneuron] = val;
                    }

                    wait_for_count(new_session->waiting_counter, new_session->workers.size(), new_session->workers.size());
                }

                if (new_session->working.fetch_add(1) + 1 == new_session->workers.size() * 2)
                {
                    new_session->end = true;
                    new_session->working = 0;
                }
                wait_for_count(new_session->waiting_counter, new_session->workers.size() + 1, new_session->workers.size());

            }
        }, wid / max_nodes, wid%max_nodes);

        ++wid;
    }
}


FanniRPROP::Session::Session(uint layers, uint nodes) : inputs(nullptr, layers, nodes), outputs(layers, nodes), d(nullptr, layers, nodes), dE(layers, nodes, nodes), pdE(layers, nodes, nodes), delta(0.5, layers, nodes, nodes), deltaw(layers, nodes, nodes), expected(nodes)
{
}

FanniRPROP::Session::~Session()
{
    exists = false;
    for (auto& t : workers)
    {
        t->join();
        delete t;
        t = nullptr;
    }

    for (auto& _o : inputs)
        for (auto& a : _o)
        {
            delete a;
            a = nullptr;
        }

    for (auto& _d : d)
        for (auto& a : _d)
        {
            delete a;
            a = nullptr;
        }
}
