#include <catch2/catch_test_macros.hpp>

#include "unique_pointer.h"

#include <iostream>

using namespace stratakv;

// Sample resource to manage taken from cppreference.com
struct Foo 
{
    Foo() { std::cout << "Foo ctor\n"; }
    Foo(const Foo&) { std::cout << "Foo copy ctor\n"; }
    Foo(Foo&&) { std::cout << "Foo move ctor\n"; }
    Foo(std::string newName) : foo(newName) { std::cout << "Foo ctor with new name\n"; } 
    ~Foo() { std::cout << "~Foo dtor\n"; }

    std::string name() const { return foo; }

    std::string foo = "Foo";

    
};

struct Bar 
{
    Bar() { std::cout << "Bar ctor\n"; }
    Bar(const Bar&) { std::cout << "Bar copy ctor\n"; }
    Bar(Bar&&) { std::cout << "Bar move ctor\n"; }
    ~Bar() { std::cout << "~Bar dtor\n"; }

    std::string name() { return bar; }

    std::string bar = "Bar";
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

void f(const Foo& foo) {
    std::cout << "f(const Foo&)\n";
    std::cout << foo.name() << "\n";
}

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

TEST_CASE("Unique Pointer Move Operation", "[UniquePointer]") {
    UniquePointer<Foo> resource(new Foo());

    auto newResource = std::move(resource);

    REQUIRE(resource.get() == nullptr);
    REQUIRE(newResource.get() != nullptr);
}

TEST_CASE("Unique Pointer Reset() And Null Out", "[UniquePointer]") {
    UniquePointer<Foo> resource(new Foo());

    resource.reset();

    REQUIRE(resource.get() == nullptr);
}

TEST_CASE("Unique Pointer Reset() With New Resource", "[UniquePointer]") {
    UniquePointer<Foo> resource(new Foo());

    resource.reset(new Foo("New Foo"));

    REQUIRE(resource.get() != nullptr);
    REQUIRE(resource.get()->name() == "New Foo");
}

TEST_CASE("Unique Pointer Release() pointer and return ownership", "[UniquePointer]") {
    UniquePointer<Foo> resource(new Foo());

    void* addr = (void*)resource.get();

    Foo* ptr = resource.release();

    REQUIRE(ptr != nullptr);
    REQUIRE(static_cast<void*>(ptr) == addr);
    REQUIRE(resource.get() == nullptr);
    UniquePointer<Foo*>::element_type variable;
    delete ptr;
}

TEST_CASE("Unique Pointer Operator Bool Test Is Not Null", "[UniquePointer]") {
    stratakv::UniquePointer<Foo> resource(new Foo());

    REQUIRE(resource);
}

TEST_CASE("Unique Pointer Operator Bool Test Is Null", "[UniquePointer]") {
    stratakv::UniquePointer<Foo> resource(nullptr);

    REQUIRE(!resource);
}

TEST_CASE("Unique Pointer Operator * Test Can Be Passed To Function", "[UniquePointer]") {
    stratakv::UniquePointer<Foo> resource(new Foo());

    f(*resource);
}