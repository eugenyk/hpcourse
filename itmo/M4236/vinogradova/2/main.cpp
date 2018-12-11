#include <iostream>
#include <thread>
#include <atomic>
#include <cmath>
#include "priority_queue.h"

std::atomic<bool> ready_to_insert;
std::atomic<bool> ready_to_delete_element;
Queue<int> *queue;

void delete_elem()
{
    while (!ready_to_delete_element.load()) 
    {};

    int priority = queue->dequeue();
    if (priority != -1) 
        std::cout << "elem is removed " << std::endl;
    else
        std::cout << "queue is empty" << std::endl;
}

void insert()
{
    while (!ready_to_insert.load()) 
    {};

    int value = rand() % RAND_MAX;
    size_t priority = rand() % 100;
    queue->enqueue(value, priority);
    std::cout << "elem is inserted" << std::endl;
}

// Are priorities unique?
int run(size_t count_inserts, size_t count_deletions) 
{
    // std::cout << "Enter count of insert threads" << std::endl;
    // std::cin >> count_inserts;
    // std::cout << "Enter count of delete threads" << std::endl;  
    // std::cin >> count_deletions;
    ready_to_delete_element.store(false);
    ready_to_insert.store(false);

    set_max_level((int)log2(count_inserts));
    queue = new Queue<int>();
    std::cout << "Is queue empty: " << queue->isEmpty() << std::endl; 

    std::thread* inserting_threads = new std::thread[count_inserts];
    for (size_t i = 0; i < count_inserts; i++)
        inserting_threads[i] = std::thread(insert);

    std::thread* deleting_threads = new std::thread[count_deletions];
    for (size_t i = 0; i < count_deletions; i++)
        deleting_threads[i] = std::thread(delete_elem);
    
    ready_to_insert.store(true);
    ready_to_delete_element.store(true);

    
    for(size_t i = 0; i < count_inserts; i++)
        inserting_threads[i].join();
    for (size_t i = 0; i < count_deletions; i++) 
        deleting_threads[i].join();
    
    std::cout << "Is queue empty: " << queue->isEmpty() << std::endl;

    delete [] deleting_threads;
    delete [] inserting_threads;
    std::cout << "FINISH" << std::endl;
}

int main()
{
    for (size_t i = 0; i < 1000; i++)
    {
        size_t count_inserts = rand() % 1000 + 1;
        size_t count_deletions = rand() % 1000 + 1;
        run(count_inserts, count_deletions);
    }

    return 0;
}