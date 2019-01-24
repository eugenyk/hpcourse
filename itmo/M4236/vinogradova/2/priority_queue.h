#include <iostream>
#include <vector>
#include <atomic>
#include <climits>
#include <memory>


size_t MAX_LEVEL;

size_t random_level()
{
    size_t level = 0;
    double random_value = (double)(rand()) / RAND_MAX;
    while (random_value < 0.5 && level < MAX_LEVEL)
    {
        level++;
        random_value = (double)(rand())/RAND_MAX;
    }
    return level;
}

void set_max_level(size_t value)
{
    MAX_LEVEL = value;
}

template<class NodeT>
class HolderPtr
{
public:
    struct Holder
    {
        std::shared_ptr<NodeT> next;
        bool marked;

        Holder()
        : next(nullptr), marked(0)
        {}

        Holder(std::shared_ptr<NodeT> node, bool marked)
        : next(node), marked(marked)
        {}

    };

    HolderPtr()
    {
        std::shared_ptr<Holder> tmp = std::make_shared<Holder>();
        atomic_store(&holder, tmp);
    }

    HolderPtr(std::shared_ptr<NodeT> neighbour, bool marked)
    {
        std::shared_ptr<Holder> new_holder = std::make_shared<Holder>(neighbour, marked);
        atomic_store(&holder, new_holder);
    }

    std::shared_ptr<NodeT> getNext()
    {
        return atomic_load(&holder)->next;
    }

    std::shared_ptr<NodeT> get(bool& marked)
    {
        std::shared_ptr<Holder> tmp = atomic_load(&holder);
        marked = tmp->marked;
        return tmp->next;
    }

    void set(std::shared_ptr<NodeT> newRef, bool newMark)
    {
        std::shared_ptr<Holder> tmp = std::make_shared<Holder>(newRef, newMark);
        atomic_store(&holder, tmp);
    }

    bool CAS(std::shared_ptr<NodeT> new_value, bool new_marked)
    {
        std::shared_ptr<Holder> current = atomic_load(&holder);
        std::shared_ptr<Holder> new_holder = std::make_shared<Holder>(new_value, new_marked);
        return atomic_compare_exchange_strong(&holder, &current, new_holder);
    }

private:
    std::shared_ptr<Holder> holder;
};


template<class T>
class Queue
{
public:
    Queue()
    {
        head = std::make_shared<Node>(-1, T());
        tail = std::make_shared<Node>(INT_MAX, T());

        for(int i = 0; i <= MAX_LEVEL; i++)
        {
            head->neighbour[i].set(tail, false);
        }
    }

    int dequeue()
    {
        std::shared_ptr<Node> current = head->neighbour[0].getNext();
        while(current != tail)
        {
            if (!current->deleted.load())
                if (!current->deleted.exchange(true))
                    return remove(current->priority);
                else
                    current = current->neighbour[0].getNext();
            else
                current = current->neighbour[0].getNext();
        }
        return -1;
    }

    void enqueue(size_t priority, T value)
    {
        size_t max_elem_level = random_level();
        std::vector<std::shared_ptr<Node>> prevs(MAX_LEVEL + 1);
        std::vector<std::shared_ptr<Node>> neighbours(MAX_LEVEL + 1);
        std::shared_ptr<Node> new_node = std::make_shared<Node>(priority, value, max_elem_level);
        while (1)
        {   
            findPlace(priority, prevs, neighbours);
            
            for (size_t i = 0; i <= max_elem_level; i++)
                new_node->neighbour[i].set(neighbours[i], false);

            std::shared_ptr<Node> prev = prevs[0];
            std::shared_ptr<Node> next = neighbours[0];

            if (!prev->neighbour[0].CAS(new_node, false))
                continue;

            for (size_t i = 1; i <= max_elem_level; i++)
                while(1)
                {
                    prev = prevs[i];
                    if (prev->neighbour[i].CAS(new_node, false))
                        break;

                    findPlace(priority, prevs, neighbours);
                }

            return;
        }
    }

    bool isEmpty()
    {
        std::shared_ptr<Node> current = head->neighbour[0].getNext();
        while(current != tail)
        {
            if (!current->deleted.load())
                    return false;
            else
                current = current->neighbour[0].getNext();
        }
        return true;
    }

private:
    struct Node
    {
    public:
        T value;
        int priority;
        int level;
        std::atomic<bool> deleted;
        std::vector<HolderPtr<Node>> neighbour;

        Node()
        : priority(0), deleted(0), level(MAX_LEVEL)
        {
            neighbour = std::vector<HolderPtr<Node>>(MAX_LEVEL + 1);
        }

        Node(size_t new_priority, T new_value, size_t new_level = MAX_LEVEL)
        : priority(new_priority), value(new_value), deleted(0), level(new_level)
        {
            neighbour = std::vector<HolderPtr<Node>>(level + 1);
        }
    };

    std::shared_ptr<Node> head;
    std::shared_ptr<Node> tail;

    bool findPlace(int needed_node_priority, std::vector<std::shared_ptr<Node>>& prevs, std::vector<std::shared_ptr<Node>>& neighbours)
    {
        bool marked;
        std::shared_ptr<Node> current;
        while (1)
        {
            bool isFailed = false;
            std::shared_ptr<Node> prev = head;
            for (int i = MAX_LEVEL; i >= 0; i--)
            {
                current = prev->neighbour[i].getNext();
                while(1)
                {
                    std::shared_ptr<Node> next = current->neighbour[i].get(marked);
                    while (marked || current->deleted.load())
                    {
                        if (prev->neighbour[i].CAS(next, false))
                        {
                            isFailed = true;
                            break;
                        }
                        current = prev->neighbour[i].getNext();
                        next = current->neighbour[i].get(marked);
                    }

                    if (current->priority < needed_node_priority && !isFailed)
                    {
                        prev = current;
                        current = next;
                    }
                    else
                        break;
                }

                if (isFailed)
                    break;
                prevs[i] = prev;
                neighbours[i] = current;
            }

            if (!isFailed)
                return (current->priority == needed_node_priority);
        }
    }

    bool remove(int priority)
    {
        std::vector<std::shared_ptr<Node>> prevs(MAX_LEVEL + 1);
        std::vector<std::shared_ptr<Node>> neighbours(MAX_LEVEL + 1);
        std::shared_ptr<Node> next;
        while(1)
        {
            bool found = findPlace(priority, prevs, neighbours);
            if (!found)
                return false;

            std::shared_ptr<Node> removed_node = neighbours[0];
            for (size_t level = removed_node->level; level >= 1; level--)
            {
                bool marked;
                next = removed_node->neighbour[level].get(marked);
                while(!marked)
                {
                    removed_node->neighbour[level].CAS(next, true);
                    next = removed_node->neighbour[level].get(marked);
                }
            }
            bool marked;
            next = removed_node->neighbour[0].get(marked);
            while(1)
            {
                bool was_marked = removed_node->neighbour[0].CAS(next, true);
                next = neighbours[0]->neighbour[0].get(marked);
                if (was_marked)
                {
                    findPlace(priority, prevs, neighbours);
                    return true;
                }
                else 
                    if (marked)
                        return false;
            }
        }
    }
};
