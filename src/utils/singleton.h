#ifndef SHARED_SINGLETON_H
#define SHARED_SINGLETON_H

template <class C>
class Singleton
{
public:

    static C& singleton()
    {
        static C _me = C();
        return _me;
    }
};

#endif //SHARED_SINGLETON_H
