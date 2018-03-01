#include "../brown_helga_k_tree.h"
#include <cmath>

#ifndef UNTITLED_LOAD_TEST_H
#define UNTITLED_LOAD_TEST_H

void thread_insert_one_and_count(int size, int *count, cds::container::BrownHelgaKtree *tree) {
    bool res;
    for (int i = 0; i < size; ++i) {
        res = tree->insert(1);
        if (res) (*count)++;
    }
}

void thread_delete_one_and_count(int size, int *count, cds::container::BrownHelgaKtree *tree) {
    bool res;
    for (int i = 0; i < size; ++i) {
        res = tree->remove(1);
        if (res) (*count)--;
    }
}

bool test_insert_one_and_count() {
    int size = 1000;
    int *count = new int(0);
    auto *tree = new cds::container::BrownHelgaKtree();
    std::thread t(thread_insert_one_and_count, size, count, tree);
    t.join();
    if (*count == 1) {
        return true;
    }
    return false;
}

bool test_delete_one_and_count() {
    int size = 1000000;
    int *count = new int(0);
    auto *tree = new cds::container::BrownHelgaKtree();
    tree->insert(1);

    std::thread t(thread_delete_one_and_count, size, count, tree);
    t.join();
    if (*count == -1) {
        return true;
    }
    std::cout << *count << std::endl;
    return false;
}

bool test_same_count_operation() {
    int size = 10000000;
    int *count_insert_0 = new int(0);
    int *count_insert_1 = new int(0);
    int *count_insert_2 = new int(0);
    int *count_insert_3 = new int(0);
    int *count_remove_0 = new int(0);
    int *count_remove_1 = new int(0);
    int *count_remove_2 = new int(0);
    int *count_remove_3 = new int(0);


    auto *tree = new cds::container::BrownHelgaKtree();

    std::thread *t_insert_0 = new std::thread(thread_insert_one_and_count, size, count_insert_0, tree);
    std::thread *t_insert_1 = new std::thread(thread_insert_one_and_count, size, count_insert_1, tree);
    std::thread *t_insert_2 = new std::thread(thread_insert_one_and_count, size, count_insert_2, tree);
    std::thread *t_insert_3 = new std::thread(thread_insert_one_and_count, size, count_insert_3, tree);
    std::thread *t_delete_0 = new std::thread(thread_delete_one_and_count, size, count_remove_0, tree);
    std::thread *t_delete_1 = new std::thread(thread_delete_one_and_count, size, count_remove_1, tree);
    std::thread *t_delete_2 = new std::thread(thread_delete_one_and_count, size, count_remove_2, tree);
    std::thread *t_delete_3 = new std::thread(thread_delete_one_and_count, size, count_remove_3, tree);

    t_insert_0->join();
    t_insert_1->join();
    t_insert_2->join();
    t_insert_3->join();
    t_delete_0->join();
    t_delete_1->join();
    t_delete_2->join();
    t_delete_3->join();

    int diff = abs(*count_insert_0 + *count_remove_0 + *count_insert_1 + *count_remove_1 +
    *count_insert_2 + *count_insert_3 + *count_remove_2 + *count_remove_3);

    delete count_insert_0;
    delete count_insert_1;
    delete count_insert_2;
    delete count_insert_3;
    delete count_remove_0;
    delete count_remove_1;
    delete count_remove_2;
    delete count_remove_3;


    delete tree;

    if (diff <= 1) {
        return true;
    } else {
        return false;
    }


}


#endif //UNTITLED_LOAD_TEST_H

