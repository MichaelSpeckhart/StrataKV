#pragma once

#ifndef STRATAKV_UNIQUE_POINTER_H
#define STRATAKV_UNIQUE_POINTER_H

#include <memory>
#include <iostream>

namespace stratakv {



template <class T, class Deleter = std::default_delete<T>>
requires (std::is_default_constructible_v<Deleter>)
class UniquePointer {
    public:

    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    constexpr UniquePointer(std::nullptr_t = nullptr) noexcept : ptr_(nullptr) {}

    constexpr explicit UniquePointer(T* pointer, Deleter d = Deleter()) : ptr_(pointer), deleter_(std::move(d)) {}

    constexpr UniquePointer(UniquePointer&& other) : ptr_(other.release()), deleter_(std::move(other.deleter_)) {}

    UniquePointer(const UniquePointer&) = delete;

    UniquePointer& operator=(const UniquePointer&) = delete;

    constexpr T* release() noexcept {
        if (ptr_ == nullptr) {
            return nullptr;
        }
        
        T* pointer = ptr_;

        ptr_ = nullptr;

        return pointer;
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
        return *ptr_;
    }

    constexpr T* get() const noexcept {
        return ptr_;
    }
    
    constexpr Deleter& get_deleter() noexcept {
        return deleter_;
    }

    constexpr UniquePointer& operator=(UniquePointer&& rvalue) noexcept {
        if (this != &rvalue) {
            reset(rvalue.release());
        }

        return *this;
    }

    constexpr explicit operator bool() const noexcept {
        if (ptr_ == nullptr) {
            std::cout << "Pointer is null\n";
        }
        return get() != nullptr;
    }

    ~UniquePointer() {
        if (ptr_)
            deleter_(ptr_);
    }

    private:
    T* ptr_;
    Deleter deleter_;
};


// Array Specialization

template <class T, class Deleter>
class UniquePointer<T[], Deleter> {

    public:



    private:
    T* ptr_;
    Deleter deleter_;
};

}
#endif 