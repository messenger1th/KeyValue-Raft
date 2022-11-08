#include <iostream>
#include "SkipList.h"
#include <climits>
#include <cassert>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <algorithm>
using namespace std;

bool test1(size_t list_level = 20, int element_count = 1000000) {
    SkipList<int, int> list(list_level);
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < element_count; ++i) {
        list.insert(i, i);
//        assert(list.size() == i + 1);
    }
    for (int i = 0; i < element_count; ++i) {
        list.erase(i);
        assert(list.size() == element_count - i - 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << "Pass Test1 : single element insert and erase consequently, elapsed :" << elapsed.count() << endl;
    return true;
}

bool test2(size_t list_level = 30, int element_count = 10000) {
    SkipList<int, int> list(list_level);
    for (int i = 0; i < element_count; ++i) {
        list.insert(i, i);
        list.erase(i);
        assert(list.size() == 0);
    }
    cout << "Pass Test2 : single value insert & erase" << endl;
    return true;
}

bool test3(size_t list_level = 30, int element_count = 10000) {
    SkipList<int, int> list(list_level);
    for (int i = 0; i < element_count; ++i) {
        list.insert(i, i);
        list.insert(i, i);
        list.erase(i);
        assert(list.size() == (i + 1));
    }
    cout << "Pass Test3 : two element insert &  erase one of them" << endl;
    return true;
}


bool test4(size_t list_level = 30, int element_count = 10000) {
    SkipList<int, int> list(list_level);
    for (int i = 0; i < element_count; ++i) {
        list.erase(i);
        list.insert(i, i);
        list.insert(i, i);
        assert(list.size() == 2 * (i + 1));
    }
    cout << "Pass Test4 : erase element non in list but insert it twice" << endl;
    return true;
}
bool test5(size_t list_level = 30, int element_count = 10000) {
    SkipList<int, int> list(list_level);
    unordered_map<int, int> frequency;
    int element_total_frequency = 0;
    for (int i = 0; i < element_count; ++i) {
        int insert_random_value = rand() % element_count;
        list.insert(insert_random_value, insert_random_value);
        ++frequency[insert_random_value];
        ++element_total_frequency;
        int erase_random_value = rand() % INT_MAX;
        list.erase(erase_random_value);
        if (frequency[erase_random_value]) {
            --frequency[erase_random_value];
            --element_total_frequency;
        }
        assert(list.size() == element_total_frequency);
    }
    cout << "Pass Test5 : random insert & random erase" << endl;
    return true;
}

bool test_erase_all(size_t list_level = 15, size_t element_count = 10000) {
    SkipList<int, int> list(list_level);
    for (int i = 0; i < element_count; ++i) {
        list.insert(i, i);
    }
    for (int i = 0; i < element_count; ++i) {
        list.erase_all(i);
    }
    cout << "Pass Test erase_all()" << endl;
    return true;
}

bool test_erase_range(size_t list_level = 15, size_t element_count = 10000) {
    int lower = 0, upper = element_count;
    SkipList<int, int> list(list_level);

    for (int i = 0; i < element_count; ++i) {
        list.insert(i, i);
    }
    list.erase_range(lower, element_count / 2);
    assert(list.size() == element_count / 2);
    list.erase_range(lower, upper);
    assert(list.size() == 0);
    cout << "Pass Test erase_range()" << endl;
    return true;
}

void insert_task(SkipList<int, int>& skipList, size_t patch_size, size_t base) {
    for (int i = base; i < patch_size + base; ++i) {
        skipList.insert(i, i);
    }
}
void search_task(SkipList<int, int>& skipList, size_t patch_size, size_t base) {
    for (int i = base; i < patch_size + base; ++i) {
        assert(skipList.search(i));
    }
}
void erase_task(SkipList<int, int>& skipList, size_t patch_size, size_t base) {
    for (int i = base; i < patch_size + base; ++i) {
        skipList.erase(i);
    }
}

bool test6(size_t list_level = 20, int element_count = 10000) {
    SkipList<int, int> list(list_level);

    for (int i = 0; i < element_count; ++i) {
        list.insert(i, i);
        assert(list.size() == i + 1);
    }


    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < element_count; ++i) {
        list.erase(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << "Pass Test6 : insert data & check search " << elapsed.count() << endl;
    return true;
}

bool test_multi_thread( size_t thread_count = std::thread::hardware_concurrency(), size_t list_level = 20, size_t element_count = 1000000) {
    SkipList<int, int> skipList(list_level);

    /*thread information */
    auto every_thread_size = element_count / thread_count;
    auto total_size = every_thread_size * thread_count;

    /* insert 0->element count to skip list */
    vector<thread> insert_threads; insert_threads.reserve(thread_count);
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < thread_count; ++i) {
        insert_threads.emplace_back(insert_task, std::ref(skipList), every_thread_size, i * every_thread_size);
    }
    for_each(insert_threads.begin(), insert_threads.end(), [] (auto& t) {
        t.join();
    });
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> insert_duration = end - start;
    assert(skipList.size() == total_size);


    /* search 0->element count to skip list */
    vector<thread> search_treads; search_treads.reserve(thread_count);
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < thread_count; ++i) {
        search_treads.emplace_back(search_task, std::ref(skipList), every_thread_size, i * every_thread_size);
    }
    for_each(search_treads.begin(), search_treads.end(), [] (auto& t) {
        t.join();
    });
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> search_duration = end - start;
    assert(skipList.size() == total_size);

    /* erase 0->element count to skip list */
    vector<thread> erase_threads; erase_threads.reserve(thread_count);
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < thread_count; ++i) {
        erase_threads.emplace_back(erase_task, std::ref(skipList), every_thread_size, i * every_thread_size);
    }
    for_each(erase_threads.begin(), erase_threads.end(), [] (auto& t) {
        t.join();
    });
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> erase_duration = end - start;
    assert(skipList.size() == 0);

    cout << "Pass Test multi-task " << endl;
    cout << "insert duration: " << insert_duration.count() << endl;
    cout << "search duration: " << search_duration.count() << endl;
    cout << "erase  duration: " << erase_duration.count() << endl;
}

bool do_test() {
    srand(time(NULL));

    /* test for insert & erase*/
//    test1();
//    test2();
//    test3();
//    test4();
//    test5();
//    test6();
    /* test for erase_all() & erase_range() */
//    test_erase_all();
//    test_erase_range();

    /* do concurrency test */
//    test_multi_thread();
    return true;
}

int main(int argc, const char** argv) {
    cout << "Hello Skip List!" << endl;
    do_test();
}
