#ifndef SHARED_LOG_H
#define SHARED_LOG_H

#include "singleton.h"
#include "strutils.h"
#include "nlqueue.h"
#include <ostream>
#include <thread>
#include <list>

class Log : public Singleton<Log>
{
public:
    Log();
    ~Log();

    void log(std::string str);

    template <typename... T>
    void log(T const&... t)
    {
        if (sizeof...(t) == 0)
            return;

        return log(makestr(t...));
    }

    inline void addOutput(std::ostream* out)
    {
        if (out == nullptr)
            return;

        outs.push_back(out);
    }

    inline void close(bool flush = true)
    {
        if (run.exchange(false))
            worker.join();
        if (flush)
            _flush();

        outs.clear();
    }


private:
    struct Msg
    {
        std::string msg;
        decltype(std::this_thread::get_id()) thread_id;
        std::chrono::system_clock::time_point queue_time;
    };

    bool init;
    std::atomic_bool run;
    std::thread worker;

    NlQueue<Msg*> msgs;
    std::list<std::ostream*> outs;

    void _flush();
};

#define sLog Log::singleton()

#endif //SHARED_LOG_H
