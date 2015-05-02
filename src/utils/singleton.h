#ifndef SHARED_SINGLETON_H
#define SHARED_SINGLETON_H

template <class C>
class Singleton
{
public:

	template <typename... ARGS>
    static C& singleton(ARGS const&... ctor_args)
    {
        static C _me(ctor_args...);
        return _me;
    }
};

#endif //SHARED_SINGLETON_H
