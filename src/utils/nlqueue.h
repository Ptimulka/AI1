#ifndef SHARED_NON_LOCKING_QUEUE_H
#define SHARED_NON_LOCKING_QUEUE_H

#include <list>
#include <atomic>
#include <exception>

template <typename T>
class NlQueue
{
public:
    NlQueue()
    {
        std::atomic_init(&guard, new Node());
        std::atomic_init(&head, guard);
    }

    ~NlQueue()
    {
        Node* tmp = nullptr;
        while (head.load() != nullptr)
        {
            tmp = head;
            head = tmp->next;
            delete tmp;
        }
    }

    inline void push(T const& t)
    {
        _push(new Node(t));
    }

    inline void push(T&& t)
    {
        _push(new Node(std::move(t)));
    }

    inline T pop()
    {
        return _pop();
    }

    inline bool empty()
    {
        return ((Node*)head.load())->value == nullptr;
    }

    std::list<T> fetchAll()
    {
        decltype(fetchAll()) ret;
        auto last = guard.load();
        auto first = head.exchange(last);

        if (first == last)
            return ret;

        while (first != last)
        {
            if (first->value == nullptr)
            {
                head.compare_exchange_strong(last, first);
                break;
            }
            ret.push_back(*first->value);
            auto tmp = first;
            first = tmp->next;
            delete tmp;
        }

        return ret;
    }

private:
    struct Node
    {
        inline Node(T const& t) : value(new T(t)) { }
        inline Node(T&& t) : value(new T(std::move(t))) { }

        inline Node() = default;
        inline Node(Node const& n) = delete;

        inline ~Node() { delete value; }

        T* value = nullptr;
        Node* next = nullptr;
    };

    std::atomic<Node*> head;
    std::atomic<Node*> guard;

    void _push(Node* n)
    {
        auto tmp = n->value;
        n->value = nullptr;
        auto last = guard.exchange(n);
        last->next = n;
        last->value = tmp;
    }

    T _pop()
    {
        Node* first = nullptr;
        Node* prev = nullptr;
        do
        {
            first = head.load();
            prev = head.exchange(first->next);
        } while (prev != first);

        if (first == guard)
            throw std::runtime_error("Pop from empty queue!");

        auto ret = *first->value;
        delete first;
        return ret;
    }
};


#endif //SHARED_NON_LOCKING_QUEUE_H
