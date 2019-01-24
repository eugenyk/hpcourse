#pragma once

#include <cds/details/marked_ptr.h>
#include <cds/gc/hp.h>

template <typename T>
class lock_free_priority_queue
{
public:

    lock_free_priority_queue();
    ~lock_free_priority_queue();
    bool enqueue(const T& val);
    bool dequeue(T& val);
    bool empty();

    // unsafe
    size_t size();

private:
    struct node;
    typedef  cds::details::marked_ptr<node, 1> marked_node_ptr;
    typedef std::atomic<marked_node_ptr> atomic_node_ptr;

    atomic_node_ptr head;

    struct node
    {
        T data;
        atomic_node_ptr next;

        explicit node(const T& data)
            : data(data){}

    }; //node

    struct position
    {
        atomic_node_ptr* prev_ref ;
        marked_node_ptr cur_node;
        marked_node_ptr next_node;

        cds::gc::HP::GuardArray<3> guards;

        enum
        {
            guard_prev_item,
            guard_cur_item,
            guard_next_item
        };


        void init(atomic_node_ptr &head)
        {
            prev_ref = &head;
            next_node = nullptr;
            cur_node = guards.protect(position::guard_cur_item, head,
                                  [](marked_node_ptr &p)
                                  {
                                      return p.ptr();
                                  });


            if (cur_node.ptr() == nullptr) {
                cur_node = nullptr;
                next_node = nullptr;
            }
        }

        void read_next()
        {
            next_node = guards.protect( position::guard_next_item, cur_node->next,
                                    [](marked_node_ptr &p)
                                    {
                                        return p.ptr();
                                    });
        }

        void cur_move()
        {
            cur_node = next_node;
            guards.copy(position::guard_cur_item, position::guard_next_item);
        }
    }; //position

    bool link_node(node* new_node, position& pos);
    void retire_node(node* del_node);
    bool unlink_node(position& pos, T& val);
    bool search(const T& val, position& pos);
}; //lock_free_priority_queue


template <typename T>
lock_free_priority_queue<T>::lock_free_priority_queue()
        : head(nullptr) {}

template <typename T>
lock_free_priority_queue<T>::~lock_free_priority_queue()
{
    static int i = 0;
    T val = 0;
    while (dequeue(val) && (i++, true));
}

template <typename T>
bool lock_free_priority_queue<T>::enqueue(const T& val)
{
    node* pNode = new node(val);
    position pos;

    while (true) {
        if (search(val, pos))
            return false;

        if (link_node(pNode, pos))
            return true;

    }
}

template <typename T>
bool lock_free_priority_queue<T>::dequeue(T& val)
{
    position pos;
    pos.init(head);

    while (true) {

        if (pos.cur_node.ptr() == nullptr)
            return false;

        pos.read_next();

        if (pos.prev_ref->load().all() != pos.cur_node.ptr()) {
            pos.init(head);
            continue;
        }

        if (pos.next_node.bits() == 1) {
            // pos.cur_node logically deleted. Help the delete for real
            marked_node_ptr cur(pos.cur_node.ptr());
            if (pos.prev_ref->compare_exchange_strong(cur, marked_node_ptr(pos.next_node.ptr()))) {
                retire_node(pos.cur_node.ptr());
            }
            else {
                pos.init(head);
                continue;
            }

            pos.cur_move();
        } else {
            if (unlink_node(pos, val))
                return true;
        }
    }
}

template <typename T>
bool lock_free_priority_queue<T>::empty()
{
    position pos;
    pos.init(head);
    while (true) {
      if (pos.cur_node.ptr() == nullptr)
        return true;

      pos.read_next();

      if (pos.prev_ref->load().all() != pos.cur_node.ptr()) {
        pos.init(head);
        continue;
      }

      if (pos.next_node.bits() == 1) {
        // pos.cur_node logically deleted. Help the delete for real
        marked_node_ptr cur(pos.cur_node.ptr());
        if (pos.prev_ref->compare_exchange_strong(cur, marked_node_ptr(pos.next_node.ptr()))) {
          retire_node(pos.cur_node.ptr());
        }
        else {
          pos.init(head);
          continue;
        }

        pos.cur_move();
      } else {
        return false;
      }
    }
}

//unsafe method
template <typename T>
size_t lock_free_priority_queue<T>::size()
{
    size_t counter = 0;
    node* cur = head.load().ptr();
    while (cur) {
        counter++;
        cur = cur->next.load().ptr();
    }
    return counter;
}

template <typename T>
bool lock_free_priority_queue<T>::link_node(node* new_node, position& pos)
{
    assert( new_node != nullptr );

    // link new node and cur
    marked_node_ptr cur(pos.cur_node);
    new_node->next.store(cur);
    // change ref to new_node
    if (pos.prev_ref->compare_exchange_strong(cur, marked_node_ptr(new_node)))
        return true;

    // delete link if CAS failed
    new_node->next.store(marked_node_ptr());
    return false;
}

template <typename T>
void lock_free_priority_queue<T>::retire_node(node* del_node)
{
    cds::gc::HP::retire(del_node, [](void* ptr){delete static_cast<node*>(ptr);});
}

template <typename T>
bool lock_free_priority_queue<T>::unlink_node(position& pos, T& val)
{
    assert( pos.prev_ref != nullptr );
    assert( pos.cur_node != nullptr );

    // logical deletion
    marked_node_ptr next(pos.next_node.ptr(), 0);
    if (pos.cur_node->next.compare_exchange_strong(next, marked_node_ptr(pos.next_node.ptr(), 1))) {
        // physical deletion
        val = pos.cur_node->data;
        marked_node_ptr cur(pos.cur_node);
        if (pos.prev_ref->compare_exchange_strong(cur, marked_node_ptr(pos.next_node)))
            retire_node(pos.cur_node.ptr());
        return true;
    }
    return false;
}

template <typename T>
bool lock_free_priority_queue<T>::search(const T& val, position& pos)
{
    pos.init(head);
    while (true) {

        if (pos.cur_node.ptr() == nullptr)
            return false;

        pos.read_next();

        if (pos.prev_ref->load().all() != pos.cur_node.ptr()) {
            pos.init(head);
            continue;
        }

        if (pos.next_node.bits() == 1) {
            // pos.cur_node logically deleted. Help the delete for real
            marked_node_ptr cur(pos.cur_node.ptr());
            if (pos.prev_ref->compare_exchange_strong(cur, marked_node_ptr(pos.next_node.ptr()))) {
                retire_node(pos.cur_node.ptr());
            }
            else {
                pos.init(head);
                continue;
            }
        }
        else {
            assert(pos.cur_node.ptr() != nullptr);
            if (pos.cur_node->data >= val) {
                return pos.cur_node.ptr()->data == val;
            }
            pos.prev_ref = &(pos.cur_node->next);
            pos.guards.copy(position::guard_prev_item, position::guard_cur_item);
        }
        pos.cur_move();
    }
}
