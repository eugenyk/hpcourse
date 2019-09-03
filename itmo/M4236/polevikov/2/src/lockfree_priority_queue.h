#pragma once

#include <atomic>
#include <cstddef>

#include <cds/gc/hp.h>
#include <cds/details/marked_ptr.h>

#include <iostream>

struct queue_stats
{
    size_t items_total;
    size_t items_removed;
    bool is_sorted;

    queue_stats() : items_total(0), items_removed(0), is_sorted(true) {}

    void print() const
    {
        std::cout << "items total: " << items_total << std::endl;
        std::cout << "items removed: " << items_removed << std::endl;
        std::cout << "is sorted: " << is_sorted << std::endl;
    }
};

template<class T>
class lockfree_priority_queue
{
public:
    lockfree_priority_queue();
    ~lockfree_priority_queue();

    void push(const T&) const;
    void pop() const;
    bool empty() const;

    void print() const;
    queue_stats stats() const;

private:
    struct node;
    using node_marked_ptr = cds::details::marked_ptr<node, 1>;

    struct node
    {
        T data;
        std::atomic<node_marked_ptr> next;

        explicit node(const T& data) : data(data) {}
    };

    struct position
    {
        node_marked_ptr prev_node;
        node_marked_ptr curr_node;
        node_marked_ptr next_node;
        cds::gc::HP::GuardArray<3> guards;

        void init(const std::atomic<node_marked_ptr> &head)
        {
            curr_node = guards.protect(1, head,
                                       [](node_marked_ptr &p)
                                       { return p.ptr(); });
            next_node = guards.protect(2, curr_node->next,
                                       [](node_marked_ptr &p)
                                       { return p.ptr(); });
        }

        void advance()
        {
            prev_node = curr_node;
            guards.copy(0, 1);
            curr_node = next_node;
            guards.copy(1, 2);
            next_node = guards.protect(2, curr_node->next,
                                       [](node_marked_ptr &p)
                                       { return p.ptr(); });
        }
    };

    void search(position&, const T&) const;
    bool try_insert(position&, node_marked_ptr&) const;
    bool try_retire(position&) const;
    void retire_node(node_marked_ptr&) const;

    std::atomic<node_marked_ptr> _head;
    std::atomic<node_marked_ptr> _tail;
};

template<class T>
lockfree_priority_queue<T>::lockfree_priority_queue()
{
    node_marked_ptr head(new node(T()));
    node_marked_ptr tail(new node(T()));
    _head.store(head);
    _tail.store(tail);
    _head.load()->next.store(tail);
    _tail.load()->next.store(node_marked_ptr(nullptr));
}

template<class T>
lockfree_priority_queue<T>::~lockfree_priority_queue()
{
    node_marked_ptr curr_node = _head.load()->next;

    while (_head.load()->next.load().ptr() != _tail)
    {
        _head.load()->next.store(curr_node->next);
        delete curr_node.ptr();
        curr_node = _head.load()->next;
    }

    delete _head.load().ptr();
    delete _tail.load().ptr();
}

template<class T>
void lockfree_priority_queue<T>::push(const T& data) const
{
    node_marked_ptr new_node(new node(data));
    position pos;
    while (true)
    {
        search(pos, data);
        if (pos.next_node.bits() == 1)
        {
            try_retire(pos);
            continue;
        }
        if (try_insert(pos, new_node))
        {
            return;
        }
    }
}

template<class T>
void lockfree_priority_queue<T>::search(position& pos, const T& data) const
{
    pos.init(_head);
    while (pos.next_node.ptr() != _tail
           and pos.next_node.bits() == 0
           and pos.next_node->data > data)
    {
        pos.advance();
    }
}

template<class T>
bool lockfree_priority_queue<T>::try_insert(position& pos,
                                            node_marked_ptr& new_node) const
{
    node_marked_ptr next(pos.next_node.ptr());
    new_node->next.store(next);
    return pos.curr_node->next.compare_exchange_strong(next, new_node);
}

template<class T>
void lockfree_priority_queue<T>::pop() const
{
    position pos;
    pos.init(_head);

    if (pos.next_node.ptr() == _tail)
    {
        return;
    }

    pos.advance();
    while (true)
    {
        node_marked_ptr next(pos.next_node);
        if (next.bits() == 0)
        {
            node_marked_ptr marked_next(next.ptr(), 1);
            if (pos.curr_node->next.compare_exchange_strong(next, marked_next))
            {
                try_retire(pos);
                return;
            }
        }
        if (pos.curr_node.ptr() == _tail)
        {
            return;
        }
        pos.advance();
    }
}

template<class T>
bool lockfree_priority_queue<T>::empty() const
{
    position pos;
    pos.init(_head);
    while (pos.next_node.ptr() != _tail and pos.next_node.bits() == 1)
    {
        pos.advance();
    }
    return pos.next_node == _tail;
}

template<class T>
bool lockfree_priority_queue<T>::try_retire(position& pos) const
{
    node_marked_ptr curr(pos.curr_node.ptr());
    node_marked_ptr next(pos.next_node.ptr());
    if (pos.prev_node->next.compare_exchange_strong(curr, next))
    {
        retire_node(curr);
        return true;
    }
    return false;
}

template<class T>
void lockfree_priority_queue<T>::retire_node(node_marked_ptr &node_ptr) const
{
    cds::gc::HP::retire(node_ptr.ptr(),
                        [](void* ptr)
                        { delete static_cast<node*>(ptr); });
}

template<class T>
void lockfree_priority_queue<T>::print() const
{
    node_marked_ptr curr_node = _head.load()->next;
    std::cout << "head -> ";
    while (curr_node.ptr() != _tail)
    {
        if (curr_node->next.load().bits() == 0)
        {
            std::cout << curr_node->data << " -> ";
        }
        else
        {
            std::cout << curr_node->data << " [r]" << " -> ";
        }
        curr_node = curr_node->next;
    }
    std::cout << "tail" << std::endl;
}

template<class T>
queue_stats lockfree_priority_queue<T>::stats() const
{
    queue_stats stats;
    node_marked_ptr curr_node = _head;
    while (curr_node.ptr() != _tail)
    {
        if (curr_node.ptr() != _head)
        {
            stats.items_total += 1;
        }
        if (curr_node->next.load().bits() == 1)
        {
            stats.items_removed += 1;
        }
        if (curr_node.ptr() != _head
            and curr_node->data < curr_node->next.load()->data)
        {
            stats.is_sorted = false;
        }
        curr_node = curr_node->next;
    }
    return stats;
}
