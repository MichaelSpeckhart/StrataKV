#include <catch2/catch_test_macros.hpp>

#include "unique_pointer.h"

#include <iostream>



// Sample resource to manage taken from cppreference.com
struct Foo 
{
    Foo() { std::cout << "Foo ctor\n"; }
    Foo(const Foo&) { std::cout << "Foo copy ctor\n"; }
    Foo(Foo&&) { std::cout << "Foo move ctor\n"; }
    ~Foo() { std::cout << "~Foo dtor\n"; }

    std::string name() { return foo; }

    std::string foo = "Foo";
};

// Sample Deleter taken from cppreference.com
struct D
{
    D() {};
    D(int num) : number(num) { std::cout << "D num ctor\n"; }
    D(const D&) { std::cout << "D copy ctor\n"; }
    D(D&) { std::cout << "D non-const copy ctor\n"; }
    D(D&&) { std::cout << "D move ctor \n"; }

    void bar()
    {
        std::cout << "call D::bar(), my number is: " << std::dec << number << '\n';
    }

    void operator()(Foo* p) const
    {
        std::cout << "D is deleting a Foo\n";
        delete p;
    };


    int number;
};

struct D2
{
    int number;
 
    void bar()
    {
        std::cout << "call D::bar(), my number is: " << std::dec << number << '\n';
    }
 
    void operator()(Foo* p) const
    {
        std::cout << "call deleter for Foo object 0x" << std::hex << (void*)p << '\n';
        delete p;
    }
};

// 
TEST_CASE("Initialize Unique Pointer Null", "[UniquePointer]") {
    UniquePointer<Foo> resource;
    std::unique_ptr<Foo> resourceTwo;
    std::cout << __cplusplus << std::endl;
    REQUIRE(resourceTwo.get() == nullptr);
    REQUIRE(resource.get() == nullptr);
}

TEST_CASE("Initialize Unique Pointer With Foo", "[UniquePointer]") {
    UniquePointer<Foo> resource(new Foo);
    std::unique_ptr<Foo> resourceTwo(new Foo);

    REQUIRE(resource->name() == resource->foo);
    REQUIRE(resourceTwo->name() == resource->foo);
}

TEST_CASE("Initialize Unique Pointer With Foo And Deleter", "[UniquePointer]") {
    UniquePointer<Foo, D2> resource(new Foo(), D2(42));
    std::unique_ptr<Foo, D2> resourceTwo(new Foo(), D2(43));

    auto deleter = resource.get_deleter();
    auto deleterTwo = resourceTwo.get_deleter();

    REQUIRE(deleter.number == 42);
    // std::cout << deleter.number << "\n";
    REQUIRE(deleterTwo.number == 43);
}

TEST_CASE("Unique Pointer Move Operation", "UniquePointer") {
    UniquePointer<Foo> resource(new Foo());

    auto newResource = std::move(resource);

    REQUIRE(resource.get() == nullptr);
    REQUIRE(newResource.get() != nullptr);
}