#ifndef PIMPL_H_
#define PIMPL_H_

#include <memory> // std::unique_ptr

class Interface
{
public:
    virtual ~Interface() {}

    virtual void func_a() = 0;
    virtual void func_b() = 0;
};

class PublicImplementation
{
    // smart pointer provides exception safety
    std::unique_ptr<Interface> impl;

public:
    PublicImplementation();

    // pass-through invoker
    Interface* operator->() { return impl.get(); }
};

#endif // PIMPL_H_
Pimpl.cpp

#include "Pimpl.h"
#include <iostream>

class PrivateImplementation
: public Interface
{
public:

    void func_a() override { std::cout << "a" << '\n'; }
    void func_b() override { std::cout << "b" << '\n'; }
};

PublicImplementation::PublicImplementation()
: impl(new PrivateImplementation)
{
}
And finally this is what the client code does:

Main.cpp

#include "Pimpl.h"

int main()
{
    PublicImplementation pi; // not a pointer

    pi->func_a(); // pointer semantics
    pi->func_b();
}
