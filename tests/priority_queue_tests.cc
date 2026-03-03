#include <catch2/catch_test_macros.hpp>

#include "priority_queue.h"
#include <iostream>


class PriorityQueueFixture {
    public:

    PriorityQueueFixture() {
        mMinHeap.push(5);
    }



    private:

    stratakv::PriorityQueue<int, std::vector<int>, std::greater<int>> mMinHeap;



};

TEST_CASE_METHOD(PriorityQueueFixture, "Test Pop First Element", "[PriorityQueue]") {
    stratakv::PriorityQueue<int, std::vector<int>, std::greater<int>> mMinHeap;

    mMinHeap.push(5);

    auto val = mMinHeap.top();

    std::cout << "Val: " << val << "\n";

    mMinHeap.push(4);

    val = mMinHeap.top();

    std::cout << "Val: " << val << "\n";

    mMinHeap.push(6);

    mMinHeap.pop(); // Pops 4

    val = mMinHeap.top();

    std::cout << "Val: " << val << "\n";


    assert(val == 5);


}
