#pragma once

#ifndef STRATAKV_UNIQUE_POINTER_H
#define STRATAKV_UNIQUE_POINTER_H

#include <memory>


template <class T, class Deleter = std::default_delete<T>>
requires (std::is_default_constructible_v<Deleter>)
class UniquePointer {
    public:


    constexpr UniquePointer(std::nullptr_t = nullptr) noexcept : ptr_(nullptr) {}

    constexpr explicit UniquePointer(T* pointer, Deleter d = Deleter()) : ptr_(pointer), deleter_(std::move(d)) {}

    constexpr UniquePointer(UniquePointer&& other) : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    constexpr T* release() noexcept {
        if (ptr_ == nullptr) {
            return nullptr;
        }
        
        deleter_()(ptr_);

        return ptr_;
    }

    void swap(UniquePointer& ptr2) noexcept {
        std::swap(ptr2.ptr_, this->ptr_);
    }

    constexpr void reset(T* pointer = nullptr) noexcept {
        deleter_(ptr_);

        ptr_ = std::move(pointer);
    }

    constexpr T* operator->() const noexcept {
        if (ptr_ == nullptr) {
            return nullptr;
        }

        return ptr_;
    }
    
    typename std::add_lvalue_reference<T>::type operator*() const noexcept(
        noexcept(*std::declval<T*>())
    ) {
        return ptr_;
    }

    constexpr T* get() const noexcept {
        return ptr_;
    }
    
    constexpr Deleter& get_deleter() noexcept {
        return deleter_;
    }

    constexpr UniquePointer& operator=(UniquePointer&& rvalue) noexcept {
        if (this != &rvalue) {
            delete ptr_;
            this->ptr_ = rvalue.ptr_;
            rvalue.ptr_ = nullptr;
        }

        return *this;
    }

    ~UniquePointer() {
        if (ptr_)
            deleter_(ptr_);
    }

    private:
    T* ptr_;
    Deleter deleter_;
};




#endif 