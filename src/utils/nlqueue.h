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
        _push(new Node(t));
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
        auto first = head.exchange(guard);
        if (first == guard)
            return ret;

        while (first->value != nullptr)
        {
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
        inline Node(T&& t) : value(new T(t)) { }

        inline Node() = default;
        inline Node(Node const& n) = delete;

        inline ~Node() { delete value; }

        T* value = nullptr;
        Node* next = nullptr;
    };

    std::atomic<Node*> head;
    Node* guard = new Node();

    void _push(Node* n)
    {
        guard->next = n;
        auto tmp = n->value;
        n->value = nullptr;
        guard->value = tmp;
        guard = n;
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
