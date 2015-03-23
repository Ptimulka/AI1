#include "log.h"
#include <thread>
using namespace std;

Log::Log() : init(false), run(), worker([this]{ while (this->run || !init){ std::this_thread::sleep_for(std::chrono::seconds(1)); if (init) this->_flush(); }}), msgs()
{
    atomic_init(&run, true);
    init = true;
}

Log::~Log()
{
    close();
}

void Log::log(string str)
{
    if (!run)
        return;

    Msg* m = new Msg();
    m->msg = std::move(str);
    m->queue_time = std::chrono::system_clock::now();
    m->thread_id = std::this_thread::get_id();
    msgs.push(m);
}

void Log::_flush()
{
    for (auto msg : msgs.fetchAll())
    {
        for (auto o : outs)
            (*o) << makestr("[Thread ", msg->thread_id, " at ", msg->queue_time, "]:  ", msg->msg) << '\n';
        delete msg;
    }
}
