#ifndef MIRROR_FLAG_HEADER
#define MIRROR_FLAG_HEADER

template <typename T>
struct Flag
{
public:
    typedef T type;

public:
    Flag() : flag(T())
    {}
    Flag(T const& f) : flag(f)
    {}
    Flag(Flag const& f) : flag(f.flag)
    {}

    //inline Flag& operator <<(T const& mask)
    //{
    //    flag |= mask;
    //    return (*this);
    //}

    //inline Flag& operator <<(Flag const& f)
    //{
    //    flag |= f.flag;
    //    return (*this);
    //}

    //inline Flag& operator >>(T const& mask)
    //{
    //    flag &= ~(mask);
    //    return (*this);
    //}

    //inline Flag& operator >>(Flag const& f)
    //{
    //    flag &= ~(f.flag);
    //    return (*this);
    //}

    inline bool operator [](T const& mask) const
    {
        return ((flag & mask) != 0);
    }

    inline bool operator [](Flag const& f) const
    {
        return ((flag & f.flag) != 0);
    }

    inline bool operator ==(Flag const& f) const
    {
        return (flag == f.flag);
    }

    inline bool operator !=(Flag const& f) const
    {
        return (flag != f.flag);
    }

    inline bool operator ()(Flag const& f, T const& mask) const
    {
        return (*this)[mask] == f[mask];
    }

    inline bool operator ()(Flag const& f, Flag const& mask) const
    {
        return (*this)[mask] == f[mask];
    }

    inline Flag& reverse()
    {
        flag = ~flag;
    }

    inline Flag reversed() const
    {
        return Flag(~flag);
    }

    inline Flag& intersect(Flag const& f)
    {
        (flag &= f.flag);
        return (*this);
    }

    inline Flag intersection(Flag const& f) const
    {
        return Flag(flag & f.flag);
    }

    inline Flag& substract(Flag const& f)
    {
        flag &= ~(f.flag);
        return (*this);
    }

    inline Flag remainding(Flag const& f)
    {
        return Flag(flag & ~f.flag);
    }

    inline Flag& add(Flag const& f)
    {
        flag |= f.flag;
        return (*this);
    }
    
    inline void operator =(Flag const& f)
    {
        flag = f.flag;
    }

    inline void operator =(T const& f)
    {
        flag = f;
    }

    inline operator bool() const
    {
        return (0!=flag);
    }

    T flag;
};

#endif //MIRROR_FLAG_HEADER
