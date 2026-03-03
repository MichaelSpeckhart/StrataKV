#pragma once

#include <vector>
#include <utility>
#include <functional>

#include <queue>


#ifndef STRATAKV_PRIORITY_QUEUE_H
#define STRATAKV_PRIORITY_QUEUE_H


namespace stratakv {

template <typename T, typename Container = std::vector<T>, typename Comparator = std::less<typename Container::value_type>> 
class PriorityQueue {
    public:

    PriorityQueue() {

    }


    void push(const T value) {
        container_.push_back(value);

        heapify_up(container_.size() - 1);
    } 

    void pop() {
        if (!container_.empty()) {
            container_[0] = container_.back();
            
            container_.pop_back();

            heapify_down(0);
        }
    }

    T top() {
        if (!container_.empty())
            return container_.front();
        
        return T();
    }


    private:
    void heapify_up(int index) {
        while (index > 0) {
            int parent_idx = (index - 1) / 2;
            
            if (compare(container_[index], container_[parent_idx])) {
                break;
            }

            std::swap(container_[index], container_[parent_idx]);
            index = parent_idx;
        }
    }

    void heapify_down(int index) {
        while (index < container_.size()) {
            
            int left = (2 * index) + 1;
            int right = (2 * index) + 2;

            int smallest = index;
            
            if (left < container_.size() && compare(container_[left], container_[smallest])) {

                smallest = left;

            } 
            if (right < container_.size() && compare(container_[right], container_[smallest])) {

                smallest = right;
            }

            if (smallest == index) {
                break;
            }

            std::swap(container_[index], container_[smallest]);
            index = smallest;

        }
    }   


    typename Container::size_type queue_size_;
    Container container_;
    Comparator compare;

};

} 


#endif