#include "fann_irprop_logical.h"
#include <cassert>
#include <algorithm>
using namespace std;

void * FanniRPROP::createSession()
{
    return initSession();
}

void FanniRPROP::createNewNet(initializer_list<uint> const & layers)
{
    assert(layers.size() >= 2);
    net.clear();
    net.resize(layers.size());

    max_nodes = *max(layers.begin(), layers.end());

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
    } while (mem.compare_exchange_strong(as_ull(expected), as_ull(sum)) != as_ull(expected));
}

void wait_for_count(atomic_uint& counter, unsigned count)
{
    auto waiting = counter.fetch_add(1) + 1;
    if (waiting < count)
        while (counter < count)
            this_thread::sleep_for(chrono::milliseconds(1));

    waiting = counter.fetch_add(1) + 1;
    if (waiting == count * 2)
        counter.exchange(0);
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

FanniRPROP::Session* FanniRPROP::initSession()
{
    Session* new_session = new Session;
    new_session->exists = true;
    new_session->run = false;
    new_session->learning = false;
    atomic_init(&new_session->working, 0);
    atomic_init(&new_session->waiting_counter, 0);

    new_session->learn_rate_pos = 0.8;
    new_session->learn_rate_neg = 1.2;
    new_session->delta_max = 50;
    new_session->delta_min = 0.001;
    new_session->error = 0.0;
    new_session->preverror = 0.0;
    new_session->outputs.resize(net.size());
    new_session->d.resize(net.size());
    new_session
    new_session->outputs.resize(net.size());
    for (uint i = 0; i < net.size(); ++i)
    {
        new_session->outputs[i].resize(net[i].nodes.size());
        for (auto& a : new_session->outputs[i])
            atomic_init(&a, as_ull(0.0));
    }

    uint wid = 0;
    new_session->workers.resize(max_nodes*max_nodes);
    for (auto* tptr : new_session->workers)
    {
        tptr = new thread([new_session, this](uint myneuron, uint prevneuron) {
            while (new_session->exists)
            {
                while (!new_session->run)
                    this_thread::sleep_for(chrono::milliseconds(100));

                new_session->working.fetch_add(1);

                for (uint i = 1; i < net.size(); ++i)
                {
                    double tmp = new_session->outputs[i - 1][prevneuron];
                    atomic_double_add(new_session->outputs[i][myneuron], tmp);

                    wait_for_count(new_session->waiting_counter, new_session->workers.size());

                    if (myneuron == 0)
                    {
                        auto mem = new_session->outputs[i][myneuron].load();
                        auto val = 1.0 / exp(-as_double(mem));
                        new_session->outputs[i][myneuron] = as_ull(val);
                    }

                    wait_for_count(new_session->waiting_counter, new_session->workers.size());
                }

                if (new_session->learning)
                {
                    auto nextneuron = prevneuron;

                    if (nextneuron == 0)
                        new_session->d.back()[myneuron] = as_ull((new_session->outputs.back()[myneuron] - new_session->expected[myneuron]) * as_double(new_session->outputs.back()[myneuron]) *(1 - as_double(new_session->outputs.back()[myneuron])));

                    wait_for_count(new_session->waiting_counter, new_session->workers.size());

                    for (int lid = net.size() - 2; lid >= 0; ++lid)
                    {
                        double tmp = as_double(new_session->d[lid+1][nextneuron].load()) * net[lid+1].nodes[nextneuron].in_weights[myneuron];
                        atomic_double_add(new_session->d[lid][myneuron], tmp);
                        wait_for_count(new_session->waiting_counter, new_session->workers.size());
                    }

                    for (int lid = 1; lid<net.size() - 1; ++lid)
                    {
                        auto& d = new_session->d[lid][nextneuron];
                        auto& pdE = new_session->pdE[lid][myneuron][prevneuron];
                        auto& dE = new_session->dE[lid][myneuron][prevneuron];
                        auto& delta = new_session->delta[lid][myneuron][prevneuron];
                        auto& deltaw = new_session->deltaw[lid][myneuron][prevneuron];
                        auto& w = net[lid].nodes[myneuron].in_weights[prevneuron];

                        dE = d * as_double(new_session->outputs[lid-1][myneuron]);
                        wait_for_count(new_session->waiting_counter, new_session->workers.size());

                        if (pdE*dE > 0)
                        {
                            delta = min(new_session->learn_rate_pos * delta, new_session->delta_max);
                            deltaw = -sign(dE)*delta;
                            w += deltaw;
                        }
                        else if (pdE*dE< 0)
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

                }

                new_session->working.fetch_sub(1);

            }
        }, wid/max_nodes, wid%max_nodes);
    }

    return new_session;
}
