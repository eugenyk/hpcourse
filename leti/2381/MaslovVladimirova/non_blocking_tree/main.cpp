#include <iostream>
#include "brown_helga_k_tree.h"
#include "load_test/load_test.h"

void thread_insert(int n, int size, cds::container::BrownHelgaKtree *tree) {
    for (int i = 0; i < size; ++i) {
        std::string log = "thread_" + std::to_string(n);
        if (tree->insert(i))
            log += " inserted ";
        else
            log += " did not insert ";

        log += std::to_string(i) + "\n";
        std::cout << log;
    }
}

void thread_random_insert(int n, int size, cds::container::BrownHelgaKtree *tree) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, size - 1);

    for (int i = 0; i < size; ++i) {
        std::string log = "thread_" + std::to_string(n);
        int rand = dist(gen);
        if (tree->insert(rand))
            log += " inserted ";
        else
            log += " did not insert ";

        log += std::to_string(rand) + "\n";
        std::cout << log;
    }
}

void thread_random_remove(int n, int size, cds::container::BrownHelgaKtree *tree) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, size - 1);

    for (int i = 0; i < size; ++i) {
        std::string log = "thread_" + std::to_string(n);
        int rand = dist(gen);
        if (tree->remove(rand))
            log += " removed ";
        else
            log += " did not remove ";

        log += std::to_string(rand) + "\n";
        std::cout << log;
    }
}

void thread_remove(int n, int size, cds::container::BrownHelgaKtree *tree) {
    for (int i = 0; i < size; ++i) {
        std::string log = "thread_" + std::to_string(n);
        if (tree->remove(i))
            log += " removed ";
        else
            log += " did not remove ";

        log += std::to_string(i) + "\n";
        std::cout << log;
    }
}

void found_test() {
    auto *brownHelgaKtree = new cds::container::BrownHelgaKtree(4);
    int SIZE = 100;
    std::thread t1(thread_insert, 1, SIZE, brownHelgaKtree);
    std::thread t2(thread_insert, 2, SIZE, brownHelgaKtree);
    std::thread t3(thread_random_insert, 3, SIZE, brownHelgaKtree);
    t1.join();
    t2.join();
    t3.join();

    bool allFound = true;
    for (int i = 0; i < SIZE; i++) {
        if (!brownHelgaKtree->find(i))
            allFound = false;
    }
    std::cout << std::endl << (allFound ? "ALL KEYS FOUND" : "FIND TEST FAILED") << std::endl;
}

void insert_with_remove_test() {
    auto *brownHelgaKtree = new cds::container::BrownHelgaKtree(4);
    int SIZE = 100;
    std::thread t1(thread_insert, 1, SIZE, brownHelgaKtree);
    std::thread t2(thread_random_remove, 2, SIZE, brownHelgaKtree);
    std::thread t3(thread_random_insert, 3, SIZE, brownHelgaKtree);

    t1.join();
    t2.join();
    t3.join();
    delete brownHelgaKtree;
}

int main() {
    std::cout << "**********************INSERT TEST**********************" << std::endl;
    if (test_same_count_operation()) { std::cout << "test_same_count_operation passed" << std::endl; } else { std::cout << "test_same_count_operation failed" << std::endl; }
    if (test_insert_one_and_count()) { std::cout << "test_insert_one_and_count passed" << std::endl; } else { std::cout << "test_insert_one_and_count failed" << std::endl; }
    if (test_delete_one_and_count()) { std::cout << "test_delete_one_and_count passed" << std::endl; } else { std::cout << "test_delete_one_and_count failed" << std::endl; }
    return 0;

}